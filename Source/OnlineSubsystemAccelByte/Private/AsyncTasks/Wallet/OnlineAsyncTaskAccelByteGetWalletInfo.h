// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteEcommerceModels.h"
#include <OnlineIdentityInterfaceAccelByte.h>
#include <OnlineWalletInterfaceAccelByte.h>

/**
 * Task for get wallet info
 */
class FOnlineAsyncTaskAccelByteGetWalletInfo : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteGetWalletInfo, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetWalletInfo(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InCurrencyCode, bool bInAlwaysRequestToService);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetWalletInfo");
	}

private:

	/**
	 * Delegate handler for when get wallet info succeeds
	 */
	void OnGetWalletInfoSuccess(const FAccelByteModelsWalletInfo& Result);
	THandler<FAccelByteModelsWalletInfo> OnGetWalletInfoSuccessDelegate;

	/**
	 * Delegate handler for when get wallet info fails
	 */
	void OnGetWalletInfoError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetWalletInfoErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	FAccelByteModelsWalletInfo CachedWalletInfo;
	FString CurrencyCode;
	bool bAlwaysRequestToService;
};
