// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteUserModels.h"

/**
 * Task to query a bulk of users by AccelByte or platform IDs, will add these users to the user cache.
 */
class FOnlineAsyncTaskAccelByteQueryUsersByIds
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryUsersByIds, ESPMode::ThreadSafe>
{
public:
	/**
	 * Queries a bulk of AccelByte IDs using a local user index
	 */
	FOnlineAsyncTaskAccelByteQueryUsersByIds(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const TArray<FString>& AccelByteIds, bool InBIsImportant, const FOnQueryUsersComplete& InDelegate);

	/**
	 * Queries a bulk of platform IDs to attempt to get AccelByte accounts using a local user index
	 */
	FOnlineAsyncTaskAccelByteQueryUsersByIds(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString InPlatformType, const TArray<FString>& PlatformIds, bool InBIsImportant, const FOnQueryUsersComplete& InDelegate);

	/**
	 * Queries a bulk of AccelByte IDs using a user ID
	 */
	FOnlineAsyncTaskAccelByteQueryUsersByIds(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const TArray<FString>& AccelByteIds, bool InBIsImportant, const FOnQueryUsersComplete& InDelegate);

	/**
	 * Queries a bulk of platform IDs to attempt to get AccelByte accounts using a user ID
	 */
	FOnlineAsyncTaskAccelByteQueryUsersByIds(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString InPlatformType, const TArray<FString>& PlatformIds, bool InBIsImportant, const FOnQueryUsersComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryUsersByIds");
	}

private:

	/**
	 * Type of platform that we are querying these users from.
	 * Will be set to ACCELBYTE if these are all AccelByte IDs, or the specific platform name otherwise.
	 */
	FString PlatformType;

	/**
	 * Array of IDs that we will query
	 */
	TArray<FString> UserIds;

	/**
	 * Array of IDs that's split to Max element count.
	 */
	TArray<TArray<FString>> SplitUserIds;

	/**
	 * Array of user IDs returned from query by platform user IDs
	 */
	TArray<FPlatformUserIdMap> QueriedUserMapByPlatformUserIds;

	/**
	 * Index of the split user IDs we last fetch
	 */
	FThreadSafeCounter LastSplitQueryIndex {0};

	/**
	 * Whether all of these users that we are querying will be marked as important.
	 */
	bool bIsImportant;

	/**
	 * Delegate that will be fired once the queries complete
	 */
	FOnQueryUsersComplete Delegate;

	/**
	 * Actual array of user IDs that will be queried on the backend
	 */
	TQueue<FString> UsersToQuery;

	/**
	 * Native platform Ids to be queried using native OSS
	 */
	TArray<TSharedRef<const FUniqueNetId>> PlatformIdsToQuery;

	/**
	 * Struct of user IDs and platform ID that will be queried on the backend
	 */
	FPlatformAccountInfoRequest PlatformAccountsInfoRequest;

	/**
	 * Array of users that we were able to query from the backend
	 */
	TArray<FAccelByteUserInfoRef> UsersQueried;

	/**
	 * Array of users that we already had cached in our user store
	 */
	TArray<FAccelByteUserInfoRef> UsersCached;

	/**
	 * Flag representing whether we have finished querying basic user info
	 */
	FThreadSafeBool bHasQueriedBasicUserInfo = false;

	/**
	 * Flag representing whether we have finished querying platform user information
	 */
	FThreadSafeBool bHasQueriedUserPlatformInfo = false;

	/**
	 * Delegate handler for when querying platform ID mappings in bulk succeeds
	 */
	void OnBulkQueryPlatformIdMappingsSuccess(const FBulkPlatformUserIdResponse& Result);

	/**
	 * Delegate handler for when querying platform ID mappings in bulk fails
	 */
	void OnBulkQueryPlatformIdMappingsError(int32 ErrorCode, const FString& ErrorMessage);

	/**
	 * Calls method to get basic user information by an array of AccelByte IDs
	 */
	void GetBasicUserInfo(const TArray<FString>& AccelByteIds);

	/*
	 * Query users basic info using low level SDK (only users that is not in cache).
	 */
	void GetUserOtherPlatformBasicPublicInfo();

	/**
	 * Delegate handler for when querying basic user information by AccelByte IDs succeeds
	 */
	void OnGetUserOtherPlatformBasicPublicInfoSuccess(const FAccountUserPlatformInfosResponse& Result);

	/**
	 * Delegate handler for when querying basic user information by platform IDs fails
	 */
	void OnGetUserOtherPlatformBasicPublicInfoError(int32 ErrorCode, const FString& ErrorMessage);

	/**
	 * Make a call to query the user manually on the platform that corresponds to the one we are currently on
	 */
	void QueryUsersOnNativePlatform(const TArray<TSharedRef<const FUniqueNetId>>& PlatformUniqueIds);

	/**
	 * Method to attempt to extract platform type and ID that matches current platform OSS from basic info structure into
	 * a composite ID structure that we are creating.
	 */
	void ExtractPlatformDataFromBasicUserInfo(const FAccountUserPlatformData& BasicInfo, FAccelByteUniqueIdComposite& CompositeId);

	/**
	* Method to query User by Other Platform with an array of User IDs
	*/
	void BulkGetUserByOtherPlatformUserIds(const TArray<FString>& InUserIds);
};
