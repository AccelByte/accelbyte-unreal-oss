// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineUserCacheAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Core/Platform/AccelBytePlatformHandleModels.h"

class FOnlineAsyncTaskAccelByteSyncPlatformFriend
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncPlatformFriend, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSyncPlatformFriend(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InLocalUserNetId
		, const EAccelBytePlatformType InNativePlatform);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncPlatformFriend");
	}

private:
	/** Name of native platform. */
	EAccelBytePlatformType NativePlatform{EAccelBytePlatformType::None};

	/**
	 * Array of IDs that's split to Max element count.
	 */
	TArray<TArray<FString>> SplitUserIds;

	/**
	 * Array of user IDs returned from query by platform user IDs.
	 */
	TArray<FPlatformUserIdMap> QueriedUserMapByPlatformUserIds;

	/**
	 * Index of the split user IDs we last fetch.
	 */
	FThreadSafeCounter LastSplitQueryIndex {0};

	/**
	 * Array of shared pointer to the friends instance that we are synced to,
	 * only gets passed to the friend interface if the sync was successful,
	 * otherwise this just gets cleaned up with the task.
	 */
	TArray<TSharedPtr<FOnlineFriend>> SyncedFriends;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Function to query native friend list based on current platform. */
	bool QueryPlatformFriendList();

	/** Handler for QueryPlatformFriendList success. */
	void OnQueryPlatformFriendListSuccess(const TArray<AccelByte::FPlatformUser>& PlatformFriends);

	/** Handler for QueryPlatformFriendList error. */
	void OnQueryPlatformFriendListError(const AccelByte::FPlatformHandlerError& Response);

	/**
	* Method to query User by Other Platform with an array of User IDs.
	*/
	void BulkGetUserByOtherPlatformUserIds(const TArray<FString>& InUserIds);

	/** Handler for successful bulk query of AccelByte user id based on 3rd party user id. */
	void OnBulkGetUserByOtherPlatformUserIdsSuccess(const FBulkPlatformUserIdResponse& Result);

	/** Handler for failed bulk query of AccelByte user id based on 3rd party user id. */
	void OnBulkGetUserByOtherPlatformUserIdsError(int32 ErrorCode, const FString& ErrorMessage);

	/** Handler for completed a query for user information from UserStore interface. */
	void OnQuerySyncedFriendComplete(const bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried);

	/** Handler for successful adding 3rd party platform friend to current user. */
	void OnBulkFriendRequestSuccess();

	/** Handler for failed adding 3rd party platform friend to current user. */
	void OnBulkFriendRequestError(int32 ErrorCode, const FString& ErrorMessage);
};
