// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Core/AccelByteUtilities.h"
#include "OnlineErrorAccelByte.h"
#include "InterfaceModels/OnlineIdentityInterfaceAccelByteModels.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "OnlineSubsystemAccelBytePackage.h"

typedef FOnlineAccountCredentialsAccelByte FOnlineAccelByteAccountCredentials;

DECLARE_DELEGATE_OneParam(FOnAuthenticateServerComplete, bool /*bWasSuccessful*/)
DECLARE_MULTICAST_DELEGATE_FourParams(FOnConnectLobbyComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*Error*/);
typedef FOnConnectLobbyComplete::FDelegate FOnConnectLobbyCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnLoginWithOAuthErrorComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FErrorOAuthInfo& /*Error*/);
typedef FOnLoginWithOAuthErrorComplete::FDelegate FOnLoginWithOAuthErrorCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FAccelByteOnLoginComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnLoginComplete::FDelegate FAccelByteOnLoginCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FiveParams(FAccelByteOnSimultaneousLoginComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, ESimultaneousLoginResult /*Result*/, const FUniqueNetId& /*UserId*/, const FErrorOAuthInfo& /*Error*/);
typedef FAccelByteOnSimultaneousLoginComplete::FDelegate FAccelByteOnSimultaneousLoginCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FAccelByteOnLogoutComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnLogoutComplete::FDelegate FAccelByteOnLogoutCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FAccelByteOnConnectLobbyComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnConnectLobbyComplete::FDelegate FAccelByteOnConnectLobbyCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FiveParams(FAccelByteOnLobbyConnectionClosed, int32 /*LocalUserNum*/, const FUniqueNetId& /*UserId*/, int32 /*StatusCode*/, const FString& /*Reason*/, bool /*bWasClean*/);
typedef FAccelByteOnLobbyConnectionClosed::FDelegate FAccelByteOnLobbyConnectionClosedDelegate;

DECLARE_MULTICAST_DELEGATE_FiveParams(FAccelByteOnLobbyReconnecting, int32 /*LocalUserNum*/, const FUniqueNetId& /*UserId*/, int32 /*StatusCode*/, const FString& /*Reason*/, bool /*bWasClean*/);
typedef FAccelByteOnLobbyReconnecting::FDelegate FAccelByteOnLobbyReconnectingDelegate;

DECLARE_MULTICAST_DELEGATE(FAccelByteOnLobbyReconnected);
typedef FAccelByteOnLobbyReconnected::FDelegate FAccelByteOnLobbyReconnectedDelegate;

DECLARE_DELEGATE_TwoParams(FGenerateCodeForPublisherTokenComplete, bool /*bWasSuccessful*/, const FCodeForTokenExchangeResponse& Result /*Result*/);

