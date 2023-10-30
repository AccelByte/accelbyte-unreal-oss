// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteEcommerceModels.h"

class FOnlineAsyncTaskAccelByteCreateNewOrder
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCreateNewOrder, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCreateNewOrder(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FAccelByteModelsOrderCreate& InOrderCreate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateNewOrder");
	}

private: 
	// Endpoint Handlers
	void HandleSuccess(const FAccelByteModelsOrderInfo& Result);
	void HandleError(int32 ErrorCode, const FString& ErrorMessage);

	// Error Information
	int32 ErrorCode;
	FString ErrorMessage;

	// Output Variable 
	FAccelByteModelsOrderInfo OrderInfo;
	// Input Variable 
	FAccelByteModelsOrderCreate OrderCreate;
	
};
