// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "Models/AccelByteEcommerceModels.h"

class FOnlineAsyncTaskAccelByteQueryOfferBySku : public FOnlineAsyncTaskAccelByte
{
public:
	FOnlineAsyncTaskAccelByteQueryOfferBySku(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InSku,
		const FOnQueryOnlineStoreOffersComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryOfferBySku");
	}

private:
	void HandleGetItemBySku(const FAccelByteModelsItemInfo& Result);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	FString Sku;
	FOnQueryOnlineStoreOffersComplete Delegate;
	FOnlineStoreOfferRef Offer;
	FString Language;

	FString ErrorMsg;
	THandler<FAccelByteModelsItemInfo> OnSuccess;
	FErrorHandler OnError;
};
