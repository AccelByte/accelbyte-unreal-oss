// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineStoreInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteQueryOfferDynamicData
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryOfferDynamicData, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryOfferDynamicData(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FUniqueOfferId& InOfferId,
		const FOnQueryOnlineStoreOffersComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryOfferDynamicData");
	}

private:
	void HandleGetItemDynamicData(const FAccelByteModelsItemDynamicData& Result);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	FString ErrorMsg;
	FUniqueOfferId OfferId;
	THandler<FAccelByteModelsItemDynamicData> OnSuccess;
	FErrorHandler OnError;
	FOnQueryOnlineStoreOffersComplete Delegate;
	FAccelByteModelsItemDynamicData DynamicData;
};