DECLARE_MULTICAST_DELEGATE_FiveParams(FAccelByteOnPlatformTokenRefreshedComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FPlatformTokenRefreshResponse& /*RefreshResponse*/, const FName& /*PlatformName*/, const FErrorOAuthInfo& /*Error*/);
typedef FAccelByteOnPlatformTokenRefreshedComplete::FDelegate FAccelByteOnPlatformTokenRefreshedCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FAccelByteOnLoginQueued, int32 /*LocalUserNum*/, const FAccelByteModelsLoginQueueTicketInfo& /*TicketInfo*/);
typedef FAccelByteOnLoginQueued::FDelegate FAccelByteOnLoginQueuedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FAccelByteOnLoginQueueCancelComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnLoginQueueCancelComplete::FDelegate FAccelByteOnLoginQueueCancelCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FAccelByteOnLoginQueueCanceledByUser, int32 /*LocalUserNum*/);
typedef FAccelByteOnLoginQueueCanceledByUser::FDelegate FAccelByteOnLoginQueueCanceledByUserDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FAccelByteOnLoginTicketStatusUpdated, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsLoginQueueTicketInfo& /*TicketInfo*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnLoginTicketStatusUpdated::FDelegate FAccelByteOnLoginTicketStatusUpdatedDelegate;

/**
 * AccelByte service implementation of the online identity interface
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineIdentityAccelByte : public IOnlineIdentity, public TSharedFromThis<FOnlineIdentityAccelByte, ESPMode::ThreadSafe>
{
public:
	/**
	 * Construct an instance of our identity interface
	 *
	 * @param InSubsystem Subsystem that owns this identity interface
	 */
	FOnlineIdentityAccelByte(FOnlineSubsystemAccelByte* InSubsystem);
	virtual ~FOnlineIdentityAccelByte() override = default;

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	//~ Begin IOnlineIdentity Interface
	virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentialsAccelByte& AccountCredentials);
	virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials) override;
	virtual bool Logout(int32 LocalUserNum) override;
	virtual bool AutoLogin(int32 LocalUserNum) override;
	virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId& UserId) const override;
	virtual TArray<TSharedPtr<FUserOnlineAccount> > GetAllUserAccounts() const override;
	virtual FUniqueNetIdPtr GetUniquePlayerId(int32 LocalUserNum) const override;
	virtual FUniqueNetIdPtr CreateUniquePlayerId(uint8* Bytes, int32 Size) override;
	virtual FUniqueNetIdPtr CreateUniquePlayerId(const FString& Str) override;
	virtual ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const override;
	virtual ELoginStatus::Type GetLoginStatus(const FUniqueNetId& UserId) const override;
	virtual FString GetPlayerNickname(int32 LocalUserNum) const override;
	virtual FString GetPlayerNickname(const FUniqueNetId& UserId) const override;
	virtual FString GetAuthToken(int32 LocalUserNum) const override;
	virtual void RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate) override;
	virtual void GetLinkedAccountAuthToken(int32 LocalUserNum, const FOnGetLinkedAccountAuthTokenCompleteDelegate& Delegate) const override;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
	virtual void GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate, EShowPrivilegeResolveUI ShowResolveUI = EShowPrivilegeResolveUI::Default) override;
#else
	virtual void GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate) override;
#endif
	virtual FPlatformUserId GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const override;
	virtual FString GetAuthType() const override;
#if ENGINE_MAJOR_VERSION > 4
	virtual int32 GetLocalUserNumFromPlatformUserId(FPlatformUserId PlatformUserId) const override;
