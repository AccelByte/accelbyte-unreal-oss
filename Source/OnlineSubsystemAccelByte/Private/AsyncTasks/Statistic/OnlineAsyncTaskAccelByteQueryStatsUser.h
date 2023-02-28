// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteStatisticModels.h"
 

class FOnlineAsyncTaskAccelByteQueryStatsUser : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteQueryStatsUser, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryStatsUser(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum, const FUniqueNetId& InLocalUserId,
		const TSharedRef<const FUniqueNetId> InStatsUser, const TArray<FString>& InStatCodes, const FOnlineStatsQueryUserStatsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryStatsUser");
	}

private:
	void OnGetUserStatItemsSuccess(const FAccelByteModelsUserStatItemPagingSlicedResult& Result);

	void OnGetUserStatsItemsError(int32 Code, FString const& ErrMsg);
	FErrorHandler OnError;
	TSharedPtr<const FOnlineStatsUserStats> OnlineUserStatsPair;

	int32 LocalUserNum;
	FUniqueNetIdAccelByteUserRef StatsUser;
	TMap<FString, FVariantData> Stats;
	TArray<FString> StatNames{};
	FOnlineStatsQueryUserStatsComplete Delegate;
	FString ErrorCode;
	FString ErrorMessage;

};
