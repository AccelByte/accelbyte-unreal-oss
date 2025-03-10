﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineStoreInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteQueryOfferByFilter
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryOfferByFilter, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryOfferByFilter(FOnlineSubsystemAccelByte* const InABSubsystem,
		const FUniqueNetId& InUserId,
		const FOnlineStoreFilter& InFilter,
		const FOnQueryOnlineStoreOffersComplete& InDelegate,
		bool AutoCalcEstimatedPrice);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryOfferByFilter");
	}

private:
	void HandleGetItemByCriteria(const FAccelByteModelsItemPagingSlicedResult& Result);
	void HandleSearchItem(const FAccelByteModelsItemPagingSlicedResult& Result);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	void FilterAndAddResults(const FAccelByteModelsItemPagingSlicedResult& Result);
	void GetNextOffset(FString const& NextUrl, int32& OutOffset, int32& OutLimit);

	FOnlineStoreFilter Filter;
	FOnQueryOnlineStoreOffersComplete Delegate;
	FString Language;
	bool AutoCalcEstimatedPrice;

	FString ErrorMsg;
	FAccelByteModelsItemCriteria SearchCriteriaRequest;
	bool bIsSearchByCriteria {false};
	TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef> OfferMap;
};
