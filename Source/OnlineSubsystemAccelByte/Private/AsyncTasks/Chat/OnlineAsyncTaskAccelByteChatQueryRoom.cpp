// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatQueryRoom.h"

FOnlineAsyncTaskAccelByteChatQueryRoom::FOnlineAsyncTaskAccelByteChatQueryRoom(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FAccelByteModelsChatQueryTopicRequest& InQuery,
	const FOnChatQueryRoomComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Query(InQuery)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatQueryRoom::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const AccelByte::Api::Chat::FQueryTopicResponse OnQueryRoomSuccessDelegate =
		AccelByte::Api::Chat::FQueryTopicResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteChatQueryRoom::OnQueryRoomSuccess);
	const FErrorHandler OnQueryRoomErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteChatQueryRoom::OnQueryRoomError);

	ApiClient->Chat.QueryTopic(Query, OnQueryRoomSuccessDelegate, OnQueryRoomErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoom::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineChatAccelBytePtr ChatInterface;
	if (ensure(FOnlineChatAccelByte::GetFromSubsystem(Subsystem, ChatInterface)))
	{
		if (bWasSuccessful)
		{
			ChatInterface->AddChatRoomMembers(FoundUsers);
		}

		for (const auto& Room : ChatRoomInfoList)
		{
			ChatInterface->AddTopic(Room);
		}
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set chat room member chat interface instance is not valid!"));
	}

	Delegate.ExecuteIfBound(bWasSuccessful, ChatRoomInfoList, LocalUserNum);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoom::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoom::OnQueryRoomError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to query room. Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatQueryRoom::OnQueryRoomSuccess(const FAccelByteModelsChatQueryTopicResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Processed: %s, Length: %d"), *Response.Processed.ToIso8601(), Response.Data.Num());
	SetLastUpdateTimeToCurrentTime();
	
	TArray<FString> UserIds;
	for (const auto& TopicData : Response.Data)
	{
		FAccelByteChatRoomInfoRef ChatRoomInfo = FAccelByteChatRoomInfo::Create();
		ChatRoomInfo->SetTopicData(TopicData);
		ChatRoomInfoList.Add(ChatRoomInfo);
		
		for (const auto& Id : ChatRoomInfo->GetMembers())
		{
			UserIds.AddUnique(Id);
		}
	}

	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query chat room as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnQueryUsersComplete OnQueryUsersCompleteDelegate = FOnQueryUsersComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteChatQueryRoom::OnQueryMemberInformationComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, UserIds, OnQueryUsersCompleteDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoom::OnQueryMemberInformationComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Length: %d"), UsersQueried.Num());
	SetLastUpdateTimeToCurrentTime();

	FoundUsers = UsersQueried;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
