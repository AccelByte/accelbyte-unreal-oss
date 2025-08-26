// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Core/Platform/AccelBytePlatformHandleModels.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePurchaseInterfaceAccelByte.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FOnQueryReceiptsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts");
	}

private:
	FOnQueryReceiptsComplete Delegate{};

	TArray<FPurchaseReceipt> Purchases{};

	FOnlineError OnlineError;

	void OnGetPurchasesSuccess(const TArray<FPlatformPurchasePtr>& Response);
	void OnGetPurchasesError(const FPlatformHandlerError& Response);
};
