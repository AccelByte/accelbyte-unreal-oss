// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Task for connect AccelByte Lobby
 */
class FOnlineAsyncTaskAccelByteConnectLobby
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteConnectLobby, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteConnectLobby(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const bool InSuppressConnectSuccessIfAlreadyConnected = false);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteConnectLobby");
	}

private:

	/**
	 * Delegate handler fired when the lobby websocket successfully connects.
	 *
	 * @param LocalUserNum Index of the user that successfully connected to lobby.
	 * @param UserId NetId of the user that successfully connected to lobby.
	 */
	void OnLobbyConnectSuccess();
	AccelByte::Api::Lobby::FConnectSuccess OnLobbyConnectSuccessDelegate;

	/**
	 * Delegate handler fired when we fail to connect to the lobby websocket.
	 */
	void OnLobbyConnectError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnLobbyConnectErrorDelegate;

	/** Delegate handler for when receive a lobby disconnected notification. */
	void OnLobbyDisconnectedNotif(const FAccelByteModelsDisconnectNotif&);
	AccelByte::Api::Lobby::FDisconnectNotif OnLobbyDisconnectedNotifDelegate;

	void UnbindDelegates();

	/**
	 * Create a new token generator for this lobby connection based on configuration values.
	 */
	TSharedPtr<AccelByte::IAccelByteTokenGenerator> CreateTokenGenerator();

	/**
	 * String representing the error code that occurredx
	 */
	FString ErrorStr;

	/**
	 * Suppress calling connection success if already connected when executing this task
	*/
	bool SuppressConnectSuccessIfAlreadyConnected{ false };
};
