// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteValidateUserInput
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteValidateUserInput, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteValidateUserInput(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FUserInputValidationRequest& InUserInputValidationRequest);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteValidateUserInput");
	}

private:
	// Endpoint Handlers
	void HandleSuccess(const FUserInputValidationResponse& InUserInputValidationResponse);
	void HandleError(int32 Code, const FString& Message);

	// Error Information
	FOnlineError OnlineError;

	// Input Parameter
	FUserInputValidationRequest UserInputValidationRequest;

	int32 LocalUserNum = 0;

	// Output Parameter
	FUserInputValidationResponse UserInputValidationResponse;
};
