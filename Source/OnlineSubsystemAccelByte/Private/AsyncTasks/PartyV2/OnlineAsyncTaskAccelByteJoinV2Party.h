// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"

/**
 * Async Task to join a party session 
 */
class FOnlineAsyncTaskAccelByteJoinV2Party
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteJoinV2Party, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteJoinV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, bool bInIsRestoreSession);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJoinV2Party");
	}

private:
	/** Name of the session to create if join is successful */
	FName SessionName{};

	/** Whether or not we are just restoring this party session. If true, we will only get the up to date session details and then finalize join. */
	bool bIsRestoreSession{false};

	/** Information on the party just obtained from the backend */
	FAccelByteModelsV2PartySession PartyInfo{};

	/** Enum used to signal what result occurred with the join session call */
	EOnJoinSessionCompleteResult::Type JoinSessionResult;

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(JoinParty, FAccelByteModelsV2PartySession);
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(GetPartyDetails, FAccelByteModelsV2PartySession);
};

