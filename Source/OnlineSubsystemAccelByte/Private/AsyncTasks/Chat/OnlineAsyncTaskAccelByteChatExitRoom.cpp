// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatExitRoom.h"

#include "OnlineChatInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteChatExitRoom::FOnlineAsyncTaskAccelByteChatExitRoom(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FChatRoomId& InRoomId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RoomId(InRoomId)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatExitRoom::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const AccelByte::Api::Chat::FChatActionTopicResponse OnExitRoomSuccessDelegate =
		TDelegateUtils<AccelByte::Api::Chat::FChatActionTopicResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatExitRoom::OnExitRoomSuccess);
	const FErrorHandler OnExitRoomErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatExitRoom::OnExitRoomError);

	API_FULL_CHECK_GUARD(Chat, ErrorString);
	Chat->QuitTopic(RoomId, OnExitRoomSuccessDelegate, OnExitRoomErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatExitRoom::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to exit chat room as our chat interface instance is not valid!"));
		return;
	}
	ChatInterface->TriggerOnChatRoomExitDelegates(UserId.ToSharedRef().Get(), RoomId, bWasSuccessful, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatExitRoom::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to exit chat room as our chat interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful)
	{
		ChatInterface->RemoveTopic(RoomId);
		
		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsChatV2TopicQuitPayload TopicQuitPayload{};
			TopicQuitPayload.UserId = UserId->GetAccelByteId();
			TopicQuitPayload.TopicId = RoomId;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2TopicQuitPayload>(TopicQuitPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatExitRoom::OnExitRoomError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to leave chat topic with ID '%s' as the request to quit failed on the backend! Error code: %d; Error message: %s"), *RoomId, ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatExitRoom::OnExitRoomSuccess(const FAccelByteModelsChatActionTopicResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TopicId: %s"), *Response.TopicId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
