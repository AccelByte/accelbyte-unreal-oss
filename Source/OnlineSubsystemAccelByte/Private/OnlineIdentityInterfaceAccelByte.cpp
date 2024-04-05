// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineIdentityInterfaceAccelByte.h"
#include "Runtime/Launch/Resources/Version.h"
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
#include "Core/AccelByteOauth2Api.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLogin.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteSimultaneousLogin.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteRefreshPlatformToken.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteConnectLobby.h"
#include "Misc/Base64.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "GameServerApi/AccelByteServerOauth2Api.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteLoginServer.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteConnectChat.h"
#include "Templates/SharedPointer.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteGenerateCodeForPublisherToken.h"
#include "AsyncTasks/LoginQueue/OnlineAsyncTaskAccelByteLoginQueueCancelTicket.h"

using namespace AccelByte;

/** Begin FOnlineIdentityAccelByte */
FOnlineIdentityAccelByte::FOnlineIdentityAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
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
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLogin"
bool FOnlineIdentityAccelByte::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	return Login(LocalUserNum, FOnlineAccountCredentialsAccelByte{ AccountCredentials });
};

bool FOnlineIdentityAccelByte::Login(int32 LocalUserNum, const FOnlineAccountCredentialsAccelByte& AccountCredentials)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; Type: %s; Id: %s"), LocalUserNum, *AccountCredentials.Type, *AccountCredentials.Id);

	// @todo multiuser Remove this check once the SDK supports more than one player
	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		const FString ErrorStr = TEXT("login-failed-invalid-user-index");
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Invalid LocalUserNum specified. LocalUserNum=%d; MaxLocalPlayers=%d"), LocalUserNum, MAX_LOCAL_PLAYERS);
		
		TriggerOnLoginChangedDelegates(LocalUserNum);
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
		FErrorOAuthInfo ErrorOAuthInfo { (int32)ErrorCodes::InvalidRequest, ErrorStr,
		FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest), ErrorStr };
		TriggerOnLoginWithOAuthErrorCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorOAuthInfo);
		TriggerAccelByteOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ONLINE_ERROR_ACCELBYTE(ErrorStr));
		return false;
	}

	if (LocalPlayerLoggingIn[LocalUserNum])
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning
			, TEXT("Player at index '%d' is already logging in! Ignoring subsequent call to log in.")
			, LocalUserNum);
		// Do not trigger delegates as it could impact handlers for previous log in attempt
		return false;
	}

	LocalPlayerLoggingIn[LocalUserNum] = true;

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
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User already logged in at user index '%d'!"), LocalUserNum);
		
		// Clear log in progress flag
		LocalPlayerLoggingIn[LocalUserNum] = false;

		TriggerOnLoginChangedDelegates(LocalUserNum);
		TriggerOnLoginCompleteDelegates(LocalUserNum, true, *UserIdPtr, ErrorStr); 
		FErrorOAuthInfo ErrorOAuthInfo { (int32)ErrorCodes::InvalidRequest, ErrorStr,
			FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest), ErrorStr };
		TriggerOnLoginWithOAuthErrorCompleteDelegates(LocalUserNum, false, *UserIdPtr, ErrorOAuthInfo);
		TriggerAccelByteOnLoginCompleteDelegates(LocalUserNum, false, *UserIdPtr, ONLINE_ERROR_ACCELBYTE(ErrorStr));
		return false;
	}

	FOnlineAsyncTaskInfo TaskInfo;
	TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
	TaskInfo.bCreateEpicForThis = true;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Clear log in progress flag
		LocalPlayerLoggingIn[LocalUserNum] = false;

		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte online subsystem is null"));
		const FString ErrorStr = TEXT("login-failed-online-subsystem-null");
		TriggerOnLoginChangedDelegates(LocalUserNum);
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
		return false;
	}
	else if (AccountCredentials.Type.Contains(FAccelByteUtilities::GetUEnumValueAsString(EAccelByteLoginType::Simultaneous)))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTask<FOnlineAsyncTaskAccelByteSimultaneousLogin>(TaskInfo
			, AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, AccountCredentials
			, AccountCredentials.bCreateHeadlessAccount);
	}
	else
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTask<FOnlineAsyncTaskAccelByteLogin>(TaskInfo
			, AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, AccountCredentials
			, AccountCredentials.bCreateHeadlessAccount);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to login!"));
	}
	return true;
}
#undef ONLINE_ERROR_NAMESPACE

