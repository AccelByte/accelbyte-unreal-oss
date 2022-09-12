// Copyright (c) 2021 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

class FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session : public FOnlineAsyncTaskAccelByte
{
public:

	/** Constructor to setup the task to register a dedicated session */
	FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session(FOnlineSubsystemAccelByte* const InABInterface, int32 InHostingPlayerNum, FName InSessionName, const FOnlineSessionSettings& InNewSessionSettings, bool InRegisterToSessionBrowser);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRegisterDedicatedSession");
	}

private:

	/**
	 * Index of the user that is hosting this server, usually zero */
	int32 HostingPlayerNum;
	
	/** Name of the local session object that we are registering */
	FName SessionName;

	/** The game mode we want to use */
	FString GameMode;

	/** Settings for the session we wish to create */
	FOnlineSessionSettings NewSessionSettings;

	/** Whether this server is a local server or not */
	bool bIsLocal = false;

	/** Name that this server is registered under */
	FString ServerName;

	/** IP address string representation that we registered the server with, used to associate the address with the session info */
	FString RegisteredIpAddress;

	/** Port that we registered the server with, used to associate the server address with the session info */
	int32 RegisteredPort;
	
	/** If this server should register to server browser immediately or wait for matchmaker to create the session */
	bool bRegisterToServerBrowser;
	
	/**
	 * Makes an SDK call to authenticate ourselves as a server client. Required to register servers to the DSM.
	 *
	 * @param SessionName Name of the session that we created when authentication was required, passed to subsequent calls.
	 */
	void OnAuthenticateServerComplete(bool bAuthenticationSuccessful);

	/**
	 * Registers either a local or Armada remote server to the DSM. Allows users to match with this server.
	 *
	 * @param SessionName Name of the session that was created for this server
	 */
	void RegisterAccelByteDedicatedServer();

	/**
	 * Get the IP address that we wish to register the server under. Will account for local, as well as Armada servers.
	 */
	void GetRegisterIpAddress(FString& IpString, int32& Port);

	void CreateGameSession();
	void RegisterCreatedGameSession(FString SessionId);
	void OnSessionCreateSuccess(const FAccelByteModelsSessionBrowserData& Data);
};
