// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGetInputValidations
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetInputValidations, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetInputValidations(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& InLanguageCode, bool bInDefaultOnEmpty);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetInputValidations");
	}

private:
	// Endpoint Handlers
	void HandleSuccess(const FInputValidation& InInputValidation);
	void HandleError(int32 Code, const FString& Message);

	// Error Information
	FOnlineError OnlineError;

	// Input Parameters
	FString LanguageCode;
	bool bDefaultOnEmpty;

	// Output Parameter
	FInputValidation InputValidation;
};