bool FOnlineIdentityAccelByte::Logout(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	Logout(LocalUserNum, FString());

	return true;
}

bool FOnlineIdentityAccelByte::Logout(int32 LocalUserNum, FString Reason)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

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

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLogin"
bool FOnlineIdentityAccelByte::AutoLogin(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if (IsRunningDedicatedServer())
	{
#if AB_USE_V2_SESSIONS
		// Servers have a custom authentication flow where we just associate them with user index zero. Async task code for
		// servers should not access the identity interface, this is basically just implemented to fit the flow of a dedicated
		// server on Unreal closely.

		// Skip login server if server is already logged in.
		if(IsServerAuthenticated())
		{
			const FString ErrorStr = TEXT("login-failed-already-logged-in");
			TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
			TriggerAccelByteOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ONLINE_ERROR_ACCELBYTE(ErrorStr));

			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User already logged in at user index '%d'!"), LocalUserNum)
			return false;
		}
		
		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if (!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte online subsystem is null"));
			const FString ErrorStr = TEXT("login-failed-online-subsystem-null");
			TriggerOnLoginChangedDelegates(LocalUserNum);
			TriggerOnLoginCompleteDelegates(LocalUserNum, true, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
			return false;
		}
		else
		{
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginServer>(AccelByteSubsystemPtr.Get(), LocalUserNum);
		}
		return true;
#else
		// #NOTE For compatibility reasons, V1 sessions still won't support auto login, since those tasks already have a way
		// to authenticate a server.
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AutoLogin does not work with AccelByte servers, server login is done automatically during registration."));
		return false;
#endif
	}

	FOnlineAccountCredentialsAccelByte Credentials{ bIsAutoLoginCreateHeadless };

	// Check if we have an environment variable set named "JUSTICE_AUTHORIZATION_CODE", which if so will initiate a
	// launcher login and reset the type to be a "launcher" login
	const FString LauncherCode = FAccelByteUtilities::GetAuthorizationCode();

	if (!LauncherCode.IsEmpty())
	{
		Credentials.Type = "Launcher";
	}

	// If we don't have a username and password in the command line as well as no launcher token, we still want to
	// attempt auto login using a blank credential, which may end up using a native OSS pass through if that is set up.
	bool bLoginTaskSpawned = Login(LocalUserNum, Credentials);
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Login has been called from AutoLogin. Async task spawned: %s"), LOG_BOOL_FORMAT(bLoginTaskSpawned));
	return bLoginTaskSpawned;
}
#undef ONLINE_ERROR_NAMESPACE

TSharedPtr<FUserOnlineAccount> FOnlineIdentityAccelByte::GetUserAccount(const FUniqueNetId& UserId) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	const TSharedRef<FUserOnlineAccount>* Account = NetIdToOnlineAccountMap.Find(UserId.AsShared());
	if (Account != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("FOnlineUserAccount found for user with NetID of '%s'!"), *UserId.ToDebugString());

		return *Account;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("FOnlineUserAccount not found for user with NetID of '%s'! Returning nullptr."), *UserId.ToDebugString());
	return nullptr;
}

