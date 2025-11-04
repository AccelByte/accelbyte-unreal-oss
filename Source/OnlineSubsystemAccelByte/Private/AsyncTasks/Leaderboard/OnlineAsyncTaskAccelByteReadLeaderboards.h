// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineLeaderboardInterfaceAccelByte.h"

#include "Online.h"
#include "OnlineStats.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineStatsInterface.h"

class FOnlineAsyncTaskAccelByteReadLeaderboards
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteReadLeaderboards, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteReadLeaderboards(FOnlineSubsystemAccelByte* const InABInterface,
		int32 InLocalUserNum,
		const TArray<FUniqueNetIdRef>& InUsers, 
		FOnlineLeaderboardReadRef& InReadObject);

	FOnlineAsyncTaskAccelByteReadLeaderboards(FOnlineSubsystemAccelByte* const InABInterface,
		int32 InLocalUserNum,
		const TArray<FUniqueNetIdRef>& InUsers,
		FOnlineLeaderboardReadRef& InReadObject,
		bool bIsCycle,
		const FString& CycleId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void Tick() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReadLeaderboards");
	}

private:

	void QueryLeaderboard();
	THandler<FAccelByteModelsBulkUserRankingDataV3> OnReadLeaderboardsSuccessHandler;
	void OnReadLeaderboardsSuccess(FAccelByteModelsBulkUserRankingDataV3 const& Result);

	bool bQueryUserAccountMissingRequired = false;
	bool bQueryUserAccountMissingCompleted = false;
	void OnQueryMissingUsersCompleted(bool bWasSuccessful, TArray<FAccelByteUserInfoRef> Result);

	FErrorHandler OnReadLeaderboardsFailedHandler;
	void OnReadLeaderboardsFailed(int32 Code, FString const& ErrMsg);
	TArray<TSharedRef<const FOnlineStatsUserStats>> OnlineUsersStatsPairs;

	int32 FindCycle(TArray<FAccelByteModelsCycleRank> const& Cycles, FString const& CycleId);

	bool bUseCycle = false;
	FString CycleIdValue;
	int32 CountRequests = 0;
	const TArray<FUniqueNetIdRef> AccelByteUsers;
	FOnlineLeaderboardReadRef LeaderboardObject;
	TMap<FString, FUniqueNetIdPtr> CurrentProcessedUsers;
	TMap<FString, FVariantData> Stats;
	FString ErrorCode;
	FString ErrorMessage;

	// Prevent racing condition due to async response from query Leaderboard
	FCriticalSection OnReadLeaderboardSuccessMtx;
};
