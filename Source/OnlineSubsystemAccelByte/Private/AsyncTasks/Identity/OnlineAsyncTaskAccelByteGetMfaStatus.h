// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGetMfaStatus
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetMfaStatus, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetMfaStatus(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	
protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetMfaStatus");
	}

private:
	/** MFA status for current user. */
	FMfaStatusResponse MfaStatus;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on get mfa completed. */
	void OnSuccess(const FMfaStatusResponse& Response);
	void OnError(const int32 ErrorCode, const FString& ErrorMessage);
};
