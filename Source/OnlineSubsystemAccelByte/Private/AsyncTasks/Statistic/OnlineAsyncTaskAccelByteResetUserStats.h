// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Interfaces/OnlineStatsInterface.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteStatisticModels.h"

class FOnlineAsyncTaskAccelByteResetUserStats
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteResetUserStats, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte *const InABInterface
		, FUniqueNetIdRef InStatsUserId
		, TSharedPtr<const FOnlineStatsUserStats> InUserStats
		, FOnlineStatsUpdateStatsComplete const& InDelegate = nullptr);

	FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte *const InABInterface
		, int32 InLocalUserNum
		, FUniqueNetIdRef InStatsUserId
		, TSharedPtr<const FOnlineStatsUserStats> InUserStats
		, FOnlineStatsUpdateStatsComplete const& InDelegate = nullptr);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteResetUserStats");
	}

private:

	void OnResetUserStatItemsSuccess(TArray<FAccelByteModelsUpdateUserStatItemsResponse> const& Result);

	THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>> OnBulkResetMultipleUserStatItemsValueSuccess;
	void OnResetUserStatItemsFailed(int32 Code
		, FString const& ErrMsg);
	FErrorHandler OnError;
	TSharedPtr<const FOnlineStatsUserStats> OnlineUserStatsPair;

	TArray<FAccelByteModelsUpdateUserStatItemsResponse> UserStatsResetResponse;
	FUniqueNetIdAccelByteUserRef StatsUserId;
	TMap<FString, FVariantData> Stats;
	TSharedPtr<const FOnlineStatsUserStats> UserStats;
	FOnlineStatsUpdateStatsComplete Delegate; 
	FString ErrorCode;
	FString ErrorMessage;

};
