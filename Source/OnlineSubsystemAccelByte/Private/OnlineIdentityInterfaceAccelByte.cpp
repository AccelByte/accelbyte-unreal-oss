// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineIdentityInterfaceAccelByte.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "OnlineSubsystemAccelByte.h"
#include "OnlineError.h"
#include "Core/AccelByteRegistry.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Core/AccelByteError.h"
#include "Core/AccelByteWebSocketErrorTypes.h"
#include "Core/AccelByteCredentials.h"
#include "Api/AccelByteUserProfileApi.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLogin.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteConnectLobby.h"
#include "Misc/Base64.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "GameServerApi/AccelByteServerOauth2Api.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteLoginServer.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteConnectChat.h"

/** Begin FOnlineIdentityAccelByte */
FOnlineIdentityAccelByte::FOnlineIdentityAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
{
	// this should never trigger, as the subsystem itself has to instantiate this, but just in case...
	check(AccelByteSubsystem != nullptr);
}

bool FOnlineIdentityAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineIdentityAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineIdentityAccelByte::GetFromWorld(const UWorld* World, FOnlineIdentityAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineIdentityAccelByte::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; Type: %s; Id: %s"), LocalUserNum, *AccountCredentials.Type, *AccountCredentials.Id);

	// @todo multiuser Remove this check once the SDK supports more than one player
	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		const FString ErrorStr = TEXT("login-failed-invalid-user-index");
		AB_OSS_INTERFACE_TRACE_END(TEXT("Invalid LocalUserNum specified. LocalUserNum=%d; MaxLocalPlayers=%d"), LocalUserNum, MAX_LOCAL_PLAYERS);
		
		TriggerOnLoginChangedDelegates(LocalUserNum);
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
		TriggerOnLoginStatusChangedDelegates(LocalUserNum, ELoginStatus::NotLoggedIn, ELoginStatus::NotLoggedIn, FUniqueNetIdAccelByteUser::Invalid().Get());
		return false;
	}

	// Don't attempt to authenticate again if we are already reporting as logged in
	if (GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn
		&& LocalUserNumToNetIdMap.Contains(LocalUserNum)
		&& (AccountCredentials.Type != FAccelByteUtilities::GetUEnumValueAsString(EAccelByteLoginType::RefreshToken)))
	{
		const TSharedPtr<const FUniqueNetId> UserIdPtr = GetUniquePlayerId(LocalUserNum);
		TSharedPtr<FUserOnlineAccount> UserAccount;
		if (UserIdPtr.IsValid())
		{
			const FUniqueNetId& UserId = UserIdPtr.ToSharedRef().Get();
			UserAccount = GetUserAccount(UserId);
		}

		const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
		const FString ErrorStr = TEXT("login-failed-already-logged-in");
		AB_OSS_INTERFACE_TRACE_END(TEXT("User already logged in at user index '%d'!"), LocalUserNum);

		TriggerOnLoginChangedDelegates(LocalUserNum);
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
		TriggerOnLoginStatusChangedDelegates(LocalUserNum, ELoginStatus::NotLoggedIn, ELoginStatus::NotLoggedIn, FUniqueNetIdAccelByteUser::Invalid().Get());
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLogin>(AccelByteSubsystem, LocalUserNum, AccountCredentials);
	AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to login!"));
	return true;
}

bool FOnlineIdentityAccelByte::Logout(int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	Logout(LocalUserNum, FString());

	return true;
}

