﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteQueryOfferById
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryOfferById, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryOfferById(FOnlineSubsystemAccelByte* const InABSubsystem,
		const FUniqueNetId& InUserId,
		const TArray<FUniqueOfferId>& InOfferIds,
		const FOnQueryOnlineStoreOffersComplete& InDelegate,
		const FString& StoreId,
		bool AutoCalcEstimatedPrice);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryOfferById");
	}

private:
	void HandleGetItemByIds(const TArray<FAccelByteModelsItemInfo>& Result);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	TArray<FUniqueOfferId> OfferIds;
	FString Language;
	FOnQueryOnlineStoreOffersComplete Delegate;
	FString StoreId;
	bool AutoCalcEstimatedPrice;

	FString ErrorMsg;
	THandler<TArray<FAccelByteModelsItemInfo>> OnSuccess;
	FErrorHandler OnError;
	TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef> OfferMap{};
};
