// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteStatisticModels.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "OnlineSubsystemAccelBytePackage.h"

DECLARE_MULTICAST_DELEGATE_FourParams(FOnListUserStatItemsCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsFetchUser>&, const FString& /*Error*/);
typedef FOnListUserStatItemsCompleted::FDelegate FOnListUserStatItemsCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnUserStatItemsResetCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsUpdateUserStatItemsResponse>&, const FString& /*Error*/);
typedef FOnUserStatItemsResetCompleted::FDelegate FOnUserStatItemsResetCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUserStatItemsDeleteCompleted, const FOnlineError& /*ResultState*/);
typedef FOnUserStatItemsDeleteCompleted::FDelegate FOnUserStatItemsDeleteCompletedDelegate;

DECLARE_DELEGATE_TwoParams(FOnlineStatsCreateStatsComplete, const FOnlineError& /*ResultState*/, const TArray<FAccelByteModelsBulkStatItemOperationResult>& /*Result*/);

DECLARE_DELEGATE_TwoParams(FOnUpdateMultipleUserStatItemsComplete, const FOnlineError& /*ResultState*/, const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& /*Result*/);

/**
 * Implementation of Statistic service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineStatisticAccelByte
	: public IOnlineStats
	, public TSharedFromThis<FOnlineStatisticAccelByte, ESPMode::ThreadSafe>
{ 
PACKAGE_SCOPE:
	/* Map of Users */
	TUniqueNetIdMap<TArray<TSharedRef<FAccelByteModelsFetchUser>>> UsersMap;
	
	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineStatisticAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

