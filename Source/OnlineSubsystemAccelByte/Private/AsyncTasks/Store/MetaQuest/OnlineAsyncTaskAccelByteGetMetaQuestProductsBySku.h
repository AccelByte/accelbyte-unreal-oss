// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineStoreInterfaceV2AccelByte.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const TArray<FString>& InSkus, const FOnQueryOnlineStoreOffersComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku");
	}

private:
	TArray<FString> Skus{};

	FOnQueryOnlineStoreOffersComplete Delegate{};

	TMap<FUniqueOfferId, FOnlineStoreOfferRef> Products{};

	FOnlineError OnlineError;

	void OnGetProductsBySkuSuccess(const TArray<FPlatformProductPtr>& Response);
	void OnGetProductsBySkuError(const FPlatformHandlerError& Response);
};
