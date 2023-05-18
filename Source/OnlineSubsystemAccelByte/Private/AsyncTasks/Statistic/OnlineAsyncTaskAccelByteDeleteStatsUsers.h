// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineStatisticInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteDeleteStatsUsers
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteDeleteStatsUsers, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteDeleteStatsUsers(FOnlineSubsystemAccelByte *const InABInterface
		, int32 InLocalUserNum
		, FUniqueNetIdRef const& InStatsUser
		, FString const& InStatCode
		, FString const& InAdditionalKey);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDeleteStatsUsers");
	}

private:

	FVoidHandler OnDeleteUserStatItemsValueSuccess;
	void OnDeleteUserStatsSuccess();

	FErrorHandler OnError;
	void OnDeleteUserStatsFailed(int32 Code
		, FString const& ErrMsg);
	TArray<TSharedRef<const FOnlineStatsUserStats>> OnlineUsersStatsPairs{};

	FUniqueNetIdAccelByteUserPtr StatsUser{nullptr};
	FString StatCode{};
	FString AdditionalKey{};
	TMap<FString, FVariantData> Stats{};
	FString ErrorCode{};
	FString ErrorMessage{};

};
