// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InLocalUserId
		, const TSharedRef<FPurchaseReceipt>& InPurchaseReceipt
		, const FOnRequestCompleted& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction");
	}

private:
	TSharedRef<FPurchaseReceipt> PurchaseReceipt{};

	FOnRequestCompleted CompletionDelegate{};

	FOnlineError OnlineError{};

	void OnSyncSteamIAPTransactionSuccess(const FAccelByteModelsSyncSteamIAPTransactionResponse& Response);
	void OnSyncSteamIAPTransactionError(int32 ErrorCode, const FString& ErrorMessage);

};
