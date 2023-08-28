// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatJoinPublicRoom.h"

#include "OnlineAsyncTaskAccelByteChatQueryRoomById.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteChatJoinPublicRoom::FOnlineAsyncTaskAccelByteChatJoinPublicRoom(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FChatRoomId& InRoomId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RoomId(InRoomId)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatJoinPublicRoom::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const AccelByte::Api::Chat::FChatActionTopicResponse OnJoinPublicRoomSuccessDelegate =
		TDelegateUtils<AccelByte::Api::Chat::FChatActionTopicResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatJoinPublicRoom::OnJoinPublicRoomSuccess);
	const FErrorHandler OnJoinPublicRoomErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatJoinPublicRoom::OnJoinPublicRoomError);

	ApiClient->Chat.JoinTopic(RoomId, OnJoinPublicRoomSuccessDelegate, OnJoinPublicRoomErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatJoinPublicRoom::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join chat room as our chat interface instance is not valid!"));
		return;
	}

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join chat room as our user ID is not valid!"));
		return;
	}

	ChatInterface->TriggerOnChatRoomJoinPublicDelegates(GenericUserId.ToSharedRef().Get(), RoomId, bWasSuccessful, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatJoinPublicRoom::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineChatAccelBytePtr ChatInterface;
		if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to add topic to cache as our chat interface instance is not valid!"));
			return;
		}

		// Joining will trigger the AddToTopic event which will fetch the topic data there however,
		// we should go ahead and add the topic data here so that it is immediately accessible to the joining user
		if(!JoinedRoomInfoPtr.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to add topic to cache as our room info is not valid!"));
			return;
		}

		ChatInterface->AddTopic(JoinedRoomInfoPtr.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatJoinPublicRoom::OnJoinPublicRoomError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to join chat topic with ID '%s' as the request to join failed on the backend! Error code: %d; Error message: %s"), *RoomId, ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatJoinPublicRoom::OnJoinPublicRoomSuccess(const FAccelByteModelsChatActionTopicResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TopicId: %s"), *Response.TopicId);

	// query topic to get members
	this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, TopicId = Response.TopicId]()
		{
			const FOnChatQueryRoomByIdComplete OnJoinPublicRoomSuccessDelegate =
				TDelegateUtils<FOnChatQueryRoomByIdComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatJoinPublicRoom::OnQueryTopicByIdAfterJoinRoomSuccess);
			const FErrorHandler OnJoinPublicRoomErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatJoinPublicRoom::OnJoinPublicRoomError);
			Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatQueryRoomById>(Subsystem, UserId.ToSharedRef().Get(), TopicId, OnJoinPublicRoomSuccessDelegate);
		}));
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatJoinPublicRoom::OnQueryTopicByIdAfterJoinRoomSuccess(bool bQueryRoomSuccess, FAccelByteChatRoomInfoPtr Result, int32 UserNum)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Local user %d, was successful %s"), UserNum, LOG_BOOL_FORMAT(bQueryRoomSuccess));

	if(!bQueryRoomSuccess)
	{
		UE_LOG_AB(Warning, TEXT("Failed to query room info after join success"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	JoinedRoomInfoPtr = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}