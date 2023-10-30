// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteEcommerceModels.h"

class FOnlineAsyncTaskAccelByteGetEstimatedPrice
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetEstimatedPrice, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetEstimatedPrice(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const TArray<FString>& ItemIds, const FString& Region);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetEstimatedPrice");
	}

private:
	// Endpoint Handlers
	void HandleSuccess(const TArray<FAccelByteModelsEstimatedPrices>& Result);
	void HandleError(int32 ErrorCode, const FString& ErrorMessage);

	// Error Information
	int32 ErrorCode;
	FString ErrorMessage;
	
	// Output Variable 
	TArray<FAccelByteModelsEstimatedPrices> EstimatedPrices;
	
	// Input Variables
	TArray<FString> ItemIds;
	FString Region; 
};
