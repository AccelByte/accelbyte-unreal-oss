// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelBytePreviewOrder
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelBytePreviewOrder, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelBytePreviewOrder(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FAccelByteModelsUserPreviewOrderRequest& InPreviewOrderRequest);
	
	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelBytePreviewOrder");
	}

private: 
	// Endpoint Handlers
	void HandleSuccess(const FAccelByteModelsUserPreviewOrderResponse& Result);
	void HandleError(int32 Code, const FString& Message);

	// Error Information
	int32 ErrorCode;
	FString ErrorMessage;

	// Output Variable 
	FAccelByteModelsUserPreviewOrderResponse PreviewOrderResponse;
	// Input Variable 
	FAccelByteModelsUserPreviewOrderRequest PreviewOrderRequest;
};
