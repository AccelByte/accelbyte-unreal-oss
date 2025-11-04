// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncThirdPartyFriendV2.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2"

FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2::FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FAccelByteModelsSyncThirdPartyFriendsRequest& Request)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SyncRequest(Request)
{
	LocalUserNum = InLocalUserNum;
	OnlineError = FOnlineError();
}

void FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2, SyncThirdPartyFriends, THandler<TArray<FAccelByteModelsSyncThirdPartyFriendsResponse>>);
	API_FULL_CHECK_GUARD(Lobby, ErrorStr);
	Lobby->SyncThirdPartyFriends(SyncRequest, OnSyncThirdPartyFriendsSuccessDelegate, OnSyncThirdPartyFriendsErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *ErrorStr);

	const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());

	if(bWasSuccessful)
	{
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	}
	FriendInterface->TriggerOnSyncThirdPartyPlatformFriendsV2CompleteDelegates(LocalUserNum, OnlineError, SyncPlatformResponse);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2::OnSyncThirdPartyFriendsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	ErrorStr = TEXT("sync-other-platform-friend-failed-sync");
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorStr));
	
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to sync third party friend!", ErrorCode, ErrorMessage);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
void FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2::OnSyncThirdPartyFriendsSuccess(
	const TArray<FAccelByteModelsSyncThirdPartyFriendsResponse>& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	SyncPlatformResponse = Response;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