TArray<TSharedPtr<FUserOnlineAccount>> FOnlineIdentityAccelByte::GetAllUserAccounts() const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("N/A"));

	TArray<TSharedPtr<FUserOnlineAccount>> Result;
	Result.Empty(NetIdToOnlineAccountMap.Num());

	for (const auto& Entry : NetIdToOnlineAccountMap)
	{
		Result.Add(Entry.Value);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Returning TArray of accounts with %d entries."), Result.Num());
	return Result;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityAccelByte::GetUniquePlayerId(int32 LocalUserNum) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("LocalUserNum: %d"), LocalUserNum);

	const TSharedRef<const FUniqueNetId>* UserId = LocalUserNumToNetIdMap.Find(LocalUserNum);
	if (UserId != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Found NetId for LocalUserNum of '%d'. ID is '%s'."), LocalUserNum, *(*UserId)->ToDebugString());

		return *UserId;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Could not find NetId for LocalUserNum of '%d'. Returning nullptr."), LocalUserNum);
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("LocalUserNum: %d"), LocalUserNum);

	const ELoginStatus::Type* FoundLoginStatus = LocalUserNumToLoginStatusMap.Find(LocalUserNum);
	if (FoundLoginStatus != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("LoginStatus: %s"), ELoginStatus::ToString(*FoundLoginStatus));
		return *FoundLoginStatus;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Could not find login status stored for user at index '%d'! Returning NotLoggedIn."), LocalUserNum);
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityAccelByte::GetLoginStatus(const FUniqueNetId& UserId) const
{
	// #SG #NOTE (Maxwell): Changing all verbosity to VeryVerbose to prevent log spam with controllers
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("UserId: %s"), *UserId.ToDebugString());

	int32 LocalUserNum;
	if (!GetLocalUserNum(UserId, LocalUserNum))
	{
		// #NOTE (Maxwell): Technically this should be a warning, but since this is used to check controller status we'll just want to very verbose it.
		// In the end, we will probably want to change the controller connection code to use the built-in delegates.
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Could not get login status for user '%s' as their local user index could not be found!"), *UserId.ToDebugString());
		return ELoginStatus::NotLoggedIn;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT(""));
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

	if (LocalPlayerLoggingIn[LocalUserNum])
	{
		// This method is one of the last called when log in process completes. With this in mind, update the log in
		// state used for duplicate prevention here.
		LocalPlayerLoggingIn[LocalUserNum] = false;
	}

	TriggerOnLoginStatusChangedDelegates(LocalUserNum, OldStatus, NewStatus, (UserId.IsValid() ? UserId.ToSharedRef().Get() : FUniqueNetIdAccelByteUser::Invalid().Get()));
}

void FOnlineIdentityAccelByte::AddAuthenticatedServer(int32 LocalUserNum)
{
	SetLoginStatus(LocalUserNum, ELoginStatus::LoggedIn);

	// #NOTE Not adding net IDs here as servers have no user IDs, keep that in mind when making server calls
}

void FOnlineIdentityAccelByte::FinalizeLoginQueueCancel(int32 LoginUserNum)
{
	FScopeLock ScopeLock(&LocalUserNumToLoginQueueTicketLock);
	LocalUserNumToLoginQueueTicketMap.Remove(LoginUserNum);
	
	TriggerAccelByteOnLoginQueueCanceledByUserDelegates(LoginUserNum);
}

void FOnlineIdentityAccelByte::InitializeLoginQueue(int32 LoginUserNum, const FString& TicketId)
{
	FScopeLock ScopeLock(&LocalUserNumToLoginQueueTicketLock);
	LocalUserNumToLoginQueueTicketMap.Emplace(LoginUserNum, TicketId);
}

void FOnlineIdentityAccelByte::FinalizeLoginQueue(int32 LoginUserNum)
{
	FScopeLock ScopeLock(&LocalUserNumToLoginQueueTicketLock);
	LocalUserNumToLoginQueueTicketMap.Remove(LoginUserNum);
}

FString FOnlineIdentityAccelByte::GetPlayerNickname(int32 LocalUserNum) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	const TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId found for User %d, getting nickname from UserId."), LocalUserNum);
		return GetPlayerNickname(*UserId);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Nickname could not be queried for user %d. Returning 'NullUser'."), LocalUserNum);
	return TEXT("NullUser");
}

