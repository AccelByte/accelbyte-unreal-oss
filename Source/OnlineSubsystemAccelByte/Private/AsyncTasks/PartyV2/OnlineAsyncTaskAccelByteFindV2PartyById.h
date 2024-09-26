// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

/**
 * Try and find a single party session by its ID
 */
class FOnlineAsyncTaskAccelByteFindV2PartyById
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteFindV2PartyById, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteFindV2PartyById(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const FUniqueNetId& InSessionId, const FOnSingleSessionResultCompleteDelegate& InDelegate, TSharedPtr<FAccelByteKey> InLockKey = nullptr);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetV2PartyById");
	}

private:
	/**
	 * ID of the party session that we are trying to find
	 */
	TSharedRef<const FUniqueNetIdAccelByteResource> SessionId;

	/**
	 * Delegate fired when we finish our attempt to find this single session
	 */
	FOnSingleSessionResultCompleteDelegate Delegate;

	/**
	 * Party session instance that we found on the backend from the session ID
	 */
	FAccelByteModelsV2PartySession FoundPartySession;

	/**
	 * Session search result constructed from our found party session
	 */
	FOnlineSessionSearchResult FoundSessionResult;

	THandler<FAccelByteModelsV2PartySession> OnGetPartySessionDetailsSuccessDelegate;
	void OnGetPartySessionDetailsSuccess(const FAccelByteModelsV2PartySession& InFoundPartySession);

	FErrorHandler OnGetPartySessionDetailsErrorDelegate;
	void OnGetPartySessionDetailsError(int32 ErrorCode, const FString& ErrorMessage);
};