bool FOnlineIdentityAccelByte::Logout(int32 LocalUserNum, FString Reason)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	LogoutReason = Reason;

	FVoidHandler OnLogoutSuccessDelegate = FVoidHandler::CreateThreadSafeSP(AsShared(), &FOnlineIdentityAccelByte::OnLogout, LocalUserNum, true);
	FErrorHandler OnLogoutFailedDelegate = FErrorHandler::CreateLambda([IdentityInterface = AsShared(), LocalUserNum](int32 ErrorCode, const FString& ErrorMessage) {
		UE_LOG_AB(Error, TEXT("Logging out with AccelByte OSS failed! Code: %d; Message: %s"), ErrorCode, *ErrorMessage);
		IdentityInterface->OnLogout(LocalUserNum, false);
	});

	if (!IsRunningDedicatedServer())
	{
		AccelByte::FApiClientPtr ApiClient = GetApiClient(LocalUserNum);
		if (!ApiClient.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as an API client could not be found for them!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}

		if (!ApiClient->CredentialsRef->IsSessionValid())
		{
			UE_LOG_AB(Warning, TEXT("Force log out user %d as the login session is not valid!"), LocalUserNum);
			OnLogout(LocalUserNum, true);
			return false;
		}

		// @todo multiuser Change this to logout the user based on LocalUserNum when SDK supports multiple user login
		// NOTE(Maxwell, 4/8/2021): Logout automatically signals to the credential store to forget all credentials, so this is all we need to call
		if (ApiClient->Lobby.IsConnected())
		{
			ApiClient->Lobby.Disconnect(true);
		}
		ApiClient->User.Logout(OnLogoutSuccessDelegate, OnLogoutFailedDelegate);
	}
	else
	{
		AccelByte::FServerApiClientPtr ApiClient = FMultiRegistry::GetServerApiClient();
		if (!ApiClient.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as an API client could not be found for them!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}

		ApiClient->ServerCredentialsRef->Shutdown();
		ApiClient->ServerCredentialsRef->ForgetAll();
		FMultiRegistry::RemoveApiClient();
		OnLogout(LocalUserNum, true);
	}

	return true;
}

bool FOnlineIdentityAccelByte::AutoLogin(int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if (IsRunningDedicatedServer())
	{
#if AB_USE_V2_SESSIONS
		// Servers have a custom authentication flow where we just associate them with user index zero. Async task code for
		// servers should not access the identity interface, this is basically just implemented to fit the flow of a dedicated
		// server on Unreal closely.
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginServer>(AccelByteSubsystem, LocalUserNum);
		return true;
#else
		// #NOTE For compatibility reasons, V1 sessions still won't support auto login, since those tasks already have a way
		// to authenticate a server.
		AB_OSS_INTERFACE_TRACE_END(TEXT("AutoLogin does not work with AccelByte servers, server login is done automatically during registration."));
		return false;
#endif
	}

	FOnlineAccountCredentials Credentials;

	// Try and parse the command line for the authentication type, as well as its ID and token
	FParse::Value(FCommandLine::Get(), TEXT("-AUTH_TYPE="), Credentials.Type);
	FParse::Value(FCommandLine::Get(), TEXT("-AUTH_LOGIN="), Credentials.Id);
	FParse::Value(FCommandLine::Get(), TEXT("-AUTH_PASSWORD="), Credentials.Token);

	// Check if we have an environment variable set named "JUSTICE_AUTHORIZATION_CODE", which if so will initiate a
	// launcher login and reset the type to be a "launcher" login
	const FString LauncherCode = FAccelByteUtilities::GetAuthorizationCode();

	if (!LauncherCode.IsEmpty())
	{
		Credentials.Type = "launcher";
	}

	// If we don't have a username and password in the command line as well as no launcher token, we still want to
	// attempt auto login using a blank credential, which may end up using a native OSS pass through if that is set up.
	bool bLoginTaskSpawned = Login(LocalUserNum, Credentials);
	
	AB_OSS_INTERFACE_TRACE_END(TEXT("Login has been called from AutoLogin. Async task spawned: "), LOG_BOOL_FORMAT(bLoginTaskSpawned));
	return bLoginTaskSpawned;
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityAccelByte::GetUserAccount(const FUniqueNetId& UserId) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	const TSharedRef<FUserOnlineAccount>* Account = NetIdToOnlineAccountMap.Find(UserId.AsShared());
	if (Account != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("FOnlineUserAccount found for user with NetID of '%s'!"), *UserId.ToDebugString());

		return *Account;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("FOnlineUserAccount not found for user with NetID of '%s'! Returning nullptr."), *UserId.ToDebugString());
	return nullptr;
}

