// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineLeaderboardInterfaceAccelByte.h"

#include "Online.h"
#include "OnlineStats.h"
#include "OnlineSubsystemAccelByteTypes.h"

class FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser(FOnlineSubsystemAccelByte* const InABInterface,
		int32 InLocalUserNum,
		const FUniqueNetIdRef& UserId,
		const FOnlineLeaderboardReadRef& InReadObject,
		const int32 InRange,
		bool InUseCycle,
		const FString& InCycleId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void Tick() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser");
	}
private:
	int32 Range;

	THandler<FAccelByteModelsUserRankingDataV3> OnGetUserRankingSuccessHandler;
	void OnGetUserRankingSuccess(FAccelByteModelsUserRankingDataV3 const& Result);

	THandler<FAccelByteModelsLeaderboardRankingResultV3> OnGetRankingSuccessHandler;
	void OnGetRankingSuccess(FAccelByteModelsLeaderboardRankingResultV3 const& Result);

	FErrorHandler OnRequestFailedHandler;
	void OnRequestFailed(int32 Code, FString const& Message);

	bool GetUserCycleRank(FAccelByteModelsUserRankingDataV3 const& RanksData, int32& OutRank) const;

	FOnlineLeaderboardReadRef LeaderboardObject;
	FUniqueNetIdRef User;
	FString ErrorCode;
	FString ErrorMessage;
	int32 Offset;

	bool bUseCycle;
	FString CycleId;
};