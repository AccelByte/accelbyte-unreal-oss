// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatCreateRoom.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteChatCreateRoom::FOnlineAsyncTaskAccelByteChatCreateRoom(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FAccelByteChatRoomConfig& InChatRoomConfig)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, ChatRoomConfig(InChatRoomConfig)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatCreateRoom::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());
	
	TSet<FString> UserIds = { UserId->GetAccelByteId() };
	
	const AccelByte::Api::Chat::FChatActionTopicResponse OnCreateGroupTopicSuccessDelegate =
		TDelegateUtils<AccelByte::Api::Chat::FChatActionTopicResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatCreateRoom::OnCreateGroupTopicSuccess);
	const FErrorHandler OnCreateGroupTopicErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatCreateRoom::OnCreateGroupTopicError);

	// #NOTE Passing UserIds as both members and admins, since the creating user should be both
	Members = UserIds;
	Admins = UserIds;
	API_CLIENT_CHECK_GUARD(ErrorString);
	ApiClient->Chat.CreateGroupTopic(Members, Admins, ChatRoomConfig.FriendlyName, ChatRoomConfig.bIsJoinable, OnCreateGroupTopicSuccessDelegate, OnCreateGroupTopicErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatCreateRoom::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineChatAccelBytePtr ChatInterface;
	if(!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(), ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create chat room as our chat interface instance is not valid!"));
		return;
	}

	ChatInterface->TriggerOnChatRoomCreatedDelegates(UserId.ToSharedRef().Get(), RoomId, bWasSuccessful, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatCreateRoom::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineChatAccelBytePtr ChatInterface;
	if(!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create chat room as our chat interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful)
	{
		const FAccelByteChatRoomInfoRef ChatRoomInfo = FAccelByteChatRoomInfo::Create();
		if (ChatInterface->GetTopic(RoomId) == nullptr)
		{
			FAccelByteModelsChatTopicQueryData TopicData;
			TopicData.TopicId = RoomId;
			TopicData.Members.Add(UserId->GetAccelByteId());
			ChatRoomInfo->SetTopicData(TopicData);
			ChatInterface->AddTopic(ChatRoomInfo);
		}

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsChatV2GroupTopicCreatedPayload GroupTopicCreatedPayload{};
			GroupTopicCreatedPayload.UserId = UserId->GetAccelByteId();
			GroupTopicCreatedPayload.Members = Members;
			GroupTopicCreatedPayload.Admins = Admins;
			GroupTopicCreatedPayload.Name = ChatRoomConfig.FriendlyName;
			GroupTopicCreatedPayload.IsJoinable = ChatRoomConfig.bIsJoinable;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2GroupTopicCreatedPayload>(GroupTopicCreatedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatCreateRoom::OnCreateGroupTopicError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to create group chat topic with friendly name '%s' as the request to create failed on the backend! Error code: %d; Error message: %s"), *ChatRoomConfig.FriendlyName, ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatCreateRoom::OnCreateGroupTopicSuccess(const FAccelByteModelsChatActionTopicResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TopicId: %s"), *Response.TopicId);

	RoomId = Response.TopicId;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
