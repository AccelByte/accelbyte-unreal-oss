// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineAgreementInterfaceAccelByte.h"
#include "AccelByteTimerObject.h"
#if defined(STEAM_SDK_VER) && !UE_SERVER
#include "steam/steam_api.h"
#endif

namespace AccelByte { class FApiClient; }


/**
 * Async task to authenticate a user with the AccelByte backend, either using a native platform account, or a user specified account
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskAccelByteLogin
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLogin, ESPMode::ThreadSafe>
{
#if defined(STEAM_SDK_VER) && !UE_SERVER
	class FAccelByteSteamAuthCallback {
		public:
		FAccelByteSteamAuthCallback(const TDelegate<void(GetAuthSessionTicketResponse_t*)>& InDelegate);
		~FAccelByteSteamAuthCallback();
		
		private:
		STEAM_CALLBACK(FAccelByteSteamAuthCallback, OnGetAuthSessionTicketResponse, GetAuthSessionTicketResponse_t, OnGetAuthSessionTicketResponseCallback);
		TDelegate<void(GetAuthSessionTicketResponse_t*)> Delegate;
	};
#endif
public:

	FOnlineAsyncTaskAccelByteLogin(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalUserNum
		, const FOnlineAccountCredentialsAccelByte& InAccountCredentials
		, bool bInCreateHeadlessAccount = true);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLogin");
	}

	/**
	 * User number or the controller index of the player
	 */
	int32 LoginUserNum;
	
	/**
	 * Credentials of the account that we wish to login with
	 */
	FOnlineAccountCredentialsAccelByte AccountCredentials;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	/**
	 * Object representing the error code that occurred
	 */
	FErrorOAuthInfo  ErrorOAuthObject;

	/**
	 * Digit code representing the error that occurred
	 */
	int32 ErrorCode;

	/**
	 * Type of login that we wish to do on the backend
	 */
	EAccelByteLoginType LoginType;

	/**
	 * Online user account for the user that we were able to login as
	 */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/*
	 * for cache user data, such as display names and platform IDs, used across the online subsystem. 
	 */
	TSharedPtr<FOnlineUserCacheAccelByte, ESPMode::ThreadSafe> UserCache;

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
	FUniqueNetIdPtr NativePlatformPlayerId = nullptr;

	/*
	 * Unique ID of the user account that we logged into with the secondary platform, will be invalid if we did not login with a secondary platform
	 */
	FUniqueNetIdPtr SecondaryPlatformPlayerId = nullptr;

	/*
	 * Credential for Native platform login
	 */
	FOnlineAccountCredentials NativePlatformCredentials{};

	/*
	 * Flag to know whether the login is already performed or not, mainly used on native login on steam, so it won't performed twice from the timer and the callback.
	 */
	bool bLoginPerformed{false};

	/*
	 * The platform name which user login with 
	 */
	FString PlatformId{};

	/*
	 * Will automatically create a headless account when login with 3rd party platform if set to true
	 */
	bool bCreateHeadlessAccount = true;

	/*
	 * Flag denoting whether the current log in is for the native platform rather than secondary platform.
	 */
	bool bIsNativePlatformCredentialLogin = true;

	/*
	 * flag to know if a login process is waiting for queue
	 */
	bool bLoginInQueue = false;

	/** Set from DefaultEngine.ini, will only notify in queue if estimated time is above presentation threshold */
	int32 LoginQueuePresentationThreshold{0};

	FDelegateHandle LoginCompletedHandle;

#if defined(STEAM_SDK_VER) && !UE_SERVER
	/*
	 * Object to bind steam auth callback, will only be created when login with steam
	 */
	TSharedPtr<FAccelByteSteamAuthCallback> SteamAuthCallback = nullptr;

#endif

	/**
	 * Attempts to fire off a login request with a native subsystem, if one is set up and usable.
	 *
	 * @param LocalUserNum Index of the user that we want to try and auth with native subsystem pass through
	 * @returns bool that is true if a native subsystem login was fired off, or false if not
	 */
	void LoginWithNativeSubsystem();

	/**
	 * Attempts to fire off a login request with a specific subsystem, if one is set up and usable.
	 */
	void LoginWithSpecificSubsystem(IOnlineSubsystem* NativeSubsystem);
	void LoginWithSpecificSubsystem(FName InSubsystemName);

	/**
	 * Callback for delegate fired when the `specified` subsystem has finished its authentication. Authenticates with the AccelByte back end upon firing.
	 *
	 * Specific subsystem need to be passed to maintain consistency between LoginWithSpecificSubsystem() and the delegate
	 */
	virtual void OnSpecificSubysystemLoginComplete(int32 LocalUserNum, bool bWasSubsystemLoginSuccessful, const FUniqueNetId& SubsystemUserId, const FString& SubsystemError, IOnlineSubsystem* SpecificSubsystem);

	/**
	 * Callback for when the login UI for a `specified` platform subsystem is closed. Used to allow sign in with local user
	 * accounts on a `specified` subsystem and then subsequently login with the AccelByte back end. Passing in an extra LocalUserNum,
	 * as ControllerIndex is -1 if the call fails and we need to be able to inform that the login failed.
	 * 
	 * Specific subsystem need to be passed to maintain consistency between LoginWithSpecificSubsystem() and the delegate
	 */
	void OnSpecificSubysystemLoginUIClosed(FUniqueNetIdPtr UniqueId, const int ControllerIndex, const FOnlineError& SubsystemError, IOnlineSubsystem* SpecificSubsystem);
	
	/**
	 * Perform login on the AccelByte backend using defined login type and OAuth error type
	 */
	virtual void PerformLogin(const FOnlineAccountCredentials& Credentials);
	
	/**
	 * Delegate handler for when any AccelByte login call succeeds. 
	 */
	virtual void OnLoginSuccess();

	/**
	 * Delegate handler for when any AccelByte login v4 endpoints call succeeds. 
	 */
	virtual void OnLoginSuccessV4(const FAccelByteModelsLoginQueueTicketInfo& TicketInfo);

	virtual void Tick() override;

	/**
	 * Delegate handler for when any AccelByte login call fails.
	 *
	 * @param ErrorCode Code returned from the backend representing the error that was encountered with the request
	 * @param ErrorMessage Message returned from the backend describing the error that was encountered with the request
	 * @param ErrorObject Object representing the error code that occurred
	 */
	void OnLoginErrorOAuth(int32 ErrorCode, const FString& ErrorMessage, const FErrorOAuthInfo& ErrorObject);

	FDelegateHandle OnLoginQueueCancelledDelegateHandle;
	FAccelByteOnLoginQueueCanceledByUserDelegate OnLoginQueueCancelledDelegate;

	void OnLoginQueueCancelled(int32 LoginUserNum);

	FDelegateHandle OnLoginQueueClaimTicketCompleteDelegateHandle;
	FAccelByteOnLoginQueueClaimTicketCompleteDelegate OnLoginQueueClaimTicketCompleteDelegate;

	void OnLoginQueueTicketClaimed(int32 InLoginUserNum, bool bWasClaimSuccessful, const FErrorOAuthInfo& ErrorObject);

	bool ShouldInitiateNativePlatformLogin(FOnlineAccountCredentials const& InAccountCredentials, FName& OutSubsystemName);

#if defined(STEAM_SDK_VER) && !UE_SERVER
	/**
	* Delegate handler for when received response from steam getAuthToken
	*/
	void OnGetAuthSessionTicketResponse(GetAuthSessionTicketResponse_t* CallBackParam);
#endif

	bool FetchOculusPlatformToken();

	void OnFetchOculusPlatformTokenFinished(const FString& Token);
};
