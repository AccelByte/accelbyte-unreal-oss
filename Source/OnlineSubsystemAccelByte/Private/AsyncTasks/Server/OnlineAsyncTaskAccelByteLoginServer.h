// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Async task to log in a dedicated server
 */
class FOnlineAsyncTaskAccelByteLoginServer
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLoginServer, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteLoginServer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLoginServer");
	}

private:
	/** String representing the error that occurred as a key */
	FString ErrorString{};

	/** Digit code representing the error that occurred*/
	int32 ErrorCode;

	FVoidHandler OnServerLoginSuccessDelegate;
	void OnLoginServerSuccess();

	FErrorHandler OnServerLoginErrorDelegate;
	void OnLoginServerError(int32 ErrorCode, const FString& ErrorMessage);

};

