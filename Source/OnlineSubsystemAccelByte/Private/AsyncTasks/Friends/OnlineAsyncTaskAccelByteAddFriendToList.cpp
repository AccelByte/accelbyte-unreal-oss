// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAddFriendToList.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineUserCacheAccelByte.h"

FOnlineAsyncTaskAccelByteAddFriendToList::FOnlineAsyncTaskAccelByteAddFriendToList(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendOwnerId, const FUniqueNetId& InFriendId, const EInviteStatus::Type& InInviteStatus)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendOwnerId(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InFriendOwnerId.AsShared()))
	, FriendId(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InFriendId.AsShared()))
	, InviteStatus(InInviteStatus)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteAddFriendToList::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FriendId: %s"), *FriendOwnerId->ToDebugString(), *FriendId->ToDebugString());

	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not add friend '%s' to friends list as our user store instance is invalid!"), *FriendId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnQueryUsersComplete OnQueryFriendComplete = FOnQueryUsersComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteAddFriendToList::OnQueryFriendComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, { FriendId->GetAccelByteId() }, OnQueryFriendComplete, true);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAddFriendToList::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendsInterface->AddFriendToList(LocalUserNum, FriendObject);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAddFriendToList::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	ensure(FriendsInterface.IsValid());

	if (bWasSuccessful && InviteStatus == EInviteStatus::Accepted)
	{
		FriendsInterface->TriggerOnInviteAcceptedDelegates(FriendOwnerId.Get(), FriendId.Get());
	}
	else if (bWasSuccessful && InviteStatus == EInviteStatus::PendingInbound)
	{
		FriendsInterface->TriggerOnInviteReceivedDelegates(FriendOwnerId.Get(), FriendId.Get());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAddFriendToList::OnQueryFriendComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	if (bIsSuccessful && UsersQueried.IsValidIndex(0))
	{
		TSharedRef<FAccelByteUserInfo>& Friend = UsersQueried[0];
		FriendObject = MakeShared<FOnlineFriendAccelByte>(Friend->DisplayName, Friend->Id.ToSharedRef(), InviteStatus);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to get information about friend %s!"), *FriendId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}
