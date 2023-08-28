// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatSendRoomChat.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteChatSendRoomChat::FOnlineAsyncTaskAccelByteChatSendRoomChat(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FChatRoomId& InRoomId,
	const FString& InChatMessage)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RoomId(InRoomId)
	, ChatMessage(InChatMessage)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatSendRoomChat::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const AccelByte::Api::Chat::FSendChatResponse OnSendRoomChatSuccessDelegate =
		TDelegateUtils<AccelByte::Api::Chat::FSendChatResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatSendRoomChat::OnSendRoomChatSuccess);
	const FErrorHandler OnSendRoomChatErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatSendRoomChat::OnSendRoomChatError);

	ApiClient->Chat.SendChat(RoomId, ChatMessage, OnSendRoomChatSuccessDelegate, OnSendRoomChatErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatSendRoomChat::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send chat message as our chat interface instance is not valid!"));
		return;
	}

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send chat message as our user ID is not valid!"));
		return;
	}

	if (bWasSuccessful)
	{
		FAccelByteChatRoomMemberRef ChatRoomMember = ChatInterface->GetAccelByteChatRoomMember(UserId->GetAccelByteId());
		TSharedRef<FAccelByteChatMessage> OutChatMessage = MakeShared<FAccelByteChatMessage>(UserId.ToSharedRef(), ChatRoomMember->GetNickname(), ChatMessage, CreatedAt);
		ChatInterface->AddChatMessage(UserId.ToSharedRef(), RoomId, OutChatMessage);
	}

	ChatInterface->TriggerOnSendChatCompleteDelegates(UserId.ToSharedRef().Get().GetAccelByteId(), ChatMessage, RoomId, bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatSendRoomChat::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// TODO: Finalize? 
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatSendRoomChat::OnSendRoomChatError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to send chat to topic with ID '%s' as the request to send failed on the backend! Error code: %d; Error message: %s"), *RoomId, ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatSendRoomChat::OnSendRoomChatSuccess(const FAccelByteModelsChatSendChatResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TopicId: %s, ChatId: %s"), *Response.TopicId, *Response.ChatId);

	CreatedAt = Response.Processed;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
