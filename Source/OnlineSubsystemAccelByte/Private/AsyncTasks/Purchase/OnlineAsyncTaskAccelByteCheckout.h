// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlinePurchaseInterface.h"

class FOnlineAsyncTaskAccelByteCheckout
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCheckout, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCheckout(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FPurchaseCheckoutRequest& InCheckoutRequest, const FOnPurchaseCheckoutComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCheckout");
	}

private:
	void HandleCheckoutComplete(const FAccelByteModelsOrderInfo& Result);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	FPurchaseCheckoutRequest CheckoutRequest;
	FOnPurchaseCheckoutComplete Delegate;
	FString ErrorCode;
	FString ErrorMessage;
	FString Language;

	FPurchaseReceipt Receipt;
	FAccelByteModelsPaymentFailedPayload PaymentEventPayload;
};
