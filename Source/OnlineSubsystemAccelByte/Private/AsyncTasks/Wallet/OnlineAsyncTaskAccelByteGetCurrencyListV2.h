// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineIdentityInterfaceAccelByte.h"

/**
 * Task for get currency list
 */
class FOnlineAsyncTaskAccelByteGetCurrencyListV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetCurrencyListV2, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetCurrencyListV2(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, bool bInAlwaysRequestToService);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetCurrencyListV2");
	}

private:

	/**
	 * Delegate handler for when get currency list succeeds
	 */
	void OnGetCurrencyListSuccess(const TArray<FAccelByteModelsCurrencyList>& Result);
	THandler<TArray<FAccelByteModelsCurrencyList>> OnGetCurrencyListSuccessDelegate;

	/**
	 * Delegate handler for when get currency list fails
	 */
	void OnGetCurrencyListError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetCurrencyListErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	TArray<FAccelByteModelsCurrencyList> CachedCurrencyList;
	bool bAlwaysRequestToService;
};
