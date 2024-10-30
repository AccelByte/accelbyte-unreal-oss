// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Models/AccelByteEcommerceModels.h"

/**
 * Async task for syncing iOS App Store single purchase
 */
class FOnlineAsyncTaskAccelByteSyncIOSAppStore
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncIOSAppStore, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteSyncIOSAppStore(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const TSharedRef<FPurchaseReceipt>& Receipt, const FOnRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncIOSAppStore");
	}

private:

    /**
     * Form the Sync request at the begining without a need to withhold the Receipt for too long.
     */
	FAccelByteModelsPlatformSyncMobileApple SyncRequest;
    
    /**
     * Complete the AsyncTask on Initialize if failed to initalize.
     */
    bool bIsFailedToInitializeDueToInvalidReceipt = true;
    
	/**
	 * Delegate fired when the SyncPlatformPurchase has completed.
	 */
	FOnRequestCompleted Delegate;

	/**
	 * Delegate handler for when the request to SyncPlatformPurchase succeeds.
	 */
	void OnSyncPlatformPurchaseSuccess();

	/**
	 * Delegate handler for when the request to SyncPlatformPurchase fails.
	 */
	void OnSyncPlatformPurchaseError(int32 ErrorCode, const FString& ErrorMessage);

	FString Error{};
};
