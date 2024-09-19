// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteSendMfaCodeToEmail
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSendMfaCodeToEmail, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSendMfaCodeToEmail(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId, const EAccelByteSendMfaEmailAction InAction);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	
protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendMfaCodeToEmail");
	}

private:
	/** Action code to send to user email. */
	EAccelByteSendMfaEmailAction Action;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on BE API calls completed. */
	void OnSuccess();
	void OnError(const int32 ErrorCode, const FString& ErrorMessage);
};
