// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Models/AccelByteEcommerceModels.h"
#include "Interfaces/OnlinePurchaseInterface.h"

/**
 * Async task for syncing platform purchases. Specifically for GooglePlay
 */
class FOnlineAsyncTaskAccelByteSyncGooglePlay
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncGooglePlay, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteSyncGooglePlay(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, FAccelByteModelsPlatformSyncMobileGoogle InRequest, const FOnRequestCompleted& InDelegate);

	FOnlineAsyncTaskAccelByteSyncGooglePlay(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const TSharedRef<FPurchaseReceipt>& Receipt, const FOnRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncGooglePlay");
	}

private:
	/**
	 * Request body required for sync.
	 */
	FAccelByteModelsPlatformSyncMobileGoogle Request;

	/**
	 * Delegate fired when the sync SyncPlatformPurchase has completed.
	 */
	FOnRequestCompleted Delegate;

	/**
	 * Complete the AsyncTask on Initialize if failed to initalize.
	 */
	bool bIsFailedToInitialize = false;

	/**
	 * Delegate handler for when the request to GetItemBySku succeeds.
	 */
	void OnGetItemBySkuSuccess(const FAccelByteModelsItemInfo& Response);

	/**
	 * Delegate handler for when the request to SyncPlatformPurchase succeeds.
	 */
	void OnSyncPlatformPurchaseSuccess(const FAccelByteModelsPlatformSyncMobileGoogleResponse& Response);

	/**
	 * Delegate handler for when the request fails.
	 */
	void OnRequestError(int32 ErrorCode, const FString& ErrorMessage);

	bool ParsePurchaseReceiptToPlatformSyncMobileGoogle(const TSharedRef<FPurchaseReceipt>& Receipt);

	FString Error;
};
