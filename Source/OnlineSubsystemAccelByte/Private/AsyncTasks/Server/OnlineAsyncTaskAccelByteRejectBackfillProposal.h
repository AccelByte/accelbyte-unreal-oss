// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Models/AccelByteMatchmakingModels.h"

/**
 * Task to reject a backfill proposal received from matchmaking
 */
class FOnlineAsyncTaskAccelByteRejectBackfillProposal : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteRejectBackfillProposal(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& InProposal, bool bInStopBackfilling, const FOnRejectBackfillProposalComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRejectBackfillProposal");
	}

private:
	/**
	 * Name of the session that we are rejecting the backfill proposal for
	 */
	FName SessionName{};

	/**
	 * Proposal that we received from matchmaking
	 */
	FAccelByteModelsV2MatchmakingBackfillProposalNotif Proposal{};

	/**
	 * Whether or not we also want matchmaking to stop trying to backfill for our session
	 */
	bool bStopBackfilling = false;

	/**
	 * Delegate fired when we finish the task to reject the backfill proposal
	 */
	FOnRejectBackfillProposalComplete Delegate{};

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(RejectBackfillProposal);

};

