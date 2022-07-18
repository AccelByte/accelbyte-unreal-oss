// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteEcommerceModels.h"
#include <OnlineIdentityInterfaceAccelByte.h>
#include <OnlineWalletInterfaceAccelByte.h>

/**
 * Task for get wallet transaction list
 */
class FOnlineAsyncTaskAccelByteGetWalletTransactions : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteGetWalletTransactions, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetWalletTransactions(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InCurrencyCode, int32 InOffset, int32 InLimit);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetWalletTransactions");
	}

private:

	/**
	 * Delegate handler for when get wallet transaction list succeeds
	 */
	void OnGetWalletTransactionsSuccess(const FAccelByteModelsWalletTransactionPaging& Result);
	THandler<FAccelByteModelsWalletTransactionPaging> OnGetWalletTransactionsSuccessDelegate;

	/**
	 * Delegate handler for when get wallet transaction list fails
	 */
	void OnGetWalletTransactionsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetWalletTransactionsErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	TArray<FAccelByteModelsWalletTransactionInfo> CachedWalletTransactions;
	FString CurrencyCode;
	int32 Offset, Limit;
	bool bAlwaysRequestToService;
};
