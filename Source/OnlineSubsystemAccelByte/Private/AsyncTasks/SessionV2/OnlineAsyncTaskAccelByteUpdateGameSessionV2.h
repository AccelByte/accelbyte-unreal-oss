// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Update a V2 game session instance with new settings
 */
class FOnlineAsyncTaskAccelByteUpdateGameSessionV2 : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteUpdateGameSessionV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateGameSessionV2");
	}

private:
	/**
	 * Local name of the session that we are submitting new settings for
	 */
	FName SessionName;

	/**
	 * Settings for this session that we are updating
	 */
	FOnlineSessionSettings NewSessionSettings;

	/**
	 * Updated game session data from backend, used to update session info to have a cache of session data
	 */
	FAccelByteModelsV2GameSession NewSessionData;

	void OnUpdateGameSessionSuccess(const FAccelByteModelsV2GameSession& BackendSessionData);
	void OnUpdateGameSessionError(int32 ErrorCode, const FString& ErrorMessage);

};
