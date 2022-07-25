// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"

/**
 * Task to send a request to backend that we wish to join the session associated with SessionName
 */
class FOnlineAsyncTaskAccelByteJoinV2GameSession : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteJoinV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJoinV2GameSession");
	}

private:
	/** Name of the local session that we are trying to join on the backend */
	FName SessionName;

	/** Enum used to signal what result occurred with the join session call */
	EOnJoinSessionCompleteResult::Type JoinSessionResult;

	/** Updated session info for the backend, used in Finalize to update session info with new data */
	FAccelByteModelsV2GameSession UpdatedBackendSessionInfo;

	void OnJoinGameSessionSuccess(const FAccelByteModelsV2GameSession& InUpdatedBackendSessionInfo);
	void OnJoinGameSessionError(int32 ErrorCode, const FString& ErrorMessage);
};

