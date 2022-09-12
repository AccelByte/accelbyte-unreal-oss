// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Async task to create a party for the user on the backend
 */
class FOnlineAsyncTaskAccelByteCreateV2Party : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteCreateV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateV2Party");
	}

private:

	/** Name of the session to create */
	FName SessionName;

	/** Join type to be used when creating the session */
	EAccelByteV2SessionJoinability JoinType;

	/** Information on the party just created from the backend */
	FAccelByteModelsV2PartySession PartyInfo;

	/** Settings for the session that we wish to create */
	FOnlineSessionSettings NewSessionSettings;

	/** Delegate handler for when our request to get the user's active parties from the backend succeeds */
	void OnGetMyPartiesSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result);

	/** Delegate handler for when our request to get the user's active parties from the backend fails */
	void OnGetMyPartiesError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when we successfully create a party from the backend */
	void OnCreatePartySuccess(const FAccelByteModelsV2PartySession& Result);

	/** Delegate handler for when we fail to create a party from the backend */
	void OnCreatePartyError(int32 ErrorCode, const FString& ErrorMessage);
};
