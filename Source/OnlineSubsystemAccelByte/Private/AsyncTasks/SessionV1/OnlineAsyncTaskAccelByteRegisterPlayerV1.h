// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

struct FAccelByteModelsMatchmakingResult;
struct FAccelByteModelsServerSessionResponse;

/**
 * Register an array of users to a session both on the client and on the backend.
 * 
 * For dedicated sessions, this requires the permission ADMIN:NAMESPACE:{namespace}:MATCHMAKING:CHANNEL [CREATE] on your server OAuth client.
 */
class FOnlineAsyncTaskAccelByteRegisterPlayersV1 : public FOnlineAsyncTaskAccelByte
{
public:

	/** Constructor to setup the RegisterPlayers task */
	FOnlineAsyncTaskAccelByteRegisterPlayersV1(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers, bool InBWasInvited, bool InBIsSpectator);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRegisterPlayers");
	}

private:

	/** Name of the session that we want to register these players to */
	FName SessionName;

	/** Array of players that we wish to register */
	TArray<TSharedRef<const FUniqueNetId>> Players;
	
	/** Whether these players were invited to this session or not */
	bool bWasInvited;

	/** Whether if player is spectators */
	bool bIsSpectator;

	/** Amount of players that have finished their register call, successfully or not */
	FThreadSafeCounter PendingPlayerRegistrations;

	/** Array of players that have successfully been registered to the session */
	TArray<TSharedRef<const FUniqueNetId>> SuccessfullyRegisteredPlayers;

	/**
	 * ID of the session that we are trying to register players for
	 */
	FString SessionId;
	
	/** Make a call to get information on all users that we are registering, and cache them */
	void GetAllUserInformation();

	/** Run the actual logic to register each user to the session */
	void RegisterAllPlayers();

	/**
	 * Handler for when registering player to session with session manager succeeds
	 */
	void OnRegisterPlayerSuccess(const FAccelByteModelsSessionBrowserAddPlayerResponse& Result);

	/**
	 * Handler for when registering player to session with session manager fails
	 */
	void OnRegisterPlayerError(int32 ErrorCode, const FString& ErrorMessage, FString PlayerId);

};
