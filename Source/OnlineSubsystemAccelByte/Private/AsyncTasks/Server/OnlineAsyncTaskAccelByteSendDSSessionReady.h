// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteSendDSSessionReady
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSendDSSessionReady, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSendDSSessionReady(FOnlineSubsystemAccelByte* const InABInterface, bool bInIsServerReady);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendDSReady");
	}

private:

	/** The server status to update to session service. */
	bool bIsServerReady;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Session interface for this user. */
	FOnlineSessionV2AccelBytePtr SessionInterface;

	/** Handler when send ds ready success. */
	FVoidHandler OnSendDSReadySuccessDelegate;
	void OnSendDSReadySuccess();

	/** Handler when send ds ready failed. */
	FErrorHandler OnSendDSReadyErrorDelegate;
	void OnSendDSReadyError(int32 ErrorCode, const FString& ErrorMessage);
};
