// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Async task to refresh local active session data with data from the backend
 */
class FOnlineAsyncTaskAccelByteRefreshActiveSessions
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRefreshActiveSessions, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteRefreshActiveSessions(FOnlineSubsystemAccelByte* const InABInterface, const TArray<FName>& InSessionNames, const FOnRefreshActiveSessionsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRefreshActiveSessions");
	}

private:
	/**
	 * Sessions to refreshed
	 */
	TArray<FName> SessionNames;

	/**
	 * All removed session after refresh all active sessions
	 */
	TArray<FName> RemovedSessionNames;
	
	/**
	* Delegate fired when we finish the task to refresh the session
	*/
	FOnRefreshActiveSessionsComplete Delegate{};
	
	/**
	 * Total session refreshed counter
	 */
	FThreadSafeCounter TotalSessionRefreshed{};

	/**
	 * Handle on refresh session is completed
	 * @param bWasSuccessful Whether refresh session was successful or not
	 */
	void HandleOnRefreshSessionComplete(bool bWasSuccessful);
};

