// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteStatisticModels.h"
 

class FOnlineAsyncTaskAccelByteQueryStatsUsers : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteQueryStatsUsers, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetId> InLocalUserId,
		const TArray<FUniqueNetIdRef>& InStatsUsers, const TArray<FString>& StatNames, const FOnlineStatsQueryUsersStatsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryStatsUsers");
	}

private:
	void HandleBulkFetchStatItemsValue(const TArray<FAccelByteModelsStatItemValueResponse>& Result);
	void HandleGetUserStatItems(const FAccelByteModelsUserStatItemPagingSlicedResult& Result);

	THandler<TArray<FAccelByteModelsStatItemValueResponse>> OnBulkFetchStatItemsValueSuccess;
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);
	FErrorHandler OnError;	
	FOnlineError OnlineError;
	TArray<TSharedRef<const FOnlineStatsUserStats>> OnlineUsersStatsPairs;
	THandler<FAccelByteModelsUserStatItemPagingSlicedResult> OnGetUserStatItemsSuccess;
	
	FUniqueNetIdRef LocalUserId;
	TArray<FUniqueNetIdRef> StatsUsers;
	TArray<FString> StatNames;
	FOnlineStatsQueryUsersStatsComplete Delegate;

	TArray<FString> AccelByteUserIds;
	TMap<FString, FVariantData> StatCodes;
	TArray<FString> StatCodesString{};
	int32 CountStatCodesString; 
	int32 CountUsers; 

};
