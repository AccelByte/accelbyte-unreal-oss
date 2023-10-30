// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteEcommerceModels.h"

class FOnlineAsyncTaskAccelByteGetItemByCriteria
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetItemByCriteria, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetItemByCriteria(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, FAccelByteModelsItemCriteria const& ItemCriteria, int32 const& Offset, int32 const& Limit,
		TArray<EAccelByteItemListSortBy> SortBy);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetItemByCriteria");
	}

private:
	// Endpoint Handlers
	void HandleSuccess(const FAccelByteModelsItemPagingSlicedResult& Result);
	void HandleError(int32 ErrorCode, const FString& ErrorMessage);

	// Error Information
	int32 ErrorCode;
	FString ErrorMessage;

	// Output Variable 
	FAccelByteModelsItemPagingSlicedResult ItemPagingSliced;
	
	// Input Variables 
	FAccelByteModelsItemCriteria ItemCriteria;
	int32 Offset;
	int32 Limit;
	TArray<EAccelByteItemListSortBy> SortBy; 
};
