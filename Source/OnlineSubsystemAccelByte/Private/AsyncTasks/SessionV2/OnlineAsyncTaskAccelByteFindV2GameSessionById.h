// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

/**
 * Find a single session by its ID
 */
class FOnlineAsyncTaskAccelByteFindV2GameSessionById : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteFindV2GameSessionById(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const FUniqueNetId& InSessionId, const FOnSingleSessionResultCompleteDelegate& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteFindV2SessionById");
	}

private:
	/**
	 * ID of the session that we are trying to find
	 */
	TSharedRef<const FUniqueNetIdAccelByteResource> SessionId;
	
	/**
	 * Delegate fired when we finish our attempt to find this single session
	 */
	FOnSingleSessionResultCompleteDelegate Delegate;

	/**
	 * Game session instance that we found on the backend from the session ID
	 */
	FAccelByteModelsV2GameSession FoundGameSession;

	/**
	 * Session search result constructed from our found game session
	 */
	FOnlineSessionSearchResult FoundSessionResult;

	void OnGetGameSessionDetailsSuccess(const FAccelByteModelsV2GameSession& InFoundGameSession);
	void OnGetGameSessionDetailsError(int32 ErrorCode, const FString& ErrorMessage);
};
