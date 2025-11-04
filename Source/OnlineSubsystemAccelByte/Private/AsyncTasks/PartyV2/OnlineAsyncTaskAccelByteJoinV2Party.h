// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

/**
 * Async Task to join a party session 
 */
class FOnlineAsyncTaskAccelByteJoinV2Party
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteJoinV2Party, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteJoinV2Party(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InLocalUserId
		, const FName& InSessionName
		, bool bInHasLocalUserJoined);

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

	/**
	 * Whether the local user attempting to join this session has already joined it from the backend perspective.
	 * If true, the JoinParty API call will be skipped and existing data in the pending session will be used.
	 */
	bool bHasLocalUserJoined{false};

	/** Information on the party just obtained from the backend */
	FAccelByteModelsV2PartySession PartyInfo{};

	/** Enum used to signal what result occurred with the join session call */
	EOnJoinSessionCompleteResult::Type JoinSessionResult;

	THandler<FAccelByteModelsV2PartySession> OnJoinPartySuccessDelegate;
	FErrorHandler OnJoinPartyErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(JoinParty, FAccelByteModelsV2PartySession);
};

