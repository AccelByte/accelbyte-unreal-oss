// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteDisableMfaAuthenticator
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteDisableMfaAuthenticator, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteDisableMfaAuthenticator(FOnlineSubsystemAccelByte* const InABSubsystem
		, const FUniqueNetId& InLocalUserId
		, const FString& InCode);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	
protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDisableMfaAuthenticator");
	}

private:
	/** Backup code for disabling Mfa. */
	FString Code;

	/** Store mfa token for disabling mfa backup codes if any mfa method enabled. */
	FString MfaToken;
	
	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on get mfa token completed. */
	void GetMfaToken();
	void OnGetMfaTokenSuccess(const FVerifyMfaResponse& Response);
	void OnGetMfaTokenError(const int32 ErrorCode, const FString& ErrorMessage);

	/** Handlers on disable Mfa backup codes completed. */
	void DisableMfaAuthenticator();
	void OnDisableMfaAuthenticatorSuccess();
	void OnDisableMfaAuthenticatorError(const int32 ErrorCode, const FString& ErrorMessage);
};
