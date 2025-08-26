// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Async task to refresh local party session data with data from the backend
 */
class FOnlineAsyncTaskAccelByteRefreshV2PartySession
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRefreshV2PartySession, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteRefreshV2PartySession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnRefreshSessionComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRefreshV2PartySession");
	}

private:
	/**
	 * Local name of the session that we are refreshing
	 */
	FName SessionName{};

	/**
	 * Delegate fired when we finish the task to refresh the session
	 */
	FOnRefreshSessionComplete Delegate{};

	/**
	 * Model from the backend for a party session, used to refresh local data in the interface
	 */
	FAccelByteModelsV2PartySession RefreshedPartySession{};

	/**
	 * Whether the session was returned as not found by the backend, meaning that it was deleted
	 */
	bool bWasSessionRemoved{false};

	THandler<FAccelByteModelsV2PartySession> OnRefreshPartySessionSuccessDelegate;
	FErrorHandler OnRefreshPartySessionErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(RefreshPartySession, FAccelByteModelsV2PartySession);

};

