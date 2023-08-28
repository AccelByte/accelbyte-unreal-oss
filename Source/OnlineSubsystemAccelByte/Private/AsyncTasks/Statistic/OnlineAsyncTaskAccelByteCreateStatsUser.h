// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteStatisticModels.h"
#include "OnlineStatisticInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteCreateStatsUser
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCreateStatsUser, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCreateStatsUser(FOnlineSubsystemAccelByte *const InABInterface
		, int32 InLocalUserNum
		, FUniqueNetIdRef const& InStatsUser
		, TArray<FString> const& InStatCodes
		, FOnlineStatsCreateStatsComplete const& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateStatsUser");
	}

private:
	THandler<TArray<FAccelByteModelsBulkStatItemOperationResult>> OnBulkCreateStatItemsSuccess;
	void HandleBulkCreateStatItems(TArray<FAccelByteModelsBulkStatItemOperationResult> const& Result);

	FErrorHandler OnError;
	void HandleAsyncTaskError(int32 Code
		, FString const& ErrMsg);

	FUniqueNetIdAccelByteUserPtr StatsUser{nullptr};
	TArray<FString> StatCodes{};
	FOnlineStatsCreateStatsComplete Delegate{};
	TArray<FAccelByteModelsBulkStatItemOperationResult> CreateStatsResult{};
	FString ErrorCode{};
	FString ErrorMessage{};

};
