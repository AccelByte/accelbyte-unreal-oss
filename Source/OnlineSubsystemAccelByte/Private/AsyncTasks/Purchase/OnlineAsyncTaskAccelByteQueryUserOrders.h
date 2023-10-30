// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteEcommerceModels.h"

class FOnlineAsyncTaskAccelByteQueryUserOrders
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryUserOrders, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryUserOrders(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& UserId, const FAccelByteModelsUserOrdersRequest& UserOrderRequest);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryUserOrders");
	}

private:
	// Endpoint Handlers
	void HandleSuccess(const FAccelByteModelsPagedOrderInfo& Result);
	void HandleError(int32 ErrorCode, const FString& ErrorMessage);

	// Error Information
	int32 ErrorCode;
	FString ErrorMessage;

	// Output Variable 
	FAccelByteModelsPagedOrderInfo PagedOrderInfo;
	// Input Variable 
	FAccelByteModelsUserOrdersRequest UserOrderRequest;
};
