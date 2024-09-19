// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGenerateMfaAuthenticatorSecretKey
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGenerateMfaAuthenticatorSecretKey, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGenerateMfaAuthenticatorSecretKey(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	
protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGenerateMfaAuthenticatorSecretKey");
	}

private:
	/** Generated backup codes. */
	FUser2FaSecretKey SecretKey;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on BE API calls completed. */
	void OnSuccess(const FUser2FaSecretKey& Response);
	void OnError(const int32 ErrorCode, const FString& ErrorMessage);
};