TArray<TSharedPtr<FUserOnlineAccount>> FOnlineIdentityAccelByte::GetAllUserAccounts() const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("N/A"));

	TArray<TSharedPtr<FUserOnlineAccount>> Result;
	Result.Empty(NetIdToOnlineAccountMap.Num());

	for (const auto& Entry : NetIdToOnlineAccountMap)
	{
		Result.Add(Entry.Value);
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Returning TArray of accounts with %d entries."), Result.Num());
	return Result;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityAccelByte::GetUniquePlayerId(int32 LocalUserNum) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("LocalUserNum: %d"), LocalUserNum);

	const TSharedRef<const FUniqueNetId>* UserId = LocalUserNumToNetIdMap.Find(LocalUserNum);
	if (UserId != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Found NetId for LocalUserNum of '%d'. ID is '%s'."), LocalUserNum, *(*UserId)->ToDebugString());

		return *UserId;
	}

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Could not find NetId for LocalUserNum of '%d'. Returning nullptr."), LocalUserNum);
	return nullptr;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityAccelByte::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	if (Bytes && Size > 0)
	{
		FString StrId(Size, (TCHAR*)Bytes);
		return CreateUniquePlayerId(StrId);
	}

	return nullptr;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityAccelByte::CreateUniquePlayerId(const FString& Str)
{
	return FUniqueNetIdAccelByteUser::Create(Str);
}

ELoginStatus::Type FOnlineIdentityAccelByte::GetLoginStatus(int32 LocalUserNum) const
{
	// #SG #NOTE (Maxwell): Changing all verbosity to VeryVerbose to prevent log spam with controllers
	AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("LocalUserNum: %d"), LocalUserNum);

	const ELoginStatus::Type* FoundLoginStatus = LocalUserNumToLoginStatusMap.Find(LocalUserNum);
	if (FoundLoginStatus != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("LoginStatus: %s"), ELoginStatus::ToString(*FoundLoginStatus));
		return *FoundLoginStatus;
	}

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Could not find login status stored for user at index '%d'! Returning NotLoggedIn."), LocalUserNum);
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityAccelByte::GetLoginStatus(const FUniqueNetId& UserId) const
{
	// #SG #NOTE (Maxwell): Changing all verbosity to VeryVerbose to prevent log spam with controllers
	AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("UserId: %s"), *UserId.ToDebugString());

	int32 LocalUserNum;
	if (!GetLocalUserNum(UserId, LocalUserNum))
	{
		// #NOTE (Maxwell): Technically this should be a warning, but since this is used to check controller status we'll just want to very verbose it.
		// In the end, we will probably want to change the controller connection code to use the built-in delegates.
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Could not get login status for user '%s' as their local user index could not be found!"), *UserId.ToDebugString());
		return ELoginStatus::NotLoggedIn;
	}

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT(""));
	return GetLoginStatus(LocalUserNum);
}

void FOnlineIdentityAccelByte::SetLoginStatus(const int32 LocalUserNum, const ELoginStatus::Type NewStatus)
{
	const ELoginStatus::Type OldStatus = GetLoginStatus(LocalUserNum);
	LocalUserNumToLoginStatusMap.Add(LocalUserNum, NewStatus);

	const TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (!UserId.IsValid())
	{
		return;
	}

	TriggerOnLoginStatusChangedDelegates(LocalUserNum, OldStatus, NewStatus, (UserId.IsValid() ? UserId.ToSharedRef().Get() : FUniqueNetIdAccelByteUser::Invalid().Get()));
}

void FOnlineIdentityAccelByte::AddAuthenticatedServer(int32 LocalUserNum)
{
	SetLoginStatus(LocalUserNum, ELoginStatus::LoggedIn);

	// #NOTE Not adding net IDs here as servers have no user IDs, keep that in mind when making server calls
}

