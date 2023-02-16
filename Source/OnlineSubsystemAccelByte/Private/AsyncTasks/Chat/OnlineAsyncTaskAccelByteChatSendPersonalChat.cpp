// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatSendPersonalChat.h"

FOnlineAsyncTaskAccelByteChatSendPersonalChat::FOnlineAsyncTaskAccelByteChatSendPersonalChat(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FUniqueNetId& InRecipientId,
	const FString& InChatMessage)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RecipientId(FUniqueNetIdAccelByteUser::CastChecked(InRecipientId))
	, ChatMessage(InChatMessage)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	RoomId = FOnlineChatAccelByte::PersonalChatTopicId(UserId->GetAccelByteId(), RecipientId->GetAccelByteId());

	FOnlineChatAccelBytePtr ChatInterface;
	if(!ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send personal chat as our chat interface instance is not valid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (ChatInterface->HasPersonalChat(UserId->GetAccelByteId(), RecipientId->GetAccelByteId()))
	{
		SendPersonalChat();
	}
	else
	{
		CreatePersonalTopic();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::TriggerDelegates()
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

	ChatInterface->TriggerOnSendChatCompleteDelegates(UserId.ToSharedRef().Get().GetAccelByteId(), ChatMessage, RoomId, bWasSuccessful);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if(bWasSuccessful)
	{
		FOnlineChatAccelBytePtr ChatInterface;
		if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send chat message as our chat interface instance is not valid!"));
			return;
		}
		FAccelByteChatRoomMemberRef Member = ChatInterface->GetAccelByteChatRoomMember(UserId->GetAccelByteId());
		TSharedRef<FAccelByteChatMessage> OutChatMessage = MakeShared<FAccelByteChatMessage>(UserId.ToSharedRef(), Member->GetNickname(), ChatMessage, CreatedAt);
		ChatInterface->AddChatMessage(UserId.ToSharedRef(), RoomId, OutChatMessage);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::CreatePersonalTopic()
{
	const AccelByte::Api::Chat::FChatActionTopicResponse OnCreatePersonalTopicSuccessDelegate =
		TDelegateUtils<AccelByte::Api::Chat::FChatActionTopicResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnCreatePersonalTopicSuccess);
	const FErrorHandler OnCreatePersonalTopicErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnCreatePersonalTopicError);

	ApiClient->Chat.CreatePersonalTopic(RecipientId->GetAccelByteId(), OnCreatePersonalTopicSuccessDelegate, OnCreatePersonalTopicErrorDelegate);
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::SendPersonalChat()
{
	const AccelByte::Api::Chat::FSendChatResponse OnSendRoomChatSuccessDelegate =
		TDelegateUtils<AccelByte::Api::Chat::FSendChatResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnSendRoomChatSuccess);
	const FErrorHandler OnSendRoomChatErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnSendRoomChatError);

	ApiClient->Chat.SendChat(RoomId, ChatMessage, OnSendRoomChatSuccessDelegate, OnSendRoomChatErrorDelegate);
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnSendRoomChatError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to send chat to topic with ID '%s' as the request to send failed on the backend! Error code: %d; Error message: %s"), *RoomId, ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnSendRoomChatSuccess(const FAccelByteModelsChatSendChatResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TopicId: %s, ChatId: %s"), *Response.TopicId, *Response.ChatId);

	CreatedAt = Response.Processed;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnCreatePersonalTopicError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to create personal chat topic with recepient '%s' as the request to create failed on the backend! Error code: %d; Error message: %s"), *RecipientId->GetAccelByteId(), ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatSendPersonalChat::OnCreatePersonalTopicSuccess(const FAccelByteModelsChatActionTopicResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TopicId: %s"), *Response.TopicId);

	FOnlineChatAccelBytePtr ChatInterface;
	if (ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
	{
		// Put into cache if not yet cached
		if (ChatInterface->GetTopic(RoomId) == nullptr)
		{
			const FAccelByteChatRoomInfoRef ChatRoomInfo = FAccelByteChatRoomInfo::Create();
			FAccelByteModelsChatTopicQueryData TopicData;
			TopicData.Members.Add(UserId->GetAccelByteId());
			TopicData.Members.Add(RecipientId->GetAccelByteId());
			TopicData.TopicId = RoomId;
			ChatRoomInfo->SetTopicData(TopicData);
			ChatInterface->AddTopic(ChatRoomInfo);
		}
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set topic cache as chat interface instance is not valid!"));
	}
	
	SendPersonalChat();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}