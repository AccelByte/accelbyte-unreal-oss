// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceAccelByte.h"

/**
 * Task to enqueue a dedicated session as joinable on the backend, allows for backfilling through matchmaking
 */
class FOnlineAsyncTaskAccelByteEnqueueJoinableSession : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteEnqueueJoinableSession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnEnqueueJoinableSessionComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteEnqueueJoinableSession");
	}

private:

	/**
	 * Name of the local session that we wish to enqueue as joinable
	 */
	FName SessionName;

	/**
	 * Delegate fired when our request to enqueue a joinable session completes
	 */
	FOnEnqueueJoinableSessionComplete Delegate;

	/**
	 * Delegate handler for when enqueuing a joinable session completes successfully
	 */
	void OnEnqueueJoinableSessionSuccess();

	/**
	 * Delegate handler for when enqueuing a joinable session completes with an error
	 */
	void OnEnqueueJoinableSessionError(int32 ErrorCode, const FString& ErrorMessage);

};

