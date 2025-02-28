// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRescindFriendInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRescindFriendInvite::FOnlineAsyncTaskAccelByteRescindFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnSendInviteComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendId(FUniqueNetIdAccelByteUser::CastChecked(InFriendId))
	, ListName(InListName)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteRescindFriendInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d; FriendId: %s"), LocalUserNum, *FriendId->ToDebugString());

	OnCancelFriendRequestSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRescindFriendInvite::OnCancelFriendRequestSuccess);
	OnCancelFriendRequestFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRescindFriendInvite::OnCancelFriendRequestFailed);
	API_FULL_CHECK_GUARD(Lobby, ErrorStr);
	Lobby->CancelFriendRequest(FriendId->GetAccelByteId(), OnCancelFriendRequestSuccessDelegate, OnCancelFriendRequestFailedDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRescindFriendInvite::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRescindFriendInvite::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	// If the request was successful, then we want to get rid of this user from our friends list
	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());
		FriendInterface->RemoveFriendFromList(LocalUserNum, FriendId);
	}
	
	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, FriendId.Get(), ListName, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRescindFriendInvite::OnCancelFriendRequestSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRescindFriendInvite::OnCancelFriendRequestFailed(int32 ErrorCode,	const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());
	ErrorStr = TEXT("friend-rescind-request-failed");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel friend invate to user %s! Error code: %d. Error message: %s"), *FriendId->ToDebugString(), ErrorCode, *ErrorMessage);
}
