// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

/**
 * Task to dequeue joinable session from the backend, disabling backfill from matchmaker.
 */
class FOnlineAsyncTaskAccelByteDequeueJoinableV1Session : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteDequeueJoinableV1Session, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteDequeueJoinableV1Session(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnDequeueJoinableSessionComplete& InDelegate);

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
