// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncThirdPartyBlockList.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList"

FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList::FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, const FAccelByteModelsSyncThirdPartyBlockListRequest& Request)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SyncRequest(Request)
{
	LocalUserNum = InLocalUserNum;
	OnlineError = FOnlineError();
}

void FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if (!ensure(!IsRunningDedicatedServer()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Unable to sync third party block list from a server! Must be called on a client."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList, SyncThirdPartyBlockList, THandler<TArray<FAccelByteModelsSyncThirdPartyBlockListResponse>>);
	ApiClient->Lobby.SyncThirdPartyBlockList(SyncRequest
		, OnSyncThirdPartyBlockListSuccessDelegate
		, OnSyncThirdPartyBlockListErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *ErrorText.ToString());

	const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());

	if (bWasSuccessful)
	{
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	}

	FriendInterface->TriggerOnSyncThirdPartyBlockListCompleteDelegates(LocalUserNum, OnlineError, SyncPlatformResponse);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList::OnSyncThirdPartyBlockListSuccess(
	const TArray<FAccelByteModelsSyncThirdPartyBlockListResponse>& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	SyncPlatformResponse = Response;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList::OnSyncThirdPartyBlockListError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	ErrorText = FText::FromString(TEXT("sync-other-platform-block-list-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to sync third party block list!", ErrorCode, ErrorMessage);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
