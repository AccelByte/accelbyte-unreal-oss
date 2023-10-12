// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlinePurchaseInterface.h"

class FOnlineAsyncTaskAccelByteRedeemCode
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRedeemCode, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteRedeemCode(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FRedeemCodeRequest& InRedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRedeemCode");
	}

private:
	void HandleRedeemCodeComplete(const FAccelByteModelsFulfillmentResult& Result);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	FRedeemCodeRequest RedeemCodeRequest;
	FOnPurchaseRedeemCodeComplete Delegate;
	FString Language;

	FOnlineError Error;
	FPurchaseReceipt Receipt;
	FAccelByteModelsFulfillmentResult FulfillmentResult;
};
