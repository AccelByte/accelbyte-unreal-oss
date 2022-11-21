#pragma once
// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "Models/AccelByteStatisticModels.h"
 

class FOnlineAsyncTaskAccelByteQueryStatsUser : public FOnlineAsyncTaskAccelByte
{
public:
	FOnlineAsyncTaskAccelByteQueryStatsUser(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetId> InLocalUserId,
		const TSharedRef<const FUniqueNetId> InStatsUser, const TArray<FString>& InStatCodes, const FOnlineStatsQueryUserStatsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryStatsUser");
	}

private:
	void HandleBulkFetchStatItemsValue(const TArray<FAccelByteModelsStatItemValueResponse>& Result);
	void HandleGetUserStatItems(const FAccelByteModelsUserStatItemPagingSlicedResult& Result);

	THandler<TArray<FAccelByteModelsStatItemValueResponse>> OnBulkFetchStatItemsValueSuccess;
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);
	FOnlineStatsQueryUserStatsComplete Delegate;	
	FErrorHandler OnError;	
	FOnlineError OnlineError;
	TSharedPtr<const FOnlineStatsUserStats> OnlineUserStatsPair;

	FUniqueNetIdRef LocalUserId;
	FUniqueNetIdRef StatsUser;
	FString AccelByteUserId;
	TMap<FString, FVariantData> Stats;
	TArray<FString> StatCodes{};
	TArray<FString> StatNames{};
	int32 Count; 

};
