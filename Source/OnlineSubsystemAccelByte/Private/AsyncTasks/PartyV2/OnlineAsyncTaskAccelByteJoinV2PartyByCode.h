// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Attempt to join a party session using a code. Will pass to the regular join party endpoint if successful.
 */
class FOnlineAsyncTaskAccelByteJoinV2PartyByCode
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteJoinV2PartyByCode, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteJoinV2PartyByCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InPartyCode);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJoinV2PartyByCode");
	}

private:
	/**
	 * Local name of the session that will be created by joining this party by code
	 */
	FName SessionName{};

	/**
	 * Code that will be used to attempt to join a party
	 */
	FString PartyCode{};

	/**
	 * Enum used to signal what result occurred with the join session call
	 */
	EOnJoinSessionCompleteResult::Type JoinSessionResult{};

	/**
	 * Information on the party that we just joined
	 */
	FAccelByteModelsV2PartySession JoinedPartySession{};

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(JoinPartyByCode, FAccelByteModelsV2PartySession);

};
