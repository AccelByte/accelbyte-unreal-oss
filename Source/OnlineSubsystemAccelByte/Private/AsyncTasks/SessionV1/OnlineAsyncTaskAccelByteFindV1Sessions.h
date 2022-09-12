// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

/**
 * Async task to search for either dedicated or P2P sessions through the SessionBrowser APIs.
 */
class FOnlineAsyncTaskAccelByteFindV1Sessions : public FOnlineAsyncTaskAccelByte
{
public:

	/** Constructor to setup the FindSessions task */
	FOnlineAsyncTaskAccelByteFindV1Sessions(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& InSearchSettings);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteFindSessions");
	}

private:

	/** Settings that we wish to use to find sessions from the backend */
	TSharedRef<FOnlineSessionSearch> SearchSettings;

	/** Sessions that were found for this particular search request */
	TArray<FOnlineSessionSearchResult> SearchResults;

	/** Time that we started the session search, used to implement a timeout */
	double SearchStartTimeSeconds = 0.0;

	/** Delegate handler for when we successfully find sessions from the session browser */
	void OnSessionBrowserFindSuccess(const FAccelByteModelsSessionBrowserGetResult& Result);

};
