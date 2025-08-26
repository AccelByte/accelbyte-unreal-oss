// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& PublisherClientID, const FGenerateCodeForPublisherTokenComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken");
	}

private:

	// Endpoint Handlers
	void HandleSuccess(const FCodeForTokenExchangeResponse& Result);
	void HandleError(int32 ErrorCode, const FString& ErrorMessage);
	
	// Initializer List Constructor
	FString PublisherClientID{};
	FGenerateCodeForPublisherTokenComplete Delegate{};

	FOnlineError OnlineError{};
	FCodeForTokenExchangeResponse Result{};
};
