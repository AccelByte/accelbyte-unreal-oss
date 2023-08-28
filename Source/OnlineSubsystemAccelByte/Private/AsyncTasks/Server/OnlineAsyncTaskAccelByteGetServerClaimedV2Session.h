// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteDSMModels.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Async task to get the session that has claimed this server instance
 */
class FOnlineAsyncTaskAccelByteGetServerClaimedV2Session
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetServerClaimedV2Session, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetServerClaimedV2Session(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FString& InSessionId);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetServerClaimedV2Session");
	}

private:
	/** Name of the session that we wish to get information for */
	FName SessionName{};

	/** ID of the session that has claimed this server */
	FString SessionId{};

	/** Session information from backend on the session that claimed this server */
	FAccelByteModelsV2GameSession BackendSessionInfo{};

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(GetGameSessionDetails, FAccelByteModelsV2GameSession);
};
