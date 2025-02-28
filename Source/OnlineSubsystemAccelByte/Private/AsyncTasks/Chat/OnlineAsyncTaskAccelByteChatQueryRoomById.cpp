// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatQueryRoomById.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteChatQueryRoomById::FOnlineAsyncTaskAccelByteChatQueryRoomById(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId,
	const FChatRoomId& InRoomId,
	const FOnChatQueryRoomByIdComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RoomId(InRoomId)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatQueryRoomById::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const AccelByte::Api::Chat::FQueryTopicByIdResponse OnQueryRoomSuccessDelegate =
		TDelegateUtils<AccelByte::Api::Chat::FQueryTopicByIdResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatQueryRoomById::OnQueryRoomSuccess);
	const FErrorHandler OnQueryRoomErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatQueryRoomById::OnQueryRoomError);

	API_FULL_CHECK_GUARD(Chat, ErrorString);
	Chat->QueryTopicById(RoomId, OnQueryRoomSuccessDelegate, OnQueryRoomErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoomById::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	
	Delegate.ExecuteIfBound(bWasSuccessful, RoomInfo, LocalUserNum);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoomById::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineChatAccelBytePtr ChatInterface;
	if (ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(), ChatInterface)))
	{
		if (bWasSuccessful)
		{
			ChatInterface->AddChatRoomMembers(FoundUsers);
		}
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set chat room member chat interface instance is not valid!"));
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoomById::OnQueryRoomError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to query room. Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteChatQueryRoomById::OnQueryRoomSuccess(const FAccelByteModelsChatQueryTopicByIdResponse& Response)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Processed: %s"), *Response.Processed.ToIso8601());

	SetLastUpdateTimeToCurrentTime();

	RoomInfo = FAccelByteChatRoomInfo::Create();
	RoomInfo->SetTopicData(Response.Data);

	FOnlineChatAccelBytePtr ChatInterface;
	if (ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(), ChatInterface)))
	{
		// add to cache immediately to avoid race condition
		ChatInterface->AddTopic(RoomInfo.ToSharedRef());
	}
	
	TArray<FString> UserIds;
	for (const auto& Id : Response.Data.Members)
	{
		UserIds.AddUnique(Id);
	}

	FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query chat room by id %s as our user store instance is invalid!"), *RoomId);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// query topic to get members
	this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
		{
			FOnQueryUsersComplete OnQueryUsersCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatQueryRoomById::OnQueryMemberInformationComplete);
			UserStore->QueryUsersByAccelByteIds(LocalUserNum, UserIds, OnQueryUsersCompleteDelegate);
		}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryRoomById::OnQueryMemberInformationComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Length: %d"), UsersQueried.Num());
	SetLastUpdateTimeToCurrentTime();

	FoundUsers = UsersQueried;
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