FString FOnlineIdentityAccelByte::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	const TSharedPtr<FUserOnlineAccountAccelByte> AccelByteAccount = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(GetUserAccount(UserId));
	if (AccelByteAccount.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Nickname found. User nickname is '%s'"), *AccelByteAccount->GetDisplayName());
		return AccelByteAccount->GetDisplayName();
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("FUserOnlineAccountAccelByte not found for user with NetID of '%s'!. Returning 'NullUser'."), *UserId.ToDebugString());
	return TEXT("NullUser");
}

FString FOnlineIdentityAccelByte::GetAuthToken(int32 LocalUserNum) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	const TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		const TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(UserId.ToSharedRef().Get());
		if (UserAccount.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found access token for User '%d'"), LocalUserNum);
			return UserAccount->GetAccessToken();
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Could not find access token for User '%d'"), LocalUserNum);
	return FString();
}

void FOnlineIdentityAccelByte::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	UE_LOG_ONLINE_IDENTITY(Display, TEXT("FOnlineIdentityAccelByte::RevokeAuthToken not implemented"));
	TSharedRef<const FUniqueNetId> UserIdRef(UserId.AsShared());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("AccelByte online subsystem is null"));
		Delegate.ExecuteIfBound(*UserIdRef, FOnlineError(FString(TEXT("AccelByte online subsystem is null"))));
		return;
	}

	AccelByteSubsystemPtr->ExecuteNextTick([UserIdRef, Delegate]()
	{
		Delegate.ExecuteIfBound(*UserIdRef, FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))));
	});
}

void FOnlineIdentityAccelByte::GetLinkedAccountAuthToken(int32 LocalUserNum, const FOnGetLinkedAccountAuthTokenCompleteDelegate& Delegate) const
{
	FString AccessToken = GetAuthToken(LocalUserNum);
	const bool bWasSuccessful = !AccessToken.IsEmpty();
	FExternalAuthToken AuthToken;
	AuthToken.TokenString = AccessToken;
	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, AuthToken);
}

void FOnlineIdentityAccelByte::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteCompositeId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	if (!AccelByteCompositeId->IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as their unique ID was invalid."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::UserNotFound));
		return;
	}

	if (!AccelByteCompositeId->HasPlatformInformation())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as they do not have native platform information."));
		// #NOTE (Maxwell): Just for testing with non-Steam accounts. Remove me!
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::NoFailures));
		return;
	}

	TSharedPtr<const FUniqueNetId> NativeUserId = AccelByteCompositeId->GetPlatformUniqueId();
	if (!NativeUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as we could not get a native platform ID for them."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::UserNotFound));
		return;
	}

	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as there is not a native platform OSS loaded."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::GenericFailure));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = NativeSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as the identity interface for native OSS is invalid."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::GenericFailure));
		return;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending request to native OSS to get privileges for native user '%s'!"), *NativeUserId->ToDebugString());

	// Create an internal delegate so that we can pass back our AccelByte user ID instead of just the platform ID
	const FOnGetUserPrivilegeCompleteDelegate OnGetNativeUserPrivilegeCompleteDelegate = FOnGetUserPrivilegeCompleteDelegate::CreateThreadSafeSP(AsShared(), &FOnlineIdentityAccelByte::OnGetNativeUserPrivilegeComplete, Delegate, AccelByteCompositeId);
	return IdentityInterface->GetUserPrivilege(NativeUserId.ToSharedRef().Get(), Privilege, OnGetNativeUserPrivilegeCompleteDelegate);
}

FPlatformUserId FOnlineIdentityAccelByte::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("NetId: %s"), *UniqueNetId.ToDebugString());

	// NOTE(Maxwell, 4/7/2021): This will return what is essentially the LocalUserNum, which we retrieve by iterating
	// through all entries of the map and returning the key that has its associated value set to the UniqueId passed in.
	for (const auto& Entry : LocalUserNumToNetIdMap)
	{
		if (Entry.Value.Get() == UniqueNetId)
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserNum found: %d"), Entry.Key);
#if ENGINE_MAJOR_VERSION >= 5
			return FPlatformMisc::GetPlatformUserForUserIndex(Entry.Key);
#else
			return Entry.Key;
#endif
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserNum not found for NetId '%s'"), *UniqueNetId.ToDebugString());
	return PLATFORMUSERID_NONE;
}

