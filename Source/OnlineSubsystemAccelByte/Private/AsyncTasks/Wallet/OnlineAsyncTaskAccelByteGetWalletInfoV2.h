// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteEcommerceModels.h"
#include <OnlineIdentityInterfaceAccelByte.h>
#include <OnlineWalletV2InterfaceAccelByte.h>

/**
 * Task for get wallet V2 info
 */
class FOnlineAsyncTaskAccelByteGetWalletInfoV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetWalletInfoV2, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetWalletInfoV2(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InCurrencyCode, bool bInAlwaysRequestToService);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetWalletInfoV2");
	}

private:

	/**
	 * Delegate handler for when get wallet info succeeds
	 */
	void OnGetWalletInfoSuccess(const FAccelByteModelsWalletInfoResponse& Result);
	THandler<FAccelByteModelsWalletInfoResponse> OnGetWalletInfoSuccessDelegate;

	/**
	 * Delegate handler for when get wallet info fails
	 */
	void OnGetWalletInfoError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetWalletInfoErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	FAccelByteModelsWalletInfoResponse CachedWalletInfo;
	FString CurrencyCode;
	bool bAlwaysRequestToService;
};
