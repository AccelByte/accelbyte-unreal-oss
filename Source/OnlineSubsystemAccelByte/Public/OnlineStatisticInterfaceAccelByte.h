// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteStatisticModels.h"
#include "Interfaces/OnlineStatsInterface.h"

DECLARE_MULTICAST_DELEGATE_FourParams(FOnListUserStatItemsCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsFetchUser>&, const FString& /*Error*/);
typedef FOnListUserStatItemsCompleted::FDelegate FOnListUserStatItemsCompletedDelegate;

/**
 * Implementation of Statistic service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineStatisticAccelByte : public IOnlineStats, public TSharedFromThis<FOnlineStatisticAccelByte, ESPMode::ThreadSafe>
{ 
PACKAGE_SCOPE:
	/* Map of Users */
	TUniqueNetIdMap<TArray<TSharedRef<FAccelByteModelsFetchUser>>> UsersMap;
	
	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineStatisticAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
		: AccelByteSubsystem(InSubsystem)
	{}

public:
	virtual ~FOnlineStatisticAccelByte() override = default;

	bool ListUserStatItems(int32 LocalUserNum, const TArray<FString>& StatCodes, const TArray<FString>& Tags, const FString& AdditionalKey, bool bAlwaysRequestToService);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnListUserStatItemsCompleted, bool, const TArray<FAccelByteModelsFetchUser>&, const FString&);

	/* Get the list users stat items */
	// TODO : make this method as implementation from GetStats
	bool GetListUserStatItems(int32 LocalUserNum, TArray<TSharedRef<FAccelByteModelsFetchUser>>& OutUsers);

	void QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const TArray<FString>& StatNames, const FOnlineStatsQueryUserStatsComplete& Delegate);

	//~ Begin IOnlineStats Interface
	virtual void QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const FOnlineStatsQueryUserStatsComplete& Delegate) override;
	virtual void QueryStats(const FUniqueNetIdRef LocalUserId, const TArray<FUniqueNetIdRef>& StatUsers, const TArray<FString>& StatNames, const FOnlineStatsQueryUsersStatsComplete& Delegate) override;
	virtual TSharedPtr<const FOnlineStatsUserStats> GetStats(const FUniqueNetIdRef StatsUserId) const override;
	virtual void UpdateStats(const FUniqueNetIdRef LocalUserId, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats, const FOnlineStatsUpdateStatsComplete& Delegate) override;
#if !UE_BUILD_SHIPPING
	virtual void ResetStats(const FUniqueNetIdRef StatsUserId) override;
#endif // !UE_BUILD_SHIPPING
	//~ End  IOnlineStats Interface

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineStatisticAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private : 
	TSharedPtr<const FOnlineStatsUserStats> UserStats;
};