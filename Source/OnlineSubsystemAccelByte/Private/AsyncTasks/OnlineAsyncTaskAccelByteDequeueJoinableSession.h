// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceAccelByte.h"

/**
 * Task to dequeue joinable session from the backend, disabling backfill from matchmaker.
 */
class FOnlineAsyncTaskAccelByteDequeueJoinableSession : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteDequeueJoinableSession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnDequeueJoinableSessionComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDequeueJoinableSession");
	}

private:
	
	/**
	 * Name of the local session that we wish to dequeue joinable for
	 */
	FName SessionName;

	/**
	 * Delegate fired when our request to dequeue a joinable session completes
	 */
	FOnDequeueJoinableSessionComplete Delegate;

	/**
	 * Delegate handler for when dequeuing a joinable session completes successfully
	 */
	void OnDequeueJoinableSessionSuccess();

	/**
	 * Delegate handler for when dequeuing a joinable session completes with an error
	 */
	void OnDequeueJoinableSessionError(int32 ErrorCode, const FString& ErrorMessage);

};