FString FOnlineIdentityAccelByte::GetPlayerNickname(int32 LocalUserNum) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	const TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId found for User %d, getting nickname from UserId."), LocalUserNum);
		return GetPlayerNickname(*UserId);
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Nickname could not be queried for user %d. Returning 'NullUser'."), LocalUserNum);
	return TEXT("NullUser");
}

FString FOnlineIdentityAccelByte::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	const TSharedPtr<FUserOnlineAccountAccelByte> AccelByteAccount = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(GetUserAccount(UserId));
	if (AccelByteAccount.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Nickname found. User nickname is '%s'"), *AccelByteAccount->GetDisplayName());
		return AccelByteAccount->GetDisplayName();
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("FUserOnlineAccountAccelByte not found for user with NetID of '%s'!. Returning 'NullUser'."), *UserId.ToDebugString());
	return TEXT("NullUser");
}

FString FOnlineIdentityAccelByte::GetAuthToken(int32 LocalUserNum) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	const TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		const TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(UserId.ToSharedRef().Get());
		if (UserAccount.IsValid())
		{
			AB_OSS_INTERFACE_TRACE_END(TEXT("Found access token for User '%d'"), LocalUserNum);
			return UserAccount->GetAccessToken();
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Could not find access token for User '%d'"), LocalUserNum);
	return FString();
}

void FOnlineIdentityAccelByte::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	UE_LOG_ONLINE_IDENTITY(Display, TEXT("FOnlineIdentityAccelByte::RevokeAuthToken not implemented"));
	TSharedRef<const FUniqueNetId> UserIdRef(UserId.AsShared());
	AccelByteSubsystem->ExecuteNextTick([UserIdRef, Delegate]()
	{
		Delegate.ExecuteIfBound(*UserIdRef, FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))));
	});
}

void FOnlineIdentityAccelByte::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteCompositeId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	if (!AccelByteCompositeId->IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as their unique ID was invalid."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::UserNotFound));
		return;
	}

	if (!AccelByteCompositeId->HasPlatformInformation())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as they do not have native platform information."));
		// #NOTE (Maxwell): Just for testing with non-Steam accounts. Remove me!
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::NoFailures));
		return;
	}

	TSharedPtr<const FUniqueNetId> NativeUserId = AccelByteCompositeId->GetPlatformUniqueId();
	if (!NativeUserId.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as we could not get a native platform ID for them."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::UserNotFound));
		return;
	}

	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as there is not a native platform OSS loaded."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::GenericFailure));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = NativeSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as the identity interface for native OSS is invalid."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::GenericFailure));
		return;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Sending request to native OSS to get privileges for native user '%s'!"), *NativeUserId->ToDebugString());

	// Create an internal delegate so that we can pass back our AccelByte user ID instead of just the platform ID
	const FOnGetUserPrivilegeCompleteDelegate OnGetNativeUserPrivilegeCompleteDelegate = FOnGetUserPrivilegeCompleteDelegate::CreateThreadSafeSP(AsShared(), &FOnlineIdentityAccelByte::OnGetNativeUserPrivilegeComplete, Delegate, AccelByteCompositeId);
	return IdentityInterface->GetUserPrivilege(NativeUserId.ToSharedRef().Get(), Privilege, OnGetNativeUserPrivilegeCompleteDelegate);
}

FPlatformUserId FOnlineIdentityAccelByte::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("NetId: %s"), *UniqueNetId.ToDebugString());

	// NOTE(Maxwell, 4/7/2021): This will return what is essentially the LocalUserNum, which we retrieve by iterating
	// through all entries of the map and returning the key that has its associated value set to the UniqueId passed in.
	for (const auto& Entry : LocalUserNumToNetIdMap)
	{
		if (Entry.Value.Get() == UniqueNetId)
		{
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserNum found: %d"), Entry.Key);
#if ENGINE_MAJOR_VERSION >= 5
			return FPlatformMisc::GetPlatformUserForUserIndex(Entry.Key);
#else
			return Entry.Key;
#endif
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("UserNum not found for NetId '%s'"), *UniqueNetId.ToDebugString());
	return PLATFORMUSERID_NONE;
}

