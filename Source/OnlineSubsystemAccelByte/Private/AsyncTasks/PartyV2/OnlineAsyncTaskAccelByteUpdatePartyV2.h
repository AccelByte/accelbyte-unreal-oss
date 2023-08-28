// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"

class FOnlineAsyncTaskAccelByteRefreshV2PartySession;

/**
 * Update a V2 party session instance with new settings
 */
class FOnlineAsyncTaskAccelByteUpdatePartyV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdatePartyV2, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdatePartyV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdatePartyV2");
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
	FAccelByteModelsV2PartySession NewSessionData;

	/**
	 * Flag indicating whether the request failed due to a version number mismatch
	 */
	bool bWasConflictError = false;

	void OnUpdatePartySessionSuccess(const FAccelByteModelsV2PartySession& BackendSessionData);
	void OnUpdatePartySessionError(int32 ErrorCode, const FString& ErrorMessage);

	void RefreshSession();
	void OnRefreshPartySessionSuccess(const FAccelByteModelsV2PartySession& Result);
	void OnRefreshPartySessionError(int32 ErrorCode, const FString& ErrorMessage);
};
