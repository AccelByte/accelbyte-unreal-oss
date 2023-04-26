// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

/**
 * Unregister an array of users from a session both on the client and on the backend.
 *
 * For dedicated sessions, this requires the permission ADMIN:NAMESPACE:{namespace}:MATCHMAKING:CHANNEL [DELETE] on your server OAuth client.
 */
class FOnlineAsyncTaskAccelByteUnregisterPlayersV1 : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteUnregisterPlayersV1, ESPMode::ThreadSafe>
{
public:

	/** Constructor to setup the RegisterPlayers task */
	FOnlineAsyncTaskAccelByteUnregisterPlayersV1(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUnregisterPlayers");
	}

private:

	/** Name of the session that we want to unregister these players from */
	FName SessionName;

	/** Array of players that we wish to unregister */
	TArray<TSharedRef<const FUniqueNetId>> Players;

	/** Amount of players that have we are waiting to finish their unregister call, successfully or not */
	FThreadSafeCounter PendingPlayerUnregistrations;

	/** Array of players that have successfully been unregistered from session */
	TArray<TSharedRef<const FUniqueNetId>> SuccessfullyUnregisteredPlayers;
	
	/**
	 * Handler for when our call to unregister a single player has succeeded on the backend
	 */
	void OnUnregisterPlayerFromSessionSuccess(const FAccelByteModelsSessionBrowserAddPlayerResponse& Result, int32 Index);

	/**
	 * Handler for when our call to unregister a single player has failed on the backend
	 */
	void OnUnregisterPlayerFromSessionError(int32 ErrorCode, const FString& ErrorMessage, FString PlayerId, int32 Index);

};