FString FOnlineIdentityAccelByte::GetAuthType() const
{
	return TEXT("AccelByte");
}

bool FOnlineIdentityAccelByte::GetLocalUserNum(const FUniqueNetId& NetId, int32& OutLocalUserNum) const
{
	const int32* FoundLocalUserNum = NetIdToLocalUserNumMap.Find(NetId.AsShared());
	if (FoundLocalUserNum != nullptr)
	{
		OutLocalUserNum = *FoundLocalUserNum;
		return true;
	}
	return false;
}

AccelByte::FApiClientPtr FOnlineIdentityAccelByte::GetApiClient(const FUniqueNetId& NetId)
{
	return AccelByteSubsystem->GetApiClient(NetId);
}

AccelByte::FApiClientPtr FOnlineIdentityAccelByte::GetApiClient(int32 LocalUserNum)
{
	return AccelByteSubsystem->GetApiClient(LocalUserNum);
}

bool FOnlineIdentityAccelByte::AuthenticateAccelByteServer(const FOnAuthenticateServerComplete& Delegate, int32 LocalUserNum)
{
#if AB_USE_V2_SESSIONS
	UE_LOG_AB(Warning, TEXT("FOnlineIdentityAccelByte::AuthenticateAccelByteServer is deprecated with V2 sessions. Servers should be authenticated through AutoLogin!"));
	AccelByteSubsystem->ExecuteNextTick([Delegate]() {
		Delegate.ExecuteIfBound(false);
	});
	return false;
#endif

#if UE_SERVER || UE_EDITOR
	if (!bIsServerAuthenticated && !bIsAuthenticatingServer)
	{
		bIsAuthenticatingServer = true;
		ServerAuthDelegates.Add(Delegate);

		const TSharedRef<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = SharedThis(this);
		const FVoidHandler OnLoginSuccess = FVoidHandler::CreateThreadSafeSP(IdentityInterface, &FOnlineIdentityAccelByte::OnAuthenticateAccelByteServerSuccess, LocalUserNum);
		const FErrorHandler OnLoginError = FErrorHandler::CreateThreadSafeSP(IdentityInterface, &FOnlineIdentityAccelByte::OnAuthenticateAccelByteServerError);
		FRegistry::ServerOauth2.LoginWithClientCredentials(OnLoginSuccess, OnLoginError);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Server is being or has already authenticated, skipping call!"));
		
		if (bIsServerAuthenticated)
		{
			Delegate.ExecuteIfBound(true);
		}
		else if (bIsAuthenticatingServer)
		{
			ServerAuthDelegates.Add(Delegate);
		}

		return false;
	}

	return true;
#endif
	return false;
}

bool FOnlineIdentityAccelByte::IsServerAuthenticated(int32 LocalUserNum)
{
	return GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn;
}

bool FOnlineIdentityAccelByte::ConnectAccelByteLobby(int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	// Don't attempt to connect again if we are already reporting as connected
	if (GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn && LocalUserNumToNetIdMap.Contains(LocalUserNum))
	{
		const TSharedPtr<const FUniqueNetId> UserIdPtr = GetUniquePlayerId(LocalUserNum);
		TSharedPtr<FUserOnlineAccount> UserAccount;
		if (UserIdPtr.IsValid())
		{
			const FUniqueNetId& UserId = UserIdPtr.ToSharedRef().Get();
			UserAccount = GetUserAccount(UserId);
		}

		const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
		if (UserAccountAccelByte->IsConnectedToLobby())
		{
			const FString ErrorStr = TEXT("connect-failed-already-connected");
			AB_OSS_INTERFACE_TRACE_END(TEXT("User already connected to lobby at user index '%d'!"), LocalUserNum);

			TriggerOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
			return false;
		}
		
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystem, *GetUniquePlayerId(LocalUserNum).Get());
		AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to connect lobby!"));
		return true;
	}

	const FString ErrorStr = TEXT("connect-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);

	TriggerOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
	return false;
}

