// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "Models/AccelByteStatisticModels.h"


class FOnlineAsyncTaskAccelByteResetUserStats : public FOnlineAsyncTaskAccelByte
{
public:
	FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetIdRef StatsUserId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteResetUserStats");
	}

private:

	void HandleBulkUpdateUserStatItemsValue(const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result);

	THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>> OnBulkResetMultipleUserStatItemsValueSuccess;
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);
	FErrorHandler OnError;
	FOnlineError OnlineError;

	FUniqueNetIdRef StatsUserId;
	TArray<FOnlineStatsUserUpdatedStats> UpdatedUserStats;
	TArray<FAccelByteModelsUpdateUserStatItemWithStatCode> BulkUpdateUserStatItems; 
	FOnlineStatsUpdateStatsComplete Delegate; 

};