FString FOnlineIdentityAccelByte::GetAuthType() const
{
	return TEXT("AccelByte");
}

#if ENGINE_MAJOR_VERSION > 4
int32 FOnlineIdentityAccelByte::GetLocalUserNumFromPlatformUserId(FPlatformUserId PlatformUserId) const
{
	return IOnlineIdentity::GetLocalUserNumFromPlatformUserId(PlatformUserId);
}
#endif

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

bool FOnlineIdentityAccelByte::GetLocalUserNumFromPlatformUserId(const FString& PlatformUserId, int32& OutLocalUserNum)
{
	for (const auto& LocalUser : LocalUserNumToNetIdMap)
	{
		int32 InLocalUserNum = LocalUser.Key;
		const auto& NetId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(LocalUser.Value);
		if (NetId->GetPlatformId() == PlatformUserId)
		{
			OutLocalUserNum = InLocalUserNum;
			return true;
		}
	}
	return false;
}

AccelByte::FApiClientPtr FOnlineIdentityAccelByte::GetApiClient(const FUniqueNetId& NetId)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("AccelByte online subsystem is null"));
		return nullptr;
	}

	return AccelByteSubsystemPtr->GetApiClient(NetId);
}

AccelByte::FApiClientPtr FOnlineIdentityAccelByte::GetApiClient(int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("AccelByte online subsystem is null"));
		return nullptr;
	}
	return AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
}

bool FOnlineIdentityAccelByte::AuthenticateAccelByteServer(const FOnAuthenticateServerComplete& Delegate, int32 LocalUserNum)
{
#if AB_USE_V2_SESSIONS
	UE_LOG_AB(Warning, TEXT("FOnlineIdentityAccelByte::AuthenticateAccelByteServer is deprecated with V2 sessions. Servers should be authenticated through AutoLogin!"));
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
		{
			Delegate.ExecuteIfBound(false);
		});
	}
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

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLobbyConnect"
bool FOnlineIdentityAccelByte::ConnectAccelByteLobby(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

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
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User already connected to lobby at user index '%d'!"), LocalUserNum);

			TriggerOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
			TriggerAccelByteOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ONLINE_ERROR_ACCELBYTE(ErrorStr));
			return false;
		}

		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if (!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte online subsystem is null"));
			const FString ErrorStr = TEXT("connect-failed-online-subsystem-null");
			TriggerOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
			TriggerAccelByteOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ONLINE_ERROR_ACCELBYTE(ErrorStr));
			return false;
		}
		else
		{
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), *GetUniquePlayerId(LocalUserNum).Get());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to connect lobby!"));
			return true;
		}
	}

	const FString ErrorStr = TEXT("connect-failed-not-logged-in");
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);

	TriggerOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
	TriggerAccelByteOnConnectLobbyCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ONLINE_ERROR_ACCELBYTE(ErrorStr));
	return false;
}
#undef ONLINE_ERROR_NAMESPACE

void FOnlineIdentityAccelByte::SetAutoLoginCreateHeadless(bool bInIsAutoLoginCreateHeadless)
{
	bIsAutoLoginCreateHeadless = bInIsAutoLoginCreateHeadless;
}

