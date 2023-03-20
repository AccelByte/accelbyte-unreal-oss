// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
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

class FOnlineAccountCredentialsAccelByte : public FOnlineAccountCredentials
{
public:
	EAccelByteLoginType LoginType;
	
	FOnlineAccountCredentialsAccelByte(EAccelByteLoginType InType
		, const FString& InId
		, const FString& InToken)
		: FOnlineAccountCredentials(FAccelByteUtilities::GetUEnumValueAsString(InType), InId, InToken)
		, LoginType{InType}
	{
		
	}
};

typedef FOnlineAccountCredentialsAccelByte FOnlineAccelByteAccountCredentials;

DECLARE_DELEGATE_OneParam(FOnAuthenticateServerComplete, bool /*bWasSuccessful*/)
DECLARE_MULTICAST_DELEGATE_FourParams(FOnConnectLobbyComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*Error*/);
typedef FOnConnectLobbyComplete::FDelegate FOnConnectLobbyCompleteDelegate;

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
	virtual bool Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials) override;
	virtual bool Logout(int32 LocalUserNum) override;
	virtual bool AutoLogin(int32 LocalUserNum) override;
	virtual TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId& UserId) const override;
	virtual TArray<TSharedPtr<FUserOnlineAccount> > GetAllUserAccounts() const override;
	virtual TSharedPtr<const FUniqueNetId> GetUniquePlayerId(int32 LocalUserNum) const override;
	virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(uint8* Bytes, int32 Size) override;
	virtual TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(const FString& Str) override;
	virtual ELoginStatus::Type GetLoginStatus(int32 LocalUserNum) const override;
	virtual ELoginStatus::Type GetLoginStatus(const FUniqueNetId& UserId) const override;
	virtual FString GetPlayerNickname(int32 LocalUserNum) const override;
	virtual FString GetPlayerNickname(const FUniqueNetId& UserId) const override;
	virtual FString GetAuthToken(int32 LocalUserNum) const override;
	virtual void RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate) override;
	virtual void GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate) override;
	virtual FPlatformUserId GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const override;
	virtual FString GetAuthType() const override;
	//~ End IOnlineIdentity Interface

	/** Extra method to try and get a LocalUserNum from a FUniqueNetId instance */
	bool GetLocalUserNum(const FUniqueNetId& NetId, int32& OutLocalUserNum) const;

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

	bool ConnectAccelByteLobby(int32 LocalUserNum);

	/**
	* To decide whether the current user should be logged out or not.
	* Logout required for specific range of codes.
	* Which means connection closure is abnormal and client should not reconnect (i.e. ban & access token revocation)
	*/
	static bool IsLogoutRequired(int32 WsClosedConnectionStatusCode);

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

private:
	/** Disable the default constructor, as we only want to be able to construct this interface passing in the parent subsystem */
	FOnlineIdentityAccelByte() = delete;

	/** Parent subsystem that spawned this instance */
	FOnlineSubsystemAccelByte* AccelByteSubsystem;

	/** Simple mapping for LocalUserNum to FUniqueNetIdAccelByte for users. Filled when users log in. */
	TMap<int32, TSharedRef<const FUniqueNetId>> LocalUserNumToNetIdMap;

	/** Mapping of local user indices to login statuses, used to query login status for a user */
	TMap<int32, ELoginStatus::Type> LocalUserNumToLoginStatusMap;

	/** Simple mapping for FUniqueNetIdAccelByte to LocalUserNum. Filled when users log in. */
	TUniqueNetIdMap<int32> NetIdToLocalUserNumMap;

	/** Mapping of AccelByte net IDs to AccelByte user accounts. Filled when users log in. */
	TUniqueNetIdMap<TSharedRef<FUserOnlineAccount>> NetIdToOnlineAccountMap;
	
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
	 * Array of pending server login delegates, will be all fired once login is finished
	 */
	TArray<FOnAuthenticateServerComplete> ServerAuthDelegates;

	/**
	 * Handler for when we finish authenticating a server to fire off delegates
	 */
	void OnAuthenticateAccelByteServerSuccess(int32 LocalUserNum = 0);

	/**
	 * Handler for when we fail to authenticate a server to fire off delegates
	 */
	void OnAuthenticateAccelByteServerError(int32 ErrorCode, const FString& ErrorMessage);

};
