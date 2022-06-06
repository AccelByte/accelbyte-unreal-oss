// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Engine/Texture.h"
#include "AccelByteTimerObject.h"

namespace AccelByte { class FApiClient; }

/**
 * Async task to authenticate a user with the AccelByte backend, either using a native platform account, or a user specified account
 */
class FOnlineAsyncTaskAccelByteLogin : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteLogin, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteLogin(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FOnlineAccountCredentials& InAccountCredentials);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLogin");
	}

private:

	/**
	 * Credentials of the account that we wish to login with
	 */
	FOnlineAccountCredentials AccountCredentials;

	/**
	 * String representing the error code that occurredx
	 */
	FString ErrorStr;

	/**
	 * Type of login that we wish to do on the backend
	 */
	EAccelByteLoginType LoginType;

	/**
	 * Online user account for the user that we were able to login as
	 */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/**
	 * Login status for the user, should be NotLoggedIn, unless login succeeds fully
	 */
	ELoginStatus::Type LoginStatus = ELoginStatus::NotLoggedIn;

	FAccelByteTimerObject TimerObject;

	/*
	 * Unique ID of the user account that we logged into on the native platform, will be invalid if we did not login with native platform
	 */
	TSharedPtr<const FUniqueNetId> NativePlatformPlayerId = nullptr;

	/**
	 * Attempts to fire off a login request with a native subsystem, if one is set up and usable.
	 *
	 * @param LocalUserNum Index of the user that we want to try and auth with native subsystem pass through
	 * @returns bool that is true if a native subsystem login was fired off, or false if not
	 */
	void LoginWithNativeSubsystem();

	/**
	 * Callback for delegate fired when the native subsystem has finished its authentication. Authenticates with the AccelByte back end upon firing.
	 */
	void OnNativeLoginComplete(int32 NativeLocalUserNum, bool bWasNativeLoginSuccessful, const FUniqueNetId& NativeUserId, const FString& NativeError);

	/**
	 * Callback for when the login UI for a native platform subsystem is closed. Used to allow sign in with local user
	 * accounts on a native subsystem and then subsequently login with the AccelByte back end. Passing in an extra LocalUserNum,
	 * as ControllerIndex is -1 if the call fails and we need to be able to inform that the login failed.
	 */
	void OnNativeLoginUIClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex, const FOnlineError& NativeError);

	/**
	 * Perform login on the AccelByte backend using the login type specified
	 */
	void PerformLoginWithType(const EAccelByteLoginType& LoginType, const FOnlineAccountCredentials& Credentials);

	/**
	 * Delegate handler for when any AccelByte login call succeeds.
	 *
	 * @param LocalUserNum Index of the user that the login succeeded for
	 */
	void OnLoginSuccess();

	/**
	 * Delegate handler for when any AccelByte login call fails.
	 *
	 * @param ErrorCode Code returned from the backend representing the error that was encountered with the request
	 * @param ErrorMessage Message returned from the backend describing the error that was encountered with the request
	 * @param LocalUserNum Index of the user that the login failed for
	 */
	void OnLoginError(int32 ErrorCode, const FString& ErrorMessage);

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

	/** Delegate handler for when a lobby connection is disconnected. */
	static void OnLobbyConnectionClosed(int32 StatusCode, const FString& Reason, bool WasClean, int32 InLocalUserNum);

	static void OnLobbyReconnected(int32 InLocalUserNum);

	void UnbindDelegates();

	/**
	 * Method to calculate a local offset timestamp from UTC
	 */
	FString GetLocalTimeOffsetFromUTC();

};