#endif
	//~ End IOnlineIdentity Interface

	/** Extra method to try and get a LocalUserNum from a FUniqueNetId instance */
	bool GetLocalUserNum(const FUniqueNetId& NetId, int32& OutLocalUserNum) const;
	bool GetLocalUserNumFromPlatformUserId(const FString& PlatformUserId, int32& OutLocalUserNum);

	bool Logout(int32 LocalUserNum, FString Reason);

	/**
	 * Get the FApiClient that is used for a particular user by their net ID.
	 *
	 * Used to make raw SDK calls for user if needed.
	 */
	AccelByte::FApiClientPtr GetApiClient(const FUniqueNetId& UserId);

	/**
	 * Get the FApiClient that is used for a particular user by their net ID.
	 *
	 * Used to make raw SDK calls for user if needed.
	 */
	AccelByte::FApiClientPtr GetApiClient(int32 LocalUserNum);

	/**
	 * Authenticate a server on the backend using the FRegistry global API client
	 * 
	 * #TODO (Maxwell): This really should be supported by AutoLogin as a separate authentication path, as there's a ton
	 * of hacky code we need to write to make sure that any server call is working when authenticated. If we use AutoLogin
	 * for server auth, we bypass a lot of that code. However, this would require a rework to the current server code that
	 * we don't have time for... For now, use this method for all server auth...
	 */
	bool AuthenticateAccelByteServer(const FOnAuthenticateServerComplete& Delegate, int32 LocalUserNum = 0);

	/**
	 * Check whether or not our server is authenticated or not
	 */
	bool IsServerAuthenticated(int32 LocalUserNum = 0);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnConnectLobbyComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*Error*/);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnLoginWithOAuthErrorComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FErrorOAuthInfo& /*Error Object*/);

	/**
	 * Called when user account login has completed after calling Login() or AutoLogin()
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param bWasSuccessful true if server was contacted and a valid result received
	 * @param UserId the user id received from the server on successful login
	 * @param Error Information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnLoginComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FOnlineErrorAccelByte& /*Error*/);
	
	/**
	 * Called when user simultaneous login has completed after calling Login()
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param bWasSuccessful true if server was contacted and a valid result received
	 * @param ESimultaneousLoginResult the result from the login attempt, or the last known state
	 * @param UserId the user id received from the server on successful login
	 * @param Error Information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_FOUR_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnSimultaneousLoginComplete, bool /*bWasSuccessful*/, ESimultaneousLoginResult /*Result*/, const FUniqueNetId& /*UserId*/, const FErrorOAuthInfo& /*Error*/);

	/**
	 * Delegate used in notifying the that manual logout completed
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param bWasSuccessful whether the async call completed properly or not
	 * @param Error Information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnLogoutComplete, bool /*bWasSuccessful*/, const FOnlineErrorAccelByte& /*Error*/);
 
	/**
	 * Called when user account login succesfully connected to lobby
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param bWasSuccessful true if server was contacted and a valid result received
	 * @param UserId the user id received from the server on successful connect
	 * @param Error Information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnConnectLobbyComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FOnlineErrorAccelByte& /*Error*/);

	/**
	 * Called when lobby connection closed
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param UserId the user id received from the server on successful connect
	 * @param StatusCode Current status response code from lobby closed connection
	 * @param Reason The reason of lobby connection closed
	 * @param bWasClean Whether the connection closed with a clean status or not
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_FOUR_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnLobbyConnectionClosed, const FUniqueNetId& /*UserId*/, int32 /*StatusCode*/, const FString& /*Reason*/, bool /*bWasClean*/);

	/**
	 * Called when lobby try to reconnect
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param UserId the user id received from the server on successful connect
	 * @param StatusCode Current status response code from lobby closed connection
	 * @param Reason The reason of lobby reconnecting
	 * @param bWasClean Whether the connection reconnect with a clean status or not
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_FOUR_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnLobbyReconnecting, const FUniqueNetId& /*UserId*/, int32 /*StatusCode*/, const FString& /*Reason*/, bool /*bWasClean*/);

	/**
	 * Called when lobby connected after try to reconnect
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE(MAX_LOCAL_PLAYERS, AccelByteOnLobbyReconnected);
	
	/**
	 * Triggered when the platform token refresh is already responded
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param bWasSuccessful true if server was contacted and a valid result received
	 * @param FPlatformTokenRefreshResponse the refresh result & information
	 * @param PlatformName The platform that was refreshed
	 * @param Error Information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_FOUR_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnPlatformTokenRefreshedComplete, bool /*bWasSuccessful*/, const FPlatformTokenRefreshResponse& /*RefreshResponse*/, const FName& /*PlatformName*/, const FErrorOAuthInfo& /*Error*/);
	
	/**
	 * Triggered when the login process is queued
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param TicketInfo Information about the queue ticket
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_ONE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnLoginQueued, const FAccelByteModelsLoginQueueTicketInfo& /*TicketInfo*/);

	/**
	 * Triggered when login queue cancel operation is completed
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param bWasSuccessful true if server was contacted and a valid result received
	 * @param Error Information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnLoginQueueCancelComplete, bool /*bWasSuccessful*/, const FOnlineErrorAccelByte& /*Error*/);
	
	/**
	 * Triggered when the login process is queued
	 *
	 * @param LocalUserNum the controller number of the associated user
	 * @param bWasSuccessful true if server was contacted and a valid result received
	 * @param TicketInfo Information about the queue ticket
	 * @param Error Information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnLoginTicketStatusUpdated, bool /*bWasSuccessful*/, const FAccelByteModelsLoginQueueTicketInfo& /*TicketInfo*/, const FOnlineErrorAccelByte& /*Error*/)
	
	bool ConnectAccelByteLobby(int32 LocalUserNum);

	/**
	 * Set whether to create headless account when try to do 3rd party login using AutoLogin. The default value is true.
	 */
	void SetAutoLoginCreateHeadless(bool bInIsAutoLoginCreateHeadless);

	/**
	* To decide whether the current user should be logged out or not.
	* Logout required for specific range of codes.
	* Which means connection closure is abnormal and client should not reconnect (i.e. ban & access token revocation)
	*/
	static bool IsLogoutRequired(int32 WsClosedConnectionStatusCode);

	/**
	 * @brief This function generate ab code that can be exchanged into publisher namespace token (i.e. by web portal)
	 *
	 * @param PublisherClientID The targeted game's publisher ClientID.
	 */
	void GenerateCodeForPublisherToken(int32 LocalUserNum, const FString& PublisherClientId, FGenerateCodeForPublisherTokenComplete Delegate);
	
	/**
	 * @brief Refresh the native platform token that is stored in the backend. Therefore we can prevent expiration on the backend.
	 * Default behavior: this function already registered automatically and follow the schedule of refresh token expiration.
	 * To disable this default: 
	 *     - Modify DefaultEngine.ini
	 *     - Section [OnlineSubsystemAccelByte]
	 *     - Set value bNativePlatformTokenRefreshManually=true
	 *     - Do not forget to manually call this function periodically
	 */
	bool RefreshPlatformToken(int32 LocalUserNum);

	/**
	 * @brief Refresh the native platform token that is stored in the backend. Therefore we can prevent expiration on the backend.
	 * 
	 * @param SubsystemName The name of the subsystem authorization ticket that will be refreshed
	 */
	bool RefreshPlatformToken(int32 LocalUserNum, FName SubsystemName);

	bool CancelLoginQueue(int32 LocalUserNum);

