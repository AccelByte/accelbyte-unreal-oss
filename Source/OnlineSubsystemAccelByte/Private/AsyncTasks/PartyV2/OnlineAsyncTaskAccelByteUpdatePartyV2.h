// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"

/**
 * Update a V2 party session instance with new settings
 */
class FOnlineAsyncTaskAccelByteUpdatePartyV2 : public FOnlineAsyncTaskAccelByte
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

	void OnUpdatePartySessionSuccess(const FAccelByteModelsV2PartySession& BackendSessionData);
	void OnUpdatePartySessionError(int32 ErrorCode, const FString& ErrorMessage);

};