void FOnlineIdentityAccelByte::AddNewAuthenticatedUser(int32 LocalUserNum, const TSharedRef<const FUniqueNetId>& UserId, const TSharedRef<FUserOnlineAccount>& Account)
{
	// Add to mappings for the user
	LocalUserNumToNetIdMap.Add(LocalUserNum, UserId);
	LocalUserNumToLoginStatusMap.Add(LocalUserNum, ELoginStatus::NotLoggedIn);
	NetIdToLocalUserNumMap.Add(UserId, LocalUserNum);
	NetIdToOnlineAccountMap.Add(UserId, Account);
}

void FOnlineIdentityAccelByte::OnLogout(const int32 LocalUserNum, bool bWasSuccessful)
{
	TriggerOnLogoutCompleteDelegates(LocalUserNum, bWasSuccessful);
	SetLoginStatus(LocalUserNum, ELoginStatus::NotLoggedIn);

	if (bWasSuccessful)
	{
		UE_LOG_AB(Log, TEXT("Logging out with AccelByte OSS succeeded! LocalUserNum: %d"), LocalUserNum);
		RemoveUserFromMappings(LocalUserNum);
	}
}

void FOnlineIdentityAccelByte::RemoveUserFromMappings(const int32 LocalUserNum)
{
	const TSharedRef<const FUniqueNetId>* UniqueId = LocalUserNumToNetIdMap.Find(LocalUserNum);
	if (UniqueId != nullptr)
	{
		// Remove the account map first, and then remove the unique ID by local user num
		const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteUser = FUniqueNetIdAccelByteUser::CastChecked(*UniqueId);
		AccelByte::FMultiRegistry::RemoveApiClient(AccelByteUser->GetAccelByteId());
		NetIdToLocalUserNumMap.Remove(*UniqueId);
		NetIdToOnlineAccountMap.Remove(*UniqueId);
		LocalUserNumToNetIdMap.Remove(LocalUserNum);
	}
}

void FOnlineIdentityAccelByte::OnGetNativeUserPrivilegeComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, FOnGetUserPrivilegeCompleteDelegate OriginalDelegate, TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId)
{
	OriginalDelegate.ExecuteIfBound(AccelByteId.Get(), Privilege, PrivilegeResults);
}

void FOnlineIdentityAccelByte::OnAuthenticateAccelByteServerSuccess(int32 LocalUserNum)
{
	for (FOnAuthenticateServerComplete& Delegate : ServerAuthDelegates)
	{
		Delegate.ExecuteIfBound(true);
	}

	ServerAuthDelegates.Empty();
	AddAuthenticatedServer(LocalUserNum);

	// #TODO (Maxwell): This is some super nasty code to avoid duplicate server authentication, again this should just be apart of AutoLogin...
	bIsAuthenticatingServer = false;
	bIsServerAuthenticated = true;
}

void FOnlineIdentityAccelByte::OnAuthenticateAccelByteServerError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to authenticate server! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	
	for (FOnAuthenticateServerComplete& Delegate : ServerAuthDelegates)
	{
		Delegate.ExecuteIfBound(false);
	}

	ServerAuthDelegates.Empty();

	// Reset our authentication state to false as we can retry from here
	bIsAuthenticatingServer = false;
	bIsServerAuthenticated = false;
}

bool FOnlineIdentityAccelByte::IsLogoutRequired(int32 StatusCode)
{
	//Explanation of deprecation & new assigned numbers can be read in each enums between these numbers
	int LowestDeprecatedNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectSenderError);
	int HighestDeprecatedNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectReaderError);
	if (StatusCode >= LowestDeprecatedNumber && StatusCode <= HighestDeprecatedNumber)
	{
		return false;
	}

	int32 LowestNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectServerShutdown);
	int32 HighestNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::OutsideBoundaryOfDisconnection);
	return (StatusCode > LowestNumber && StatusCode < HighestNumber);
}

/** End FOnlineIdentityAccelByte */