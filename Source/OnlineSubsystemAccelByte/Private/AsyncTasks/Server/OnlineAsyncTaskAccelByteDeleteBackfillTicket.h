// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

/**
 * Task to delete a ticket used for backfilling the session provided
 */
class FOnlineAsyncTaskAccelByteDeleteBackfillTicket
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteDeleteBackfillTicket, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteDeleteBackfillTicket(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnDeleteBackfillTicketComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDeleteBackfillTicket");
	}

private:
	/**
	 * Name of the session that we are deleting a backfill ticket for
	 */
	FName SessionName{};

	/**
	 * Delegate fired when we finish the task to delete the backfill ticket
	 */
	FOnAcceptBackfillProposalComplete Delegate{};

	FVoidHandler OnDeleteBackfillTicketSuccessDelegate;
	FErrorHandler OnDeleteBackfillTicketErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(DeleteBackfillTicket);

};

