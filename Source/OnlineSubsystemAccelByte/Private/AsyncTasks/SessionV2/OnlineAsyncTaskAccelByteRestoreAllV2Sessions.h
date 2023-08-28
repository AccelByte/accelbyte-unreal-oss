// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Async task to restore both party & game sessions if the user exits a game while still in a party or game session.
 */
class FOnlineAsyncTaskAccelByteRestoreAllV2Sessions
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRestoreAllV2Sessions, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteRestoreAllV2Sessions(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnRestoreActiveSessionsComplete& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRestoreAllV2Sessions");
	}

private:
	/** Flag denoting when we get a response back for querying information about the user's party */
	FThreadSafeBool bHasRetrievedPartySessionInfo = false;

	/** Flag denoting when we get a response back for querying information about the user's game sessions */
	FThreadSafeBool bHasRetrievedGameSessionInfo = false;

	/** Array of restored sessions that will be moved over to the session interface after execution */
	TArray<FOnlineRestoredSessionAccelByte> RestoredSessions;

	/** Delegate that is fired when the task to restore a party completes */
	FOnRestoreActiveSessionsComplete CompletionDelegate;

	/** Check whether we have completed all work for our task */
	bool HasFinishedAsyncWork();

	void OnGetMyGameSessionsSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result);
	void OnGetMyGameSessionsError(int32 ErrorCode, const FString& ErrorMessage);

	void OnGetMyPartiesSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result);
	void OnGetMyPartiesError(int32 ErrorCode, const FString& ErrorMessage);

	/** Timeout handler */
	virtual void OnTaskTimedOut() override;
};
