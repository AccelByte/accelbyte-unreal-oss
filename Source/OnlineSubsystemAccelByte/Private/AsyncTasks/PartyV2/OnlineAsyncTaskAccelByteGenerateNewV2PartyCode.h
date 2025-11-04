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
class FOnlineAsyncTaskAccelByteGenerateNewV2PartyCode
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGenerateNewV2PartyCode, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGenerateNewV2PartyCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnGenerateNewPartyCodeComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGenerateNewV2PartyCode");
	}

private:
	/**
	 * Name of the local session that we are regenerating party code for
	 */
	FName SessionName{};

	/**
	 * Delegate fired when we finish generating a new party code for the session
	 */
	FOnGenerateNewPartyCodeComplete Delegate{};

	/**
	 * Party session data resulting from generating a new code
	 */
	FAccelByteModelsV2PartySession UpdatedPartySession{};

	THandler<FAccelByteModelsV2PartySession> OnGenerateNewCodeSuccessDelegate;
	FErrorHandler OnGenerateNewCodeErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(GenerateNewCode, FAccelByteModelsV2PartySession)
};

