// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	
protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes");
	}

private:
	/** Generated backup codes. */
	FUser2FaBackupCode BackupCodes;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on BE API calls completed. */
	void OnSuccess(const FUser2FaBackupCode& Response);
	void OnError(const int32 ErrorCode, const FString& ErrorMessage);
};
