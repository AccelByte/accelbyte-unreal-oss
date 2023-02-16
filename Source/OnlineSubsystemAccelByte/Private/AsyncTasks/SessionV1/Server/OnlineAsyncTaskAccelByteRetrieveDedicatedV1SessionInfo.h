// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

struct FAccelByteModelsMatchmakingResult;
struct FAccelByteModelsServerSessionResponse;

/**
 * Gets information about a session that is associated with the current registered dedicated server.
 * 
 * Currently, dedicated sessions with AccelByte require that you register a server to the dedicated server manager, and
 * when a player tries to connect to the server for the first time, get session information at that point. This is because
 * session information isn't created on server registration, rather when the dedicated server is claimed by a matchmaking request.
 */
class FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo, ESPMode::ThreadSafe>
{
public:

	/** Constructor to setup the RetrieveDedicatedSessionInfo task */
	FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName, const FOnQueryDedicatedSessionInfoComplete& InDelegate = FOnQueryDedicatedSessionInfoComplete());

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo");
	}

private:

	/*** Name of the session that we are creating for matchmaking, usually GameSessionName */
	FName SessionName;

	/** ID of the session that we are querying information for */
	FString SessionId;

	/** Channel that the session is registered under */
	FString Channel;

	/** Flag determining whether the session is joinable outside of initial matchmaking or not */
	bool bIsSessionJoinable = false;

	/** Flag determining if the game is custom or from matchmaking */
	bool bIsCustomGame = false;

	/** Array of user IDs that are already associated with this session, used to register initial users */
	TArray<TSharedRef<const FUniqueNetId>> CurrentPlayers;

	/** Map of user IDs to team indices that can be used to match a player into a team on the game side, gets stored on the session info */
	TUniqueNetIdMap<int32> Teams;

	/** Nested array of user IDs that associate to a party, gets stored on session info */
	TSessionPartyArray Parties;

	FAccelByteModelsMatchmakingResult SessionResult;
	
	FOnQueryDedicatedSessionInfoComplete Delegate;
	
	/** Attempt to authenticate the dedicated server with client credentials */
	void OnAuthenticateServerComplete(bool bAuthenticationSuccessful);

	/** Method to send request to get dedicated session info */
	void TryQueryDedicatedSessionInfo();

	/** Delegate handler for when we successfully get session status from the backend */
	void OnQueryMatchSessionSuccess(const FAccelByteModelsMatchmakingResult& Result);

	/** Delegate handler for when we successfully get custom session status from the backend */
	void OnQueryCustomMatchSessionSuccess(const FAccelByteModelsSessionBrowserData& Result);

	/** Delegate handler for when we fail to get session status from the backend */
	void OnQueryMatchSessionError(int32 ErrorCode, const FString& ErrorMessage);

	void OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Result);

	void OnGetSessionIdError(int32 ErrorCode, const FString& ErrorMessage);

};
