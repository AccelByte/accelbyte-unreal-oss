// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteStatisticModels.h"
#include "OnlineStatisticInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteCreateStatsUser : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteCreateStatsUser, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCreateStatsUser(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum,
		const TSharedRef<const FUniqueNetId> InStatsUser, const TArray<FString>& InStatCodes, const FOnlineStatsCreateStatsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateStatsUser");
	}

private:
	THandler<TArray<FAccelByteModelsBulkStatItemOperationResult>> OnBulkCreateStatItemsSuccess;
	void HandleBulkCreateStatItems(const TArray<FAccelByteModelsBulkStatItemOperationResult>& Result);

	FErrorHandler OnError;
	void HandleAsyncTaskError(int32 Code, const FString& ErrMsg);

	int32 LocalUserNum;
	FString AccelByteUserId;
	FUniqueNetIdRef StatsUser;
	TArray<FString> StatCodes{};
	FOnlineStatsCreateStatsComplete Delegate;
	TArray<FAccelByteModelsBulkStatItemOperationResult> CreateStatsResult;
	FString ErrorCode;
	FString ErrorMessage;

};
