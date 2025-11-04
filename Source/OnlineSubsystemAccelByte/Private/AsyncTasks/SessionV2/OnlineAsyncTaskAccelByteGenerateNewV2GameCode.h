// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

/**
 * Request that the backend generate a new party code for the party session passed in
 */
class FOnlineAsyncTaskAccelByteGenerateNewV2GameCode
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGenerateNewV2GameCode, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGenerateNewV2GameCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnGenerateNewGameCodeComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGenerateNewV2GameCode");
	}

private:
	/**
	 * Name of the local session that we are regenerating party code for
	 */
	FName SessionName{};

	/**
	 * Delegate fired when we finish generating a new game code for the session
	 */
	FOnGenerateNewPartyCodeComplete Delegate{};

	/**
	 * Game session data resulting from generating a new code
	 */
	FAccelByteModelsV2GameSession UpdatedGameSession{};

	THandler<FAccelByteModelsV2GameSession> OnGenerateNewCodeSuccessDelegate;
	FErrorHandler OnGenerateNewCodeErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(GenerateNewCode, FAccelByteModelsV2GameSession)

};