PACKAGE_SCOPE:
	/**
	 * Used by the login async task to move data for the newly authenticated user to this identity instance.
	 */
	void AddNewAuthenticatedUser(int32 LocalUserNum, const TSharedRef<const FUniqueNetId>& UserId, const TSharedRef<FUserOnlineAccount>& Account);
	
	/** Set login status. */
	void SetLoginStatus(int32 LocalUserNum, ELoginStatus::Type NewStatus);

	/**
	 * Add a new authenticated server to the identity interface mappings for V2 session flow
	 */
	void AddAuthenticatedServer(int32 LocalUserNum);

	/**
	 * Used by login queue cancel to notify poller when user cancelled login queue
	 * @param LoginUserNum the user num we cancelled the login
	 */
	void FinalizeLoginQueueCancel(int32 LoginUserNum);

	/**
	 * Used to store ticket ID according to it's user num.
	 * @param LoginUserNum UserNum trying to login
	 * @param TicketId Login queue ticket ID
	 */
	void InitializeLoginQueue(int32 LoginUserNum, const FString& TicketId);

	/**
	 * Used to clear ticket ID according to it's user num.
	 * @param LoginUserNum UserNum trying to login
	 */
	void FinalizeLoginQueue(int32 LoginUserNum);

	/**
	 * @brief Retrieve User Account information from given Local User.
	 * 
	 * @param LocalUserNum the controller number of the associated user.
	 * 
	 * @return SharedPtr of User Account data.
	 */
	virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(int LocalUserNum) const;

	/**
	 * @brief Retrieve User Account information from given Local User.
	 *
	 * @param LocalUserId SharedRef of given Local User.
	 *
	 * @return SharedPtr of User Account data.
	 */
	virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetIdRef& LocalUserId) const;

	/**
	 * @brief Retrieve User Account information from given Local User.
	 *
	 * @param LocalUserId SharedPtr of given Local User.
	 *
	 * @return SharedPtr of User Account data.
	 */
	virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetIdPtr& LocalUserId) const;

	/**
	 * Online delegate to notify poller when a user cancelled login queue
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(AccelByteOnLoginQueueCanceledByUser, int32 /*LoginUserNum*/);

