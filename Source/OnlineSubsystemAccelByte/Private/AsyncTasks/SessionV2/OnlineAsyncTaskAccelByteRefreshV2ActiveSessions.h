// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
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
class FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions(FOnlineSubsystemAccelByte* const InABInterface, const int32 LocalUserNum, const TArray<FName>& InSessionNames, bool bTriggerOnReconnectedRefreshSessionDelegates);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions");
	}

private:
	
	/**
	 * LocalUserNum that requested to Refresh Active Session
	 */
	int32 LocalUserNum = 0;

	/**
	 * Sessions to refreshed
	 */
	TArray<FName> SessionNames{};
	
	/**
	 * If this is called, then it trigger delegates.
	 */
	bool bTriggerOnReconnectedRefreshSessionDelegates = true;

	/**
	 * All removed session after refresh all active sessions
	 */
	TArray<FName> RemovedSessionNames{};
	
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

