// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Task to create a new backfill ticket for the session provided
 */
class FOnlineAsyncTaskAccelByteCreateBackfillTicket : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteCreateBackfillTicket(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FString& InMatchPool, const FOnCreateBackfillTicketComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateBackfillTicket");
	}

private:
	/**
	 * Name of the session that we are creating a backfill ticket for
	 */
	FName SessionName{};

	/**
	 * Match pool that we are creating this backfill ticket against. Could be blank, in which case we want to use the
	 * match pool from the session passed in.
	 */
	FString MatchPool{};

	/**
	 * ID of the backfill ticket created from the request. Will be stored in the session's settings in case the developer
	 * needs to access it.
	 */
	FString BackfillTicketId{};

	/**
	 * Delegate fired when we finish the task to create the backfill ticket
	 */
	FOnAcceptBackfillProposalComplete Delegate{};

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(CreateBackfillTicket, FAccelByteModelsV2MatchmakingCreateBackfillTicketResponse);

};

