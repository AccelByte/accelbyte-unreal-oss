// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Core/AccelByteApiClient.h"
#include "Models/AccelByteLeaderboardModels.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineSubsystemAccelBytePackage.h"

/**
 * Index of the maximum player id that can be processed 
 */
static constexpr int32 LeaderboardUserIdsLimit = 20;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineLeaderboardAccelByte
	: public IOnlineLeaderboards
	, public TSharedFromThis<FOnlineLeaderboardAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineLeaderboardAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

public:
	virtual ~FOnlineLeaderboardAccelByte() override = default;

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(
		const IOnlineSubsystem* Subsystem,
		TSharedPtr<FOnlineLeaderboardAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(
		const UWorld* World,
		TSharedPtr<FOnlineLeaderboardAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Query multiple ranks for multiple users. This request only for Game Client.
	 * Note:
	 *		1. This endpoint only gives Cycle type leaderboard.
	 *		2. Please use 'Cycle_Point' to name the point column.
	 *
	 * @param Players Array of user to get rank for
	 * @param ReadObject This will contain the request and results when the operation completes.
	 * @param CycleId The cycle id that player want to know.
	 */
	virtual bool ReadLeaderboardsCycle(
		TArray<FUniqueNetIdRef> const& Players,
		FOnlineLeaderboardReadRef& ReadObject,
		FString const& CycleId);

	/**
	 * Query all user friends' ranks. This request only for Game Client.
	 * Note:
	 *		1. This endpoint only gives Cycle type leaderboard.
	 *		2. Please use 'Cycle_Point' to name the point column.
	 *
	 * @param LocalUserNum Index of user that is attempting to query the rank.
	 * @param ReadObject This will contain the request and results when the operation completes.
	 * @param CycleId The cycle id that player want to know.
	 */
	virtual bool ReadLeaderboardsForFriendsCycle(
		int32 LocalUserNum,
		FOnlineLeaderboardReadRef& ReadObject,
		FString const& CycleId);

	//~ Begin IOnlineAchievement Interface

	/**
	 * Query multiple ranks for multiple users. This request only for Game Client.
	 * Note:
	 *		1. This endpoint only gives All Time type leaderboard.
	 *		2. Please use 'AllTime_Point' to name the point column.
	 *
	 * @param Players Array of user to get rank for
	 * @param ReadObject This will contain the request and results when the operation completes.
	 */
	virtual bool ReadLeaderboards(
		TArray<FUniqueNetIdRef> const& Players,
		FOnlineLeaderboardReadRef& ReadObject) override;

	/**
	 * Query all user friends' ranks. This request only for Game Client.
	 * Note:
	 *		1. This endpoint only gives All Time type leaderboard.
	 *		2. Please use 'AllTime_Point' to name the point column.
	 *
	 * @param LocalUserNum Index of user that is attempting to query the rank.
	 * @param ReadObject This will contain the request and results when the operation completes.
	 */
	virtual bool ReadLeaderboardsForFriends(
		int32 LocalUserNum,
		FOnlineLeaderboardReadRef& ReadObject) override;

	/**
	 * @brief Query leaderboard ranks around a pivoted rank.
	 * 
	 * This endpoint will fetch a set amount of ranks based on the pivoted rank and the range.
	 * Note:
	 *		1. This endpoint only gives All Time type leaderboard.
	 *		
	 * @param Rank A rank you want to pivot at.
	 * @param Range The number of ranks to be retrieved below and above the pivoted rank.
	 * @param ReadObject This will contain the request and results when the operation completes.
	 */
	virtual bool ReadLeaderboardsAroundRank(
		int32 Rank,
		uint32 Range,
		FOnlineLeaderboardReadRef& ReadObject) override;

	/**
	 * @brief Query leaderboard ranks around a pivoted rank.
	 *
	 * This endpoint will fetch a set amount of ranks based on the pivoted rank and the range.
	 * Note:
	 *		1. This endpoint only gives Cycle type leaderboard.
	 * 
	 * @param Rank  A rank you want to pivot at.
	 * @param Range The number of ranks to be retrieved below and above the pivoted rank.
	 * @param CycleId The cycle of the leaderboard to retrieve the ranks at.
	 * @param ReadObject This will contain the request and results when the operation completes.
	 * @return 
	 */
	virtual bool ReadLeaderboardCycleAroundRank(
			int32 Rank,
			uint32 Range,
			FString const& CycleId,
			FOnlineLeaderboardReadRef& ReadObject);

	/**
	 * @brief Query leaderboard ranks in the range a specific user rank.
	 *
	 * This endpoint will fetch a set amount of ranks closed to a specific user rank.
	 * Note:
	 *		1. This endpoint only gives All Time type leaderboard.
	 * 
	 *
	 * @param Player The pivoted player
	 * @param Range The amount of rank to be retrieved. This means the OSS will fetch ranks below and above the user rank in the amount of the range.
	 * @param ReadObject This will contain the request and results when the operation completes.
	 */
	virtual bool ReadLeaderboardsAroundUser(
		FUniqueNetIdRef Player,
		uint32 Range,
		FOnlineLeaderboardReadRef& ReadObject) override;

	/**
	 * @brief Query leaderboard ranks in the range a specific user rank.
	 *
	 * This endpoint will fetch a set amount of ranks closed to a specific user rank.
	 * Note:
	 *		1. This endpoint only gives Cycle type leaderboard.
	 * 
	 *
	 * @param Player The pivoted player
	 * @param Range The amount of rank to be retrieved. This means the OSS will fetch ranks below and above the user rank in the amount of the range.
	 * @param CycleId The cycle of the leaderboard to retrieve the ranks at.
	 * @param ReadObject This will contain the request and results when the operation completes.
	 */
	virtual bool ReadLeaderboardCycleAroundUser(
		FUniqueNetIdRef Player,
		uint32 Range,
		FString const& CycleId,
		FOnlineLeaderboardReadRef& ReadObject);

	/**
	 * Is not supported.
	 */
	virtual void FreeStats(
		FOnlineLeaderboardRead& ReadObject) override;

	/**
	 * Is not supported.
	 */
	virtual bool WriteLeaderboards(
		FName const& SessionName,
		FUniqueNetId const& Player,
		FOnlineLeaderboardWrite& WriteObject) override;

	/**
	 * Is not supported.
	 */
	virtual bool FlushLeaderboards(
		FName const& SessionName) override;

	/**
	 * Is not supported.
	 */
	virtual bool WriteOnlinePlayerRatings(
		FName const& SessionName, int32 LeaderboardId,
		TArray<FOnlinePlayerScore> const& PlayerScores) override;
	//~ End IOnlineAchievement Interface

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineLeaderboardAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

private:
	/** Critical section for thread safe operation of the leaderboard metadata */
	mutable FCriticalSection LeaderboardMetadataLock;
};
