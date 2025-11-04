// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

/**
 * Task to send a request to backend that we wish to join the session associated with SessionName
 */
class FOnlineAsyncTaskAccelByteJoinV2GameSession
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteJoinV2GameSession, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteJoinV2GameSession(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InLocalUserId
		, const FName& InSessionName
		, bool bInHasLocalUserJoined);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJoinV2GameSession");
	}

private:
	/** Name of the local session that we are trying to join on the backend */
	FName SessionName{};

	/**
	 * Whether the local user attempting to join this session has already joined it from the backend perspective.
	 * If true, the JoinGameSession API call will be skipped and existing data in the pending session will be used.
	 */
	bool bHasLocalUserJoined{false};

	/** Enum used to signal what result occurred with the join session call */
	EOnJoinSessionCompleteResult::Type JoinSessionResult;

	/** Updated session info for the backend, used in Finalize to update session info with new data */
	FAccelByteModelsV2GameSession UpdatedBackendSessionInfo{};

	/** Whether or not we are trying to connect to a P2P socket for this session */
	bool bJoiningP2P{false};

	THandler<FAccelByteModelsV2GameSession> OnJoinGameSessionSuccessDelegate;
	FErrorHandler OnJoinGameSessionErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(JoinGameSession, FAccelByteModelsV2GameSession);

};

