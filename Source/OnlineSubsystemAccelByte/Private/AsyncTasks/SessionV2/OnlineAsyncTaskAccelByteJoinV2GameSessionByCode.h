// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Attempt to join a game session using a code. Will pass to the regular join party endpoint if successful.
 */
class FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InCode);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode");
	}

private:
	/**
	 * Local name of the session that will be created by joining this party by code
	 */
	FName SessionName{};

	/**
	 * Code that will be used to attempt to join a party
	 */
	FString Code{};

	/**
	 * Enum used to signal what result occurred with the join session call
	 */
	EOnJoinSessionCompleteResult::Type JoinSessionResult{};

	/**
	 * Information on the party that we just joined
	 */
	FAccelByteModelsV2GameSession JoinedGameSession{};

	THandler<FAccelByteModelsV2GameSession> OnJoinGameSessionByCodeSuccessDelegate;
	FErrorHandler OnJoinGameSessionByCodeErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(JoinGameSessionByCode, FAccelByteModelsV2GameSession);

};
