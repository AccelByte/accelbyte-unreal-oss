// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Models/AccelByteEcommerceModels.h"

/**
 * Async task for syncing platform purchases. Currently supports Steam, Xbox and Playstation.
 */
class FOnlineAsyncTaskAccelByteSyncPlatformPurchase : public FOnlineAsyncTaskAccelByte 
{
public:

	FOnlineAsyncTaskAccelByteSyncPlatformPurchase(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, FAccelByteModelsEntitlementSyncBase EntitlementSyncBase, const FOnRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncPlatformPurchase");
	}

private:

	FAccelByteModelsEntitlementSyncBase EntitlementSyncBase;

	/**
	 * Delegate fired when the sync SyncPlatformPurchase has completed.
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

	EAccelBytePlatformSync GetNavitePlatformSyncType();

	FString Error;
};
