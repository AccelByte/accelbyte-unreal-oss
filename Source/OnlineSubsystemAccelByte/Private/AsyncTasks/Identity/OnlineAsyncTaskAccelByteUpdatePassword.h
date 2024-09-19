// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteUpdatePassword
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdatePassword, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUpdatePassword(FOnlineSubsystemAccelByte* const InABSubsystem
		, const FUniqueNetId& InLocalUserId
		, const FUpdatePasswordRequest& InUpdatePasswordRequest
		, EAccelByteLoginAuthFactorType InFactorType
		, const FString& InCode);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	
protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdatePassword");
	}

private:
	/** Update password request. */
	FUpdatePasswordRequest UpdatePasswordRequest;
	
	/** Factor type and code for disabling Mfa backup codes. */
	EAccelByteLoginAuthFactorType FactorType;
	FString Code;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on get mfa token completed. */
	void GetMfaToken();
	void OnGetMfaTokenSuccess(const FVerifyMfaResponse& Response);
	void OnGetMfaTokenError(const int32 ErrorCode, const FString& ErrorMessage);

	/** Handlers on disable Mfa backup codes completed. */
	void UpdatePassword();
	void OnUpdatePasswordSuccess();
	void OnUpdatePasswordError(const int32 ErrorCode, const FString& ErrorMessage);
};
