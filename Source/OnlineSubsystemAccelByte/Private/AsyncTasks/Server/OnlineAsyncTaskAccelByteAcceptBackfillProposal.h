﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Models/AccelByteMatchmakingModels.h"

/**
 * Task to accept a backfill proposal received from matchmaking
 */
class FOnlineAsyncTaskAccelByteAcceptBackfillProposal
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteAcceptBackfillProposal, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteAcceptBackfillProposal(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& InProposal, const FAccelByteModelsV2MatchmakingBackfillAcceptanceOptionalParam& OptionalParameter, bool bInStopBackfilling, const FOnAcceptBackfillProposalComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteAcceptBackfillProposal");
	}

private:
	/**
	 * Name of the session that we are accepting the backfill proposal for
	 */
	FName SessionName{};

	/**
	 * Proposal that we received from matchmaking
	 */
	FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal{};

	/*
	 * Specify this optional param to modify the acceptance behavior.
	 */
	FAccelByteModelsV2MatchmakingBackfillAcceptanceOptionalParam OptionalParameter{};

	/**
	 * Whether or not we also want matchmaking to stop trying to backfill for our session
	 */
	bool bStopBackfilling = false;

	/**
	 * Delegate fired when we finish the task to accept the backfill proposal
	 */
	FOnAcceptBackfillProposalComplete Delegate{};

	/**
	 * Store game session info from accept backfill response for updating internal game session.
	 */
	FAccelByteModelsV2GameSession GameSessionInfo;

	THandler<FAccelByteModelsV2GameSession> OnAcceptBackfillProposalSuccessDelegate;
	FErrorHandler OnAcceptBackfillProposalErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(AcceptBackfillProposal, FAccelByteModelsV2GameSession);
};