public:
	virtual ~FOnlineStatisticAccelByte() override = default;

	bool ListUserStatItems(int32 LocalUserNum
		, const TArray<FString>& StatCodes
		, const TArray<FString>& Tags
		, const FString& AdditionalKey
		, bool bAlwaysRequestToService);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnListUserStatItemsCompleted, bool, const TArray<FAccelByteModelsFetchUser>&, const FString&);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnUserStatItemsResetCompleted, bool, const TArray<FAccelByteModelsUpdateUserStatItemsResponse>&, const FString&);

	/**
	 * Delegate fired when we delete one statistic for a player.
	 *
	 * @param ResultState The Result of statistic deletion process
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnUserStatItemsDeleteCompleted, const FOnlineError& /*ResultState*/);

	/* Get the list users stat items */
	// TODO : make this method as implementation from GetStats
	bool GetListUserStatItems(int32 LocalUserNum
		, TArray<TSharedRef<FAccelByteModelsFetchUser>>& OutUsers);
	
	TSharedPtr<const FOnlineStatsUserStats> GetAllListUserStatItemFromCache(const FUniqueNetIdRef StatsUserId) const;

	/**
	 * Query a specific user's stats
	 *
	 * @param LocalUserId User to query as (if applicable)
	 * @param StatsUser User to get stats for
	 * @param StatNames Stats to get stats for all specified users
	 * @param Delegate Called when the user's stats have finished being requested and are now available, or when we fail to retrieve the user's stats
	 */
	void QueryStats(const FUniqueNetIdRef LocalUserId
		, const FUniqueNetIdRef StatsUser
		, const TArray<FString>& StatNames
		, const FOnlineStatsQueryUserStatsComplete& Delegate);

	/**
	 * Query a specific user's stats
	 *
	 * @param LocalUserNum Index of user(or server) to query as
	 * @param StatsUser User to get stats for
	 * @param StatNames Stats to get stats for all specified users
	 * @param Delegate Called when the user's stats have finished being requested and are now available, or when we fail to retrieve the user's stats
	 */
	virtual void QueryStats(int32 LocalUserNum
		, const FUniqueNetIdRef StatsUser
		, const TArray<FString>& StatNames
		, const FOnlineStatsQueryUserStatsComplete& Delegate);

	/**
	 * Query a specific user's stats
	 *
	 * @param LocalUserNum Index of user(or server) to query as
	 * @param StatsUser User to get stats for
	 * @param Delegate Called when the user's stats have finished being requested and are now available, or when we fail to retrieve the user's stats
	 */
	virtual void QueryStats(int32 LocalUserNum
		, const FUniqueNetIdRef StatsUser
		, const FOnlineStatsQueryUserStatsComplete& Delegate);

	/**
	 * Query a one or more user's stats
	 *
	 * @param LocalUserNum Index of user(or server) to query as
	 * @param StatsUsers Users to get stats for
	 * @param StatNames Stats to get stats for all specified users
	 * @param Delegate Called when the user's stats have finished being requested and are now available, or when we fail to retrieve the user's stats
	 */
	virtual void QueryStats(int32 LocalUserNum
		, const TArray<FUniqueNetIdRef>& StatsUsers
		, const TArray<FString>& StatNames
		, const FOnlineStatsQueryUsersStatsComplete& Delegate);

	/**
	 * Create a one or more user's stats. Only for request by Game Server
	 * 
	 * @param LocalUserNum Index of user(server) that is attempting to create the stats
	 * @param StatsUser User to create stats for
	 * @param StatNames Stats to create stats for all specified users
	 * @param Delegate Called when the user's stats have finished being created, or when we fail to create the user's stats
	 */
	virtual void CreateStats(const int32 LocalUserNum
		, const FUniqueNetIdRef StatsUser
		, const TArray<FString>& StatNames
		, const FOnlineStatsCreateStatsComplete& Delegate);
	
	/**
	 * Emplace a user's cached stats object
	 *
	 * @param UserStats The stats to emplace
	 */
	virtual void EmplaceStats(const TSharedPtr<const FOnlineStatsUserStats>& InUserStats);

	/**
	 * Emplace user's cached stats object
	 *
	 * @param UsersStats The stats to emplace
	 */
	virtual void EmplaceStats(const TArray<TSharedPtr<const FOnlineStatsUserStats>>& InUsersStats);

	/**
	 * Remove user's cached stats object
	 *
	 * @param StatsUserId User to remove stats for
	 * @param StatsCode The stats id to remove
	 */
	virtual void RemoveStats(const FUniqueNetIdRef StatsUserId
		, const FString& StatsCode);

	/**
	 * Reset all user statistics.
	 *
	 * @param LocalUserNum Index of user(server) that is attempting to create the stats
	 * @param StatsUserId User to reset stats for
	 */
	virtual void ResetStats(const int32 LocalUserNum
		, const FUniqueNetIdRef StatsUserId);

	/**
	 * Update multiple statistics for multiple users. This request only for Game Server
	 *
	 * @param LocalUserNum Index of user(server) that is attempting to update the stats.
	 * @param BulkUpdateMultipleUserStatItems Updated Statistics.
	 * @param Delegate Called when the statistics have finished being updated, or when we fail to update the stats
	 */
	virtual void UpdateStats(const int32 LocalUserNum
		, const TArray<FOnlineStatsUserUpdatedStats>& BulkUpdateMultipleUserStatItems
		, const FOnUpdateMultipleUserStatItemsComplete& Delegate);

	/**
	 * Update multiple statistics for a user. This request only for Game Client
	 * This endpoint extends from 
	 * UpdateStats(const FUniqueNetIdRef LocalUserId, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats, const FOnlineStatsUpdateStatsComplete& Delegate)
	 * Which is providing the messages in 'Details' field after receiving the response.
	 *
	 * @param LocalUserId User to query as (if applicable)
	 * @param UpdatedUserStats Array of Updated Statistics.
	 * @param Delegate Called when the statistics have finished being updated, or when we fail to update the stats
	 */
	virtual void UpdateStats(const FUniqueNetIdRef LocalUserId, 
		const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats, 
		const FOnUpdateMultipleUserStatItemsComplete& Delegate);

	/**
	 * Delete one statistic for one user. This request only for Game Server
	 *
	 * @param LocalUserNum Index of user(server) that is attempting to delete the stats.
	 * @param StatsUserId User to delete stat for.
	 * @param StatsCode The stats id to remove.
	 * @param AdditionalKey The AdditionalKey relate to statistic.
	 */
	virtual void DeleteStats(const int32 LocalUserNum
		, const FUniqueNetIdRef StatsUserId
		, const FString& StatsCode
		, const FString & AdditionalKey);

	//~ Begin IOnlineStats Interface
	virtual void QueryStats(const FUniqueNetIdRef LocalUserId
		, const FUniqueNetIdRef StatsUser
		, const FOnlineStatsQueryUserStatsComplete& Delegate) override;
	
	virtual void QueryStats(const FUniqueNetIdRef LocalUserId
		, const TArray<FUniqueNetIdRef>& StatsUsers
		, const TArray<FString>& StatsNames
		, const FOnlineStatsQueryUsersStatsComplete& Delegate) override;
	
	virtual TSharedPtr<const FOnlineStatsUserStats> GetStats(const FUniqueNetIdRef StatsUserId) const override;
	
	virtual void UpdateStats(const FUniqueNetIdRef LocalUserId
		, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats
		, const FOnlineStatsUpdateStatsComplete& Delegate) override;

#if !UE_BUILD_SHIPPING
	virtual void ResetStats(const FUniqueNetIdRef StatsUserId) override;
#endif // !UE_BUILD_SHIPPING
	//~ End  IOnlineStats Interface

	// Statistic Utils

	/**
	 * Convert Unreal built-in enum to Accelbyte enum for statistic update strategy
	 *
	 * @param Strategy the strategy to update the statistic
	 */
	static EAccelByteStatisticUpdateStrategy ConvertUpdateStrategy(FOnlineStatUpdate::EOnlineStatModificationType Strategy);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineStatisticAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

private :
	
	TSharedPtr<const FOnlineStatsUserStats> UserStats;
	/** Critical sections for thread safe operation of UsersStats */
	mutable FCriticalSection StatsLock;
	TArray<TSharedRef<const FOnlineStatsUserStats>> UsersStats;
};