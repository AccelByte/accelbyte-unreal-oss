// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

class FOnlineAsyncTaskAccelByteStartV1Matchmaking
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteStartV1Matchmaking, ESPMode::ThreadSafe>
{
public:

	/** Constructor to setup the StartMatchmaking task */
	FOnlineAsyncTaskAccelByteStartV1Matchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TArray<TSharedRef<const FUniqueNetId>>& InLocalPlayers, FName InSessionName, const FOnlineSessionSettings& InNewSessionSettings, TSharedRef<FOnlineSessionSearch>& InSearchSettings);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteStartMatchmaking");
	}

private:

	/*** Array of user IDs corresponding to the users that we are matchmaking with */
	TArray<TSharedRef<const FUniqueNetId>> LocalPlayers;

	/*** Name of the session that we are creating for matchmaking, usually GameSessionName */
	FName SessionName;

	/** Desired settings for this matchmaking session */
	FOnlineSessionSettings NewSessionSettings;

	/** Search parameters for the matchmaking request */
	TSharedRef<FOnlineSessionSearch> SearchSettings;

	/** The key of the error message for the ErrorStrings table. */
	FString ErrorStringKey;

	/** Array of latency pairs that will get passed to matchmaking */
	TArray<TPair<FString, float>> Latencies;

	/**
	 * Delegate handler for when we get a successful response back for getting server latencies
	 */
	void OnGetServerLatenciesSuccess(const TArray<TPair<FString, float>>& Result);

	/**
	 * Delegate handler for when we get an error response back for getting server latencies
	 */
	void OnGetServerLatenciesError(int32 ErrorCode, const FString& ErrorMessage);

	/**
	 * Method to create a new named session set up for matchmaking and to send off the request to start matchmaking with the AccelByte backend.
	 */
	void CreateMatchmakingSessionAndStartMatchmaking();

	void OnStartMatchmakingResponseReceived(const FAccelByteModelsMatchmakingResponse& Result);

};
