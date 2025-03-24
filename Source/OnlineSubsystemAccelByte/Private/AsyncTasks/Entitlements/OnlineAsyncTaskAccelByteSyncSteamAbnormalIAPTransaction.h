// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InLocalUserId
		, const FOnSyncSteamAbnormalIAPTransactionComplete& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction");
	}

private:
	FOnSyncSteamAbnormalIAPTransactionComplete CompletionDelegate{};

	FOnlineError OnlineError{};

	void OnSyncSteamAbnormalIAPTransactionSuccess();
	void OnSyncSteamAbnormalIAPTransactionError(int32 ErrorCode, const FString& ErrorMessage);

};
