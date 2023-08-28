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

class FOnlineAsyncTaskAccelByteReadLeaderboardsAroundRank
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteReadLeaderboardsAroundRank, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteReadLeaderboardsAroundRank(FOnlineSubsystemAccelByte* const InABInterface,
		int InLocalUserNum,
		const FOnlineLeaderboardReadRef& InReadObject,
		int InRank,
		int Range,
		bool InUseCycle,
		const FString& InCycleId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void Tick() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReadLeaderboardsAroundRank");
	}

private:

	THandler<FAccelByteModelsLeaderboardRankingResultV3> OnReadLeaderboardRankSuccessHandler;
	void OnReadLeaderboardRankSuccess(FAccelByteModelsLeaderboardRankingResultV3 const& Result);

	FErrorHandler OnReadLeaderboardRankErrorHandler;
	void OnReadLeaderboardRankFailed(int Code, FString const& Message);
	
	FOnlineLeaderboardReadRef LeaderboardReadRef;
	FString ErrorMessage;
	FString ErrorCode;
	int32 Offset;
	int32 Limit;
	bool bUseCycle;
	FString CycleId;
};