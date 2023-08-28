// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Task to query the backend for the session ID of the dedicated session that should be registered with this server
 */
class FOnlineAsyncTaskAccelByteGetDedicatedV1SessionId
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetDedicatedV1SessionId, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetDedicatedV1SessionId(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName);

	virtual void Initialize() override;
	virtual void Finalize() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetDedicatedSessionId");
	}

private:

	/**
	 * Name of the session that we wish to query the backend for the ID of
	 */
	FName SessionName;

	/**
	 * String value representing the ID retrieved from the backend
	 */
	FString SessionId;

	/** Delegate handler for when we successfully get a session ID from the backend */
	void OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Result);

	/** Delegate handler for when we fail to get a session ID from the backend */
	void OnGetSessionIdError(int32 ErrorCode, const FString& ErrorMessage);

};

