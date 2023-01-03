// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatConfigureRoom.h"

FOnlineAsyncTaskAccelByteChatConfigureRoom::FOnlineAsyncTaskAccelByteChatConfigureRoom(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FChatRoomId& InRoomId,
	const FAccelByteChatRoomConfig& InChatRoomConfig)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RoomId(InRoomId)
	, ChatRoomConfig(InChatRoomConfig)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatConfigureRoom::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const AccelByte::Api::Chat::FChatActionTopicResponse OnUpdateTopicSuccessDelegate =
		AccelByte::Api::Chat::FChatActionTopicResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteChatConfigureRoom::OnUpdateTopicSuccess);
	const FErrorHandler OnUpdateTopicErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteChatConfigureRoom::OnUpdateTopicError);

	ApiClient->Chat.UpdateTopic(RoomId, ChatRoomConfig.FriendlyName, ChatRoomConfig.bIsJoinable, OnUpdateTopicSuccessDelegate, OnUpdateTopicErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatConfigureRoom::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create chat room as our chat interface instance is not valid!"));
		return;
	}

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create chat room as our user ID is not valid!"));
		return;
	}

	ChatInterface->TriggerOnChatRoomConfiguredDelegates(GenericUserId.ToSharedRef().Get(), RoomId, bWasSuccessful, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatConfigureRoom::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// TODO: Finalize?
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatConfigureRoom::OnUpdateTopicError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to update chat topic with ID '%s' as the request to update failed on the backend! Error code: %d; Error message: %s"), *RoomId, ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatConfigureRoom::OnUpdateTopicSuccess(const FAccelByteModelsChatActionTopicResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TopicId: %s"), *Response.TopicId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
