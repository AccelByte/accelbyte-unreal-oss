// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineUserCacheAccelByte.h"

/**
 * Task used by real time notification methods to add a new friend entry to the friends list based on invite status
 */
class FOnlineAsyncTaskAccelByteAddFriendToList
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteAddFriendToList, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteAddFriendToList(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendOwnerId, const FUniqueNetId& InFriendId, const EInviteStatus::Type& InInviteStatus);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskAccelByteAddFriendToList (bWasSuccessful: %s)"), LOG_BOOL_FORMAT(bWasSuccessful)); }

private:

	/** Id of the user that owns the list we want to add a friend to */
	TSharedRef<const FUniqueNetIdAccelByteUser> FriendOwnerId;

	/** Id of the friend that we are adding to the user's friends list */
	TSharedRef<const FUniqueNetIdAccelByteUser> FriendId;

	/** Status of the invite for the user, determines which delegates will be fired */
	EInviteStatus::Type InviteStatus;

	/** Shared pointer instance of the friend that we constructed for the list */
	TSharedPtr<FOnlineFriend> FriendObject;

	/** Delegate handler for when we complete a query for friend information */
	void OnQueryFriendComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo, ESPMode::ThreadSafe>> UsersQueried);

};

