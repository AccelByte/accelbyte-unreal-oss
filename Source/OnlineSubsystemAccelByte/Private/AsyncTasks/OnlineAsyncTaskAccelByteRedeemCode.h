// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineAsyncTaskAccelByte.h"
#include "Interfaces/OnlinePurchaseInterface.h"

class FOnlineAsyncTaskAccelByteRedeemCode : public FOnlineAsyncTaskAccelByte
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

	FOnlineError Error;
	FString Language;
	FRedeemCodeRequest RedeemCodeRequest;
	FOnPurchaseRedeemCodeComplete Delegate;
	FPurchaseReceipt Receipt;
};
