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
		const TArray<FUniqueNetIdRef>& InStatsUsers, const TArray<FString>& InStatNames, const FOnlineStatsQueryUsersStatsComplete& InDelegate);

	FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum,
		const TArray<FUniqueNetIdRef>& InStatsUsers, const TArray<FString>& InStatNames, const FOnlineStatsQueryUsersStatsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void Tick() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryStatsUsers");
	}

private:
	void OnGetUserStatItemsSuccess(const FAccelByteModelsUserStatItemPagingSlicedResult& Result);

	void OnGetUsersStatsItemsError(int32 Code, FString const& ErrMsg);
	FErrorHandler OnError;
	TArray<TSharedRef<const FOnlineStatsUserStats>> OnlineUsersStatsPairs;
	THandler<FAccelByteModelsUserStatItemPagingSlicedResult> OnGetUserStatItemsSuccessHandler;
	
	int32 LocalUserNum;
	TArray<FUniqueNetIdRef> StatsUsers;
	TArray<FString> StatNames;
	FOnlineStatsQueryUsersStatsComplete Delegate;

	TArray<FString> AccelByteUserIds;
	int32 CountUsers;
	FString ErrorCode;
	FString ErrorMessage;

};
