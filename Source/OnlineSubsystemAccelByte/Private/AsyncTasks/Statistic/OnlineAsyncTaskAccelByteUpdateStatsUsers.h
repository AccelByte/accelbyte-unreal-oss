// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteStatisticModels.h"
#include "OnlineStatisticInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteUpdateStatsUsers
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteUpdateStatsUsers, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUpdateStatsUsers(FOnlineSubsystemAccelByte *const InABInterface
		, int32 InLocalUserNum
		, TArray<FOnlineStatsUserUpdatedStats> const& InBulkUpdateMultipleUserStatItems
		, FOnUpdateMultipleUserStatItemsComplete const& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateStatsUsers");
	}

private:

	THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>> OnBulkResetMultipleUserStatItemsValueSuccess;
	void OnBulkUpdateUserStatsSuccess(TArray<FAccelByteModelsUpdateUserStatItemsResponse> const& Result);

	FErrorHandler OnError;
	void OnBulkUpdateUserStatsFailed(int32 Code
		, FString const& ErrMsg);
	TArray<TSharedRef<const FOnlineStatsUserStats>> OnlineUsersStatsPairs;

	EAccelByteStatisticUpdateStrategy ConvertUpdateStrategy(FOnlineStatUpdate::EOnlineStatModificationType Strategy);

	TArray<FUniqueNetIdRef> StatsUsers;
	FUniqueNetIdAccelByteUserPtr AccelByteUserId;
	TMap<FString, FVariantData> Stats;
	FUniqueNetIdAccelByteUserPtr StatsUserId;
	TArray<FOnlineStatsUserUpdatedStats> BulkUpdateMultipleUserStatItems{};
	TArray<FAccelByteModelsUpdateUserStatItem> BulkUpdateUserStatItemsRequest{};
	TArray<FAccelByteModelsUpdateUserStatItemsResponse> BulkUpdateUserStatItemsResult;
	FOnUpdateMultipleUserStatItemsComplete Delegate;
	FString ErrorCode;
	FString ErrorMessage;

};
