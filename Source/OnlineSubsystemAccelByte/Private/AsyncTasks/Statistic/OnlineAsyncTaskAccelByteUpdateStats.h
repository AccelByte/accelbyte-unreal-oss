// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteStatisticModels.h"


class FOnlineAsyncTaskAccelByteUpdateStats : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteUpdateStats, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUpdateStats(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetIdRef LocalUserId, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats,
		const FOnlineStatsUpdateStatsComplete& Delegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateStats");
	}

private:

	void HandleBulkUpdateUserStatItemsValue(const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result);

	THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>> OnBulkUpdateUserStatItemsValueSuccess;
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);
	FErrorHandler OnError;
	FOnlineError OnlineError;

	FUniqueNetIdRef LocalUserId;
	TArray<FOnlineStatsUserUpdatedStats> UpdatedUserStats;
	TArray<FAccelByteModelsUpdateUserStatItemWithStatCode> BulkUpdateUserStatItems; 
	FOnlineStatsUpdateStatsComplete Delegate; 

};
