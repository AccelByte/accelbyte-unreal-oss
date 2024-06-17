// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAddFriendToList.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineUserCacheAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteAddFriendToList::FOnlineAsyncTaskAccelByteAddFriendToList(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendOwnerId, const FUniqueNetId& InFriendId, const EInviteStatus::Type& InInviteStatus)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendOwnerId(FUniqueNetIdAccelByteUser::CastChecked(InFriendOwnerId))
	, FriendId(FUniqueNetIdAccelByteUser::CastChecked(InFriendId))
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

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		FOnQueryUsersComplete OnQueryFriendComplete = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAddFriendToList::OnQueryFriendComplete);
		UserStore->QueryUsersByAccelByteIds(LocalUserNum, { FriendId->GetAccelByteId() }, OnQueryFriendComplete, true);
	}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAddFriendToList::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		if (ensure(FriendsInterface.IsValid()))
		{
			FriendsInterface->AddFriendToList(LocalUserNum, FriendObject);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAddFriendToList::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	if (!ensure(FriendsInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for adding a friend to local friends list as our friends interface is invalid!"));
		return;
	}

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

void FOnlineAsyncTaskAccelByteAddFriendToList::OnQueryFriendComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo, ESPMode::ThreadSafe>> UsersQueried)
{
	if (bIsSuccessful && UsersQueried.IsValidIndex(0))
	{
		TSharedRef<FAccelByteUserInfo, ESPMode::ThreadSafe>& Friend = UsersQueried[0];
		FriendObject = MakeShared<FOnlineFriendAccelByte>(Friend, InviteStatus);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to get information about friend %s!"), *FriendId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}