private:
	/** Disable the default constructor, as we only want to be able to construct this interface passing in the parent subsystem */
	FOnlineIdentityAccelByte() = delete;

	/** Parent subsystem that spawned this instance */
	TWeakPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> AccelByteSubsystem;

	/** Simple mapping for LocalUserNum to FUniqueNetIdAccelByte for users. Filled when users log in. */
	TMap<int32, FUniqueNetIdRef> LocalUserNumToNetIdMap;

	/** Mapping of local user indices to login statuses, used to query login status for a user */
	TMap<int32, ELoginStatus::Type> LocalUserNumToLoginStatusMap;

	/** Simple mapping for FUniqueNetIdAccelByte to LocalUserNum. Filled when users log in. */
	TUniqueNetIdMap<int32> NetIdToLocalUserNumMap;
	
	/** Mapping of AccelByte net IDs to AccelByte user accounts. Filled when users log in. */
	TUniqueNetIdMap<TSharedRef<FUserOnlineAccount>> NetIdToOnlineAccountMap;
	
	mutable FCriticalSection LocalUserNumToLoginQueueTicketLock;
	
	/** Mapping of local user indices to it's login queue ticket */
	TMap<int32, FString> LocalUserNumToLoginQueueTicketMap;

	FString LogoutReason; // Error Code for when we logged out

	/**
	 * Call when the user logged out.
	 * 
	 * @param LocalUserNum Index of the user who logged out
	 * @param bWasSuccessful True if the user is successfully logged out or False otherwise.
	 */
	void OnLogout(const int32 LocalUserNum, bool bWasSuccessful);

	/**
	 * Remove all mappings for a user by their user index. Should only be called when the user logs out.
	 * 
	 * @param LocalUserNum Index of the user that we wish to remove mappings for
	 */
	void RemoveUserFromMappings(const int32 LocalUserNum);

	/**
	 * Handler for when our internal call to the native platform for querying user privileges completes.
	 */
	void OnGetNativeUserPrivilegeComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, FOnGetUserPrivilegeCompleteDelegate OriginalDelegate, TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId);

	/**
	 * Flag denoting whether or not we are currently authenticating as a server to prevent duplicate logins
	 */
	bool bIsAuthenticatingServer = false;

	/**
	 * Flag denoting whether or not we have successfully authenticated as a server in this instance
	 */
	bool bIsServerAuthenticated = false;

	/**
	 * Flag denoting whether or not we create headless account on successfully 3rd party login using AutoLogin
	 */
	bool bIsAutoLoginCreateHeadless = true;

	/**
	 * Array of pending server login delegates, will be all fired once login is finished
	 */
	TArray<FOnAuthenticateServerComplete> ServerAuthDelegates;

	/**
	 * Array of booleans reflecting whether a local player at the corresponding index is currently logging in.
	 * Prevents double log in if already in progress.
	 */
#if ENGINE_MAJOR_VERSION >= 5
	TStaticArray<bool, 4> LocalPlayerLoggingIn{InPlace, false};
#else
	TStaticArray<bool, 4> LocalPlayerLoggingIn{false};
#endif

	/**
	 * Handler for when we finish authenticating a server to fire off delegates
	 */
	void OnAuthenticateAccelByteServerSuccess(int32 LocalUserNum = 0);

	/**
	 * Handler for when we fail to authenticate a server to fire off delegates
	 */
	void OnAuthenticateAccelByteServerError(int32 ErrorCode, const FString& ErrorMessage);

};
