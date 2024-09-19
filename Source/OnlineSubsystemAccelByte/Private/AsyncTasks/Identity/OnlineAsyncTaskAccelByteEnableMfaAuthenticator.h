// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteEnableMfaAuthenticator
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteEnableMfaAuthenticator, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteEnableMfaAuthenticator(FOnlineSubsystemAccelByte* const InABSubsystem
		, const FUniqueNetId& InLocalUserId
		, const FString& InCode);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	
protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteEnableMfaAuthenticator");
	}

private:
	/** Code for enabling Mfa authenticator. */
	FString Code;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on Enable Mfa Email completed. */
	void OnEnableMfaAuthenticatorSuccess();
	void OnEnableMfaAuthenticatorError(const int32 ErrorCode, const FString& ErrorMessage);
};
