// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineStoreInterfaceV2AccelByte.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FPurchaseCheckoutRequest& InPurchaseCheckoutRequest, const FOnPurchaseCheckoutComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct");
	}

private:
	FPurchaseCheckoutRequest CheckoutRequest{};

	FOnPurchaseCheckoutComplete Delegate{};

	FPurchaseReceipt Receipt;

	FOnlineError OnlineError;

	void OnCheckoutProductSuccess(const FPlatformPurchasePtr& Response);
	void OnCheckoutProductError(const FPlatformHandlerError& Response);
};