void FOnlineIdentityAccelByte::AddNewAuthenticatedUser(int32 LocalUserNum, const TSharedRef<const FUniqueNetId>& UserId, const TSharedRef<FUserOnlineAccount>& Account)
{
	// Add to mappings for the user
	LocalUserNumToNetIdMap.Add(LocalUserNum, UserId);
	LocalUserNumToLoginStatusMap.Add(LocalUserNum, ELoginStatus::NotLoggedIn);
	NetIdToLocalUserNumMap.Add(UserId, LocalUserNum);
	NetIdToOnlineAccountMap.Add(UserId, Account);
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLogout"
void FOnlineIdentityAccelByte::OnLogout(const int32 LocalUserNum, bool bWasSuccessful)
{
	SetLoginStatus(LocalUserNum, ELoginStatus::NotLoggedIn);
	TriggerOnLogoutCompleteDelegates(LocalUserNum, bWasSuccessful);
	TriggerAccelByteOnLogoutCompleteDelegates(LocalUserNum, bWasSuccessful, ONLINE_ERROR_ACCELBYTE(bWasSuccessful? TEXT("") : TEXT("error-failed-to-logout"), bWasSuccessful? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));

	if (bWasSuccessful)
	{
		UE_LOG_AB(Log, TEXT("Logging out with AccelByte OSS succeeded! LocalUserNum: %d"), LocalUserNum);
		RemoveUserFromMappings(LocalUserNum);
	}
}
#undef ONLINE_ERROR_NAMESPACE

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

		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if (AccelByteSubsystemPtr.IsValid() && AccelByteSubsystemPtr->GetLocalUserNumCached() == LocalUserNum)
		{
			AccelByteSubsystemPtr->ResetLocalUserNumCached();
		}

		LocalUserNumToNetIdMap.Remove(LocalUserNum);
	}
}

void FOnlineIdentityAccelByte::OnGetNativeUserPrivilegeComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, FOnGetUserPrivilegeCompleteDelegate OriginalDelegate, TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId)
{
	OriginalDelegate.ExecuteIfBound(AccelByteId.Get(), Privilege, PrivilegeResults);
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLogin"
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

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		AccelByteSubsystemPtr->SetLocalUserNumCached(LocalUserNum);
	}

	TriggerOnLoginCompleteDelegates(LocalUserNum, true, FUniqueNetIdAccelByteUser::Invalid().Get(), TEXT(""));
	TriggerAccelByteOnLoginCompleteDelegates(LocalUserNum, true, FUniqueNetIdAccelByteUser::Invalid().Get(), ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success));
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
#undef ONLINE_ERROR_NAMESPACE

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

void FOnlineIdentityAccelByte::GenerateCodeForPublisherToken(int32 InLocalUserNum, const FString& PublisherClientId , FGenerateCodeForPublisherTokenComplete Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();

	if (GetLoginStatus(InLocalUserNum) == ELoginStatus::LoggedIn && GetApiClient(InLocalUserNum).IsValid() && AccelByteSubsystemPtr.IsValid())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken>(AccelByteSubsystemPtr.Get(), InLocalUserNum, PublisherClientId, Delegate);
	}
	else
	{
		Delegate.ExecuteIfBound(false, FCodeForTokenExchangeResponse());
	}

}

bool FOnlineIdentityAccelByte::RefreshPlatformToken(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("RefreshNativePlatformToken LocalUserNum: %d"), LocalUserNum);

	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	
	if (NativeSubsystem == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Native online subsystem is null!"));
		return false;
	}

	FName Name = NativeSubsystem->GetSubsystemName();
	if (!Name.IsValid() || Name.ToString().IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Native online subsystem name is empty!"));
		return false;
	}

	return RefreshPlatformToken(LocalUserNum, Name);
}

bool FOnlineIdentityAccelByte::RefreshPlatformToken(int32 LocalUserNum, FName SubsystemName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("RefreshPlatformToken LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte online subsystem is null"));
		const FString ErrorStr = TEXT("refresh-native-platform-token-failed-online-subsystem-null");
		TriggerAccelByteOnPlatformTokenRefreshedCompleteDelegates(LocalUserNum, false, {}, {}, {});
		return false;
	}
	else
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshPlatformToken>(AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, SubsystemName);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to refresh platform token!"));
	}
	return true;
}

bool FOnlineIdentityAccelByte::CancelLoginQueue(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if(!LocalUserNumToLoginQueueTicketMap.Contains(LocalUserNum))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("local user queue ticket not found"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte online subsystem is null"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginQueueCancelTicket>(AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, LocalUserNumToLoginQueueTicketMap[LocalUserNum]);
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to cancel login queue!"));
	return true;
}


/** End FOnlineIdentityAccelByte */
