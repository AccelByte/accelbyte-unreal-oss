// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

/**
 * Async task to Synchronize DLC using the DLC sync API call respective to the user's platform.
 */
class FOnlineAsyncTaskAccelByteSyncDLC : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteSyncDLC(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FOnRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncDLC");
	}

private:

	FString Error;


	/** Delegate to be fired after get RedeemDLCCode completes */
	FOnRequestCompleted Delegate;

	/**
	 * Delegate handler for when DLC sync is successful
	 */
	void OnSyncDLCSuccess();

	/**
	 * Delegate handler for when DLC sync fails
	 */
	void OnSyncDLCFailed(int32 ErrorCode, const FString& ErrorMessage);

};

