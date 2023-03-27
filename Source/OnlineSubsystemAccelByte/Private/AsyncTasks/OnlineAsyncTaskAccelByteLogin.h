// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineAgreementInterfaceAccelByte.h"
#include "Engine/Texture.h"
#include "AccelByteTimerObject.h"

namespace AccelByte { class FApiClient; }

/**
 * Async task to authenticate a user with the AccelByte backend, either using a native platform account, or a user specified account
 */
class FOnlineAsyncTaskAccelByteLogin : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteLogin, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteLogin(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalUserNum
		, const FOnlineAccountCredentials& InAccountCredentials);

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
	 * User number or the controller index of the player
	 */
	int32 LoginUserNum;
	
	/**
	 * Credentials of the account that we wish to login with
	 */
	FOnlineAccountCredentials AccountCredentials;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	/**
	 * Object representing the error code that occurred
	 */
	FErrorOAuthInfo  ErrorOAuthObject;

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

	/**
	 * Flag if login by External UI is failed. It will retry the Native login and skip the login UI
	 */
	bool bRetryLoginSkipExternalUI {false};

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
	 * Perform login on the AccelByte backend using defined login type and OAuth error type
	 */
	void PerformLogin(const FOnlineAccountCredentials& Credentials);
	
	/**
	 * Delegate handler for when any AccelByte login call succeeds. 
	 */
	void OnLoginSuccess();

	/**
	 * Delegate handler for when any AccelByte login call fails.
	 *
	 * @param ErrorCode Code returned from the backend representing the error that was encountered with the request
	 * @param ErrorMessage Message returned from the backend describing the error that was encountered with the request
	 */
	void OnLoginError(int32 ErrorCode, const FString& ErrorMessage);

	/**
	 * Delegate handler for when any AccelByte login call fails.
	 *
	 * @param ErrorCode Code returned from the backend representing the error that was encountered with the request
	 * @param ErrorMessage Message returned from the backend describing the error that was encountered with the request
	 * @param ErrorObject Object representing the error code that occurred
	 */
	void OnLoginErrorOAuth(int32 ErrorCode, const FString& ErrorMessage, const FErrorOAuthInfo& ErrorObject);

	/**
	 * Method to calculate a local offset timestamp from UTC
	 */
	FString GetLocalTimeOffsetFromUTC();

};
