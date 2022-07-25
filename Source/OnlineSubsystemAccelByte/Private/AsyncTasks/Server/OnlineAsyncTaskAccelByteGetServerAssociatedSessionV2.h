// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteDSMModels.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Async task to get the currently associated session for this server on the backend, and then create a new session
 * instance locally to reflect state.
 */
class FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2 : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2");
	}

private:
	/** Name of the session that we wish to restore to */
	FName SessionName{};

	/** ID of the session that we have found associated with this server */
	FString SessionId{};

	/** Session information from backend for finalizing session creation */
	FAccelByteModelsV2GameSession BackendSessionInfo;

	void OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Result);
	void OnGetSessionIdError(int32 ErrorCode, const FString& ErrorMessage);

	/** Method that tries to request session data from backend */
	void RequestSessionData();

	void OnGetGameSessionDetailsSuccess(const FAccelByteModelsV2GameSession& Result);
	void OnGetGameSessionDetailsError(int32 ErrorCode, const FString& ErrorMessage);
};
