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
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteDisableMfaAuthenticator.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteDisableMfaBackupCodes.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteDisableMfaEmail.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteEnableMfaAuthenticator.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteEnableMfaBackupCodes.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteEnableMfaEmail.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteGenerateMfaAuthenticatorSecretKey.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteGenerateMfaBackupCodes.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteGetMfaStatus.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteSendMfaCodeToEmail.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteUpdatePassword.h"
#include "AsyncTasks/Identity/OnlineAsyncTaskAccelByteVerifyLoginMfa.h"
#include "AsyncTasks/LoginQueue/OnlineAsyncTaskAccelByteLoginQueueClaimTicket.h"
#include "OnlineSubsystemAccelByteLog.h"

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

FOnlineIdentityAccelByte::~FOnlineIdentityAccelByte()
{
	FReadScopeLock Lock(LocalUserNumToLogoutTaskMtx);
	for (auto& TaskPair : LocalUserNumToLogoutTask)
	{
		auto TaskPtr = TaskPair.Value.Pin();
		if (TaskPtr.IsValid())
		{
			TaskPtr->Cancel();
		}
	}
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

	{
		FWriteScopeLock Lock(LocalPlayerLoggingInMtx);
		if (LocalPlayerLoggingIn[LocalUserNum])
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning
				, TEXT("Player at index '%d' is already logging in! Ignoring subsequent call to log in.")
				, LocalUserNum);
			// Do not trigger delegates as it could impact handlers for previous log in attempt
			return false;
		}
		LocalPlayerLoggingIn[LocalUserNum] = true;
	}

	bool NetIdFound = false;
	{
		FReadScopeLock Lock(LocalUserNumToNetIdMapMtx);
		NetIdFound = LocalUserNumToNetIdMap.Contains(LocalUserNum);
	}
	// Don't attempt to authenticate again if we are already reporting as logged in
	if (GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn 
		&& NetIdFound
		&& (AccountCredentials.Type != FAccelByteUtilities::GetUEnumValueAsString(EAccelByteLoginType::RefreshToken)))
	{

		const FUniqueNetIdPtr LocalUserId = GetUniquePlayerId(LocalUserNum);
		TSharedPtr<FUserOnlineAccount> UserAccount;
		if (LocalUserId.IsValid())
		{
			UserAccount = GetUserAccount(LocalUserId);
		}

		const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
		const FString ErrorStr = TEXT("login-failed-already-logged-in");
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User already logged in at user index '%d'!"), LocalUserNum);
		
		// Clear log in progress flag
		{
			FWriteScopeLock Lock(LocalPlayerLoggingInMtx);
			LocalPlayerLoggingIn[LocalUserNum] = false;
		}

		TriggerOnLoginChangedDelegates(LocalUserNum);
		TriggerOnLoginCompleteDelegates(LocalUserNum, true, *LocalUserId, ErrorStr);
		FErrorOAuthInfo ErrorOAuthInfo { (int32)ErrorCodes::InvalidRequest, ErrorStr,
			FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest), ErrorStr };
		TriggerOnLoginWithOAuthErrorCompleteDelegates(LocalUserNum, false, *LocalUserId, ErrorOAuthInfo);
		TriggerAccelByteOnLoginCompleteDelegates(LocalUserNum, false, *LocalUserId, ONLINE_ERROR_ACCELBYTE(ErrorStr));
		return false;
	}

	FOnlineAsyncTaskInfo TaskInfo;
	TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
	TaskInfo.bCreateEpicForThis = true;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Clear log in progress flag
		{
			FWriteScopeLock Lock(LocalPlayerLoggingInMtx);
			LocalPlayerLoggingIn[LocalUserNum] = false;
		}
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
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to simultaneous login!"));
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

	FVoidHandler OnLogoutSuccessDelegate = FVoidHandler::CreateThreadSafeSP(AsShared()
		, &FOnlineIdentityAccelByte::OnLogout
		, LocalUserNum
		, true);
	FErrorHandler OnLogoutFailedDelegate = FErrorHandler::CreateThreadSafeSP(AsShared()
		, &FOnlineIdentityAccelByte::OnLogoutError
		, LocalUserNum);

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

		const auto Lobby = ApiClient->GetLobbyApi().Pin();
		if (!Lobby.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as an Lobby API could not be found for them!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}

		const auto Chat = ApiClient->GetChatApi().Pin();
		if (!Chat.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as an Chat API could not be found for them!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}

		const auto User = ApiClient->GetUserApi().Pin();
		if (!User.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as an User API could not be found for them!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}

		// @todo multiuser Change this to logout the user based on LocalUserNum when SDK supports multiple user login
		// NOTE(Maxwell, 4/8/2021): Logout automatically signals to the credential store to forget all credentials, so this is all we need to call
		if (Lobby->IsConnected())
		{
			Lobby->Disconnect(true);
		}
		if (Chat->IsConnected())
		{
			Chat->Disconnect(true);
		}
		auto TaskWPtr = User->LogoutV3(OnLogoutSuccessDelegate, OnLogoutFailedDelegate);
		{
			FWriteScopeLock Lock(LocalUserNumToLogoutTaskMtx);
			LocalUserNumToLogoutTask.Emplace(LocalUserNum, TaskWPtr);
		}
	}
	else
	{
		FOnlineSubsystemAccelBytePtr Subsytem = AccelByteSubsystem.Pin();
		if(!Subsytem.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as the Subsystem is invalid!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}

		FAccelByteInstancePtr AccelByteInstance = Subsytem->GetAccelByteInstance().Pin();
		if(!AccelByteInstance.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as the AccelByteInstance in the subsystem is invalid!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}
		
		AccelByte::FServerApiClientPtr ApiClient = AccelByteInstance->GetServerApiClient();
		if (!ApiClient.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to log out user %d as an API client could not be found for them!"), LocalUserNum);
			OnLogout(LocalUserNum, false);
			return false;
		}

		ApiClient->ServerCredentialsRef->Shutdown();
		ApiClient->ServerCredentialsRef->ForgetAll();
		
		AccelByteInstance->RemoveApiClient();
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
#if !AB_USE_V2_SESSIONS
		// #NOTE For compatibility reasons, V1 sessions still won't support auto login, since those tasks already have a way
		// to authenticate a server.
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AutoLogin does not work with AccelByte servers, server login is done automatically during registration."));
		return false;
#else
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
#endif
	}

	FOnlineAccountCredentialsAccelByte Credentials{ bIsAutoLoginCreateHeadless.load(std::memory_order_acquire) };

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

TSharedPtr<FUserOnlineAccount> FOnlineIdentityAccelByte::GetUserAccount(const FUniqueNetIdRef& UserId) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	const TSharedRef<FUserOnlineAccount>* Account = nullptr;
	{
		FReadScopeLock Lock(NetIdToOnlineAccountMapMtx);
		Account = NetIdToOnlineAccountMap.Find(UserId);
	}
	if (Account != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("FOnlineUserAccount found for user with NetID of '%s'!"), *UserId->ToDebugString());

		return *Account;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("FOnlineUserAccount not found for user with NetID of '%s'! Returning nullptr."), *UserId->ToDebugString());
	return nullptr;
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityAccelByte::GetUserAccount(const FUniqueNetIdPtr& UserId) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!UserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("NetID is invalid! Returning nullptr."));
		return nullptr;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Calling GetUserAccount with SharedRef"));
	return GetUserAccount(UserId.ToSharedRef());
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityAccelByte::GetUserAccount(const FUniqueNetId& UserId) const
{
	return GetUserAccount(UserId.AsShared());
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityAccelByte::GetUserAccount(int LocalUserNum) const
{
	const FUniqueNetIdPtr LocalUserId = GetUniquePlayerId(LocalUserNum);
	return GetUserAccount(LocalUserId);
}

TArray<TSharedPtr<FUserOnlineAccount>> FOnlineIdentityAccelByte::GetAllUserAccounts() const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("N/A"));

	FReadScopeLock Lock(NetIdToOnlineAccountMapMtx);
	TArray<TSharedPtr<FUserOnlineAccount>> Result;
	Result.Empty(NetIdToOnlineAccountMap.Num());

	for (const auto& Entry : NetIdToOnlineAccountMap)
	{
		Result.Add(Entry.Value);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Returning TArray of accounts with %d entries."), Result.Num());
	return Result;
}

FUniqueNetIdPtr FOnlineIdentityAccelByte::GetUniquePlayerId(int32 LocalUserNum) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("LocalUserNum: %d"), LocalUserNum);

	const TSharedRef<const FUniqueNetId>* UserId = nullptr;
	{
		FReadScopeLock Lock(LocalUserNumToNetIdMapMtx);
		UserId = LocalUserNumToNetIdMap.Find(LocalUserNum);
	}
	if (UserId)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Found NetId for LocalUserNum of '%d'. ID is '%s'."), LocalUserNum, *(*UserId)->ToDebugString());
		return *UserId;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Could not find NetId for LocalUserNum of '%d'. Returning nullptr."), LocalUserNum);
	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityAccelByte::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	if (Bytes && Size > 0)
	{
		FString StrId(Size, (TCHAR*)Bytes);
		return CreateUniquePlayerId(StrId);
	}

	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityAccelByte::CreateUniquePlayerId(const FString& Str)
{
	return FUniqueNetIdAccelByteUser::Create(Str);
}

ELoginStatus::Type FOnlineIdentityAccelByte::GetLoginStatus(int32 LocalUserNum) const
{
	// #SG #NOTE (Maxwell): Changing all verbosity to VeryVerbose to prevent log spam with controllers
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(VeryVerbose, TEXT("LocalUserNum: %d"), LocalUserNum);

	const ELoginStatus::Type* FoundLoginStatus = nullptr;
	{
		FReadScopeLock Lock(LocalUserNumToLoginStatusMapMtx);
		FoundLoginStatus = LocalUserNumToLoginStatusMap.Find(LocalUserNum);
	}
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
	{
		FWriteScopeLock Lock(LocalUserNumToLoginStatusMapMtx);
		LocalUserNumToLoginStatusMap.Add(LocalUserNum, NewStatus);
	}
	{
		FWriteScopeLock Lock(LocalPlayerLoggingInMtx);
		if (LocalPlayerLoggingIn[LocalUserNum])
		{
			// This method is one of the last called when log in process completes. With this in mind, update the log in
			// state used for duplicate prevention here.
			LocalPlayerLoggingIn[LocalUserNum] = false;
		}
	}

	const FUniqueNetIdPtr LocalUserId = GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		// No need to trigger delegate 
		return;
	}
	TriggerOnLoginStatusChangedDelegates(LocalUserNum, OldStatus, NewStatus, *LocalUserId);
}

void FOnlineIdentityAccelByte::AddAuthenticatedServer(int32 LocalUserNum)
{
	SetLoginStatus(LocalUserNum, ELoginStatus::LoggedIn);

	// #NOTE Not adding net IDs here as servers have no user IDs, keep that in mind when making server calls
}

void FOnlineIdentityAccelByte::FinalizeLoginQueueCancel(int32 LoginUserNum)
{
	FWriteScopeLock Lock(LocalUserNumToLoginQueueTicketMapMtx);
	LocalUserNumToLoginQueueTicketMap.Remove(LoginUserNum);
	
	TriggerAccelByteOnLoginQueueCanceledByUserDelegates(LoginUserNum);
}

void FOnlineIdentityAccelByte::InitializeLoginQueue(int32 LoginUserNum, const FString& TicketId)
{
	FWriteScopeLock Lock(LocalUserNumToLoginQueueTicketMapMtx);
	LocalUserNumToLoginQueueTicketMap.Emplace(LoginUserNum, TicketId);
}

void FOnlineIdentityAccelByte::FinalizeLoginQueue(int32 LoginUserNum)
{
	FWriteScopeLock Lock(LocalUserNumToLoginQueueTicketMapMtx);
	LocalUserNumToLoginQueueTicketMap.Remove(LoginUserNum);
}

FString FOnlineIdentityAccelByte::GetPlayerNickname(int32 LocalUserNum) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	const FUniqueNetIdPtr LocalUserId = GetUniquePlayerId(LocalUserNum);
	if (LocalUserId.IsValid() && LocalUserId->IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId found for User %d, getting nickname from UserId."), LocalUserNum);
		return GetPlayerNickname(*LocalUserId);
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

	const FUniqueNetIdPtr LocalUserId = GetUniquePlayerId(LocalUserNum);
	if (LocalUserId.IsValid() && LocalUserId->IsValid())
	{
		const TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(LocalUserId);
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

#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)
void FOnlineIdentityAccelByte::GetLinkedAccountAuthToken(int32 LocalUserNum, const FOnGetLinkedAccountAuthTokenCompleteDelegate& Delegate) const
{
	FString AccessToken = GetAuthToken(LocalUserNum);
	const bool bWasSuccessful = !AccessToken.IsEmpty();
	FExternalAuthToken AuthToken;
	AuthToken.TokenString = AccessToken;
	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, AuthToken);
}
#endif // !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
void FOnlineIdentityAccelByte::GetLinkedAccountAuthToken(int32 LocalUserNum, const FString&, const FOnGetLinkedAccountAuthTokenCompleteDelegate& Delegate) const
{
	FString AccessToken = GetAuthToken(LocalUserNum);
	const bool bWasSuccessful = !AccessToken.IsEmpty();
	FExternalAuthToken AuthToken;
	AuthToken.TokenString = AccessToken;
	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, AuthToken);
}
#endif // ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
void FOnlineIdentityAccelByte::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate, EShowPrivilegeResolveUI ShowResolveUI)
#else
void FOnlineIdentityAccelByte::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate)
#endif // ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
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

	FUniqueNetIdPtr NativeUserId = AccelByteCompositeId->GetPlatformUniqueId();
	if (!NativeUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as we could not get a native platform ID for them."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::UserNotFound));
		return;
	}

	TSharedPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query privileges for user as our internal subsystem pointer was invalid."));
		Delegate.ExecuteIfBound(UserId, Privilege, static_cast<uint32>(EPrivilegeResults::UserNotFound));
		return;
	}

	const IOnlineSubsystem* NativeSubsystem = AccelByteSubsystemPtr->GetNativePlatformSubsystem();
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
	FReadScopeLock Lock(LocalUserNumToNetIdMapMtx);
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
	const int32* FoundLocalUserNum = nullptr;
	{
		FReadScopeLock Lock(NetIdToLocalUserNumMapMtx);
		FoundLocalUserNum = NetIdToLocalUserNumMap.Find(NetId.AsShared());
	}
	if (FoundLocalUserNum != nullptr)
	{
		OutLocalUserNum = *FoundLocalUserNum;
		return true;
	}
	else
	{
		return false;
	}
}

bool FOnlineIdentityAccelByte::GetLocalUserNumFromPlatformUserId(const FString& PlatformUserId, int32& OutLocalUserNum)
{
	FReadScopeLock Lock(LocalUserNumToNetIdMapMtx);
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();

#if !AB_USE_V2_SESSIONS
// Empty statement, do nothing.
#else
	UE_LOG_AB(Warning, TEXT("FOnlineIdentityAccelByte::AuthenticateAccelByteServer is deprecated with V2 sessions. Servers should be authenticated through AutoLogin!"));
	
	if (AccelByteSubsystemPtr.IsValid())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
		{
			Delegate.ExecuteIfBound(false);
		});
	}
	return false;
#endif

#if (defined(UE_SERVER) && UE_SERVER) || (defined(UE_EDITOR) && UE_EDITOR)
	if (!bIsServerAuthenticated.load(std::memory_order_acquire)
		&& !bIsAuthenticatingServer.load(std::memory_order_acquire))
	{
		bIsAuthenticatingServer.store(true, std::memory_order_release);
		{
			FWriteScopeLock Lock(ServerAuthDelegatesMtx);
			ServerAuthDelegates.Add(Delegate);
		}

		const TSharedRef<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = SharedThis(this);
		const FVoidHandler OnLoginSuccess = FVoidHandler::CreateThreadSafeSP(IdentityInterface, &FOnlineIdentityAccelByte::OnAuthenticateAccelByteServerSuccess, LocalUserNum);
		const FErrorHandler OnLoginError = FErrorHandler::CreateThreadSafeSP(IdentityInterface, &FOnlineIdentityAccelByte::OnAuthenticateAccelByteServerError);

		if(!AccelByteSubsystemPtr.IsValid())
		{
			AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
			{
				UE_LOG_AB(Warning, TEXT("Server AccelByteSubsystem is invalid, skipping call!"));
				Delegate.ExecuteIfBound(false);
			});
		}

		FAccelByteInstancePtr AccelByteInstancePtr = AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
		if(!AccelByteInstancePtr.IsValid())
		{
			AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
			{
				UE_LOG_AB(Warning, TEXT("Server AccelByteInstance is invalid, skipping call!"));
				Delegate.ExecuteIfBound(false);
			});
		}

		FServerApiClientPtr ServerApiClientPtr = AccelByteInstancePtr->GetServerApiClient();
		if(!ServerApiClientPtr.IsValid())
		{
			AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
			{
				UE_LOG_AB(Warning, TEXT("Server ServerApiClient is invalid, skipping call!"));
				Delegate.ExecuteIfBound(false);
			});
		}
		
		ServerApiClientPtr->ServerOauth2.LoginWithClientCredentials(OnLoginSuccess, OnLoginError);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Server is being or has already authenticated, skipping call!"));
		
		if (bIsServerAuthenticated.load(std::memory_order_acquire))
		{
			Delegate.ExecuteIfBound(true);
		}
		else if (bIsAuthenticatingServer.load(std::memory_order_acquire))
		{
			FWriteScopeLock Lock(ServerAuthDelegatesMtx);
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

	bool NetIdFound = false;
	{
		FReadScopeLock Lock(LocalUserNumToNetIdMapMtx);
		NetIdFound = LocalUserNumToNetIdMap.Contains(LocalUserNum);
	}
	// Don't attempt to connect again if we are already reporting as connected
	if (GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn && NetIdFound)
	{
		const FUniqueNetIdPtr LocalUserId = GetUniquePlayerId(LocalUserNum);
		TSharedPtr<FUserOnlineAccount> UserAccount;
		if (LocalUserId.IsValid())
		{
			UserAccount = GetUserAccount(LocalUserId);
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
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), *LocalUserId);
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
	bIsAutoLoginCreateHeadless.store(bInIsAutoLoginCreateHeadless, std::memory_order_release);
}

void FOnlineIdentityAccelByte::AddNewAuthenticatedUser(int32 LocalUserNum, const TSharedRef<const FUniqueNetId>& UserId, const TSharedRef<FUserOnlineAccount>& Account)
{
	// Add to mappings for the user
	{
		FWriteScopeLock Lock(LocalUserNumToNetIdMapMtx);
		LocalUserNumToNetIdMap.Emplace(LocalUserNum, UserId);
	}
	{
		FWriteScopeLock Lock(LocalUserNumToLoginStatusMapMtx);
		LocalUserNumToLoginStatusMap.Emplace(LocalUserNum, ELoginStatus::NotLoggedIn);
	}
	{
		FWriteScopeLock Lock(NetIdToLocalUserNumMapMtx);
		NetIdToLocalUserNumMap.Emplace(UserId, LocalUserNum);
	}
	{
		FWriteScopeLock Lock(NetIdToOnlineAccountMapMtx);
		NetIdToOnlineAccountMap.Emplace(UserId, Account);
	}
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLogout"
void FOnlineIdentityAccelByte::OnLogout(const int32 LocalUserNum, bool bWasSuccessful)
{
	SetLoginStatus(LocalUserNum, ELoginStatus::NotLoggedIn);

#if !AB_USE_V2_SESSIONS
// Empty statement, do nothing.
#else
	// Signal to the session interface that a player has logged out to clean the local cache
	FOnlineSessionV2AccelBytePtr SessionInterface{};
	const TSharedRef<const FUniqueNetId>* LocalPlayerId = nullptr;
	{
		FReadScopeLock Lock(LocalUserNumToNetIdMapMtx);
		LocalPlayerId = LocalUserNumToNetIdMap.Find(LocalUserNum);
	}

	TSharedPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> PinnedSubsystem = AccelByteSubsystem.Pin();
	const bool bSessionInterfaceValid = PinnedSubsystem.IsValid() &&
		FOnlineSessionV2AccelByte::GetFromSubsystem(PinnedSubsystem.Get(), SessionInterface);
	if (LocalPlayerId != nullptr && bSessionInterfaceValid)
	{
		SessionInterface->HandleUserLogoutCleanUp((*LocalPlayerId).Get());
	}
#endif

	// Signal to game client that log out has completed
	TriggerOnLogoutCompleteDelegates(LocalUserNum, bWasSuccessful);
	TriggerAccelByteOnLogoutCompleteDelegates(LocalUserNum, bWasSuccessful, ONLINE_ERROR_ACCELBYTE(bWasSuccessful? TEXT("") : TEXT("error-failed-to-logout"), bWasSuccessful? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));

	//Unbind Event before user removed/deleted
	{
		AccelByte::FApiClientPtr ApiClient = GetApiClient(LocalUserNum);
		if (ApiClient.IsValid())
		{
			const auto Lobby = ApiClient->GetLobbyApi().Pin();
			if (Lobby.IsValid())
			{
				Lobby->UnbindEvent();
			}
			const auto Chat = ApiClient->GetLobbyApi().Pin();
			if (Chat.IsValid())
			{
				Chat->UnbindEvent();
			}
		}
	}

	if (bWasSuccessful)
	{
		UE_LOG_AB(Log, TEXT("Logging out with AccelByte OSS succeeded! LocalUserNum: %d"), LocalUserNum);
		RemoveUserFromMappings(LocalUserNum);
	}
	{	
		FWriteScopeLock Lock(LocalUserNumToLogoutTaskMtx);
		LocalUserNumToLogoutTask.Remove(LocalUserNum);
	}
}

void FOnlineIdentityAccelByte::OnLogoutError(int32 ErrorCode, const FString& ErrorMessage, int32 LocalUserNum)
{
	UE_LOG_AB(Error, TEXT("Logging out with AccelByte OSS failed! Code: %d; Message: %s"), ErrorCode, *ErrorMessage);
	OnLogout(LocalUserNum, false);
	{
		FWriteScopeLock Lock(LocalUserNumToLogoutTaskMtx);
		LocalUserNumToLogoutTask.Remove(LocalUserNum);
	}
}
#undef ONLINE_ERROR_NAMESPACE

void FOnlineIdentityAccelByte::RemoveUserFromMappings(const int32 LocalUserNum)
{
	const TSharedRef<const FUniqueNetId>* UniqueId = nullptr;
	{
		FReadScopeLock Lock(LocalUserNumToNetIdMapMtx);
		UniqueId = LocalUserNumToNetIdMap.Find(LocalUserNum);
	}
	if (UniqueId == nullptr)
	{
		return;
	}
	// Remove the account map first, and then remove the unique ID by local user num
	const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteUser = FUniqueNetIdAccelByteUser::CastChecked(*UniqueId);
	{
		FWriteScopeLock Lock(NetIdToLocalUserNumMapMtx);
		NetIdToLocalUserNumMap.Remove(*UniqueId);
	}
	{
		FWriteScopeLock Lock(NetIdToOnlineAccountMapMtx);
		NetIdToOnlineAccountMap.Remove(*UniqueId);
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid() && AccelByteSubsystemPtr->GetLocalUserNumCached() == LocalUserNum)
	{
		AccelByteSubsystemPtr->ResetLocalUserNumCached();

		FAccelByteInstancePtr AccelByteInstance = AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
		if(AccelByteInstance.IsValid())
		{
			AccelByteInstance->RemoveApiClient(AccelByteUser->GetAccelByteId());
		}
	}
	{
		FWriteScopeLock Lock(LocalUserNumToNetIdMapMtx);
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
	{
		FReadScopeLock Lock(ServerAuthDelegatesMtx);
		for (FOnAuthenticateServerComplete& Delegate : ServerAuthDelegates)
		{
			Delegate.ExecuteIfBound(true);
		}
	}
	{	
		FWriteScopeLock Lock(ServerAuthDelegatesMtx);
		ServerAuthDelegates.Empty();
	}
	AddAuthenticatedServer(LocalUserNum);

	// #TODO (Maxwell): This is some super nasty code to avoid duplicate server authentication, again this should just be apart of AutoLogin...
	bIsAuthenticatingServer.store(false, std::memory_order_release);
	bIsServerAuthenticated.store(true, std::memory_order_release);

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
	
	{
		FReadScopeLock Lock(ServerAuthDelegatesMtx);
		for (FOnAuthenticateServerComplete& Delegate : ServerAuthDelegates)
		{
			Delegate.ExecuteIfBound(false);
		}
	}
	{	
		FWriteScopeLock Lock(ServerAuthDelegatesMtx);
		ServerAuthDelegates.Empty();
	}

	// Reset our authentication state to false as we can retry from here
	bIsAuthenticatingServer.store(false, std::memory_order_release);
	bIsServerAuthenticated.store(false, std::memory_order_release);
}
#undef ONLINE_ERROR_NAMESPACE

bool FOnlineIdentityAccelByte::IsLogoutRequired(int32 StatusCode)
{
	// LOGOUT! NO RECONNECT (return true)
	// 1000        : Normal closure according to websocket spec. If client intentionally disconnects
	// 4001 - 4999 : Abnormal closure from AGS service that the SDK shouldn't reconnect for. Example, their access token got revoked.
	
	// RECONNECT (return false)
	// despite in those non-retryable range, it has ban notification.
	// 4001 - 4399 (+ BanNotification ): To be specific, this is the range of Lobby-only special case

	// RECONNECT (return false)
	// 1001 - 1015 : Abnormal closures reserved by websocket spec
	// 1016 - 3099 : reserved, unused
	// 3100 - 3999 : Abnormal closure from AGS service, that we allow the client to reconnect.
	// 4000        : Abnormal closure, server pod shutdown

	//Explanation of deprecation & new assigned numbers can be read in each enums between these numbers
	int LowestDeprecatedNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectSenderError); //4004
	int HighestDeprecatedNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectReaderError); //4007
	if (StatusCode >= LowestDeprecatedNumber && StatusCode <= HighestDeprecatedNumber)
	{
		return false;
	}

	// It means the retry reconnect attempt has been exhausted.
	// No need to logout, only need to trigger delegate
	if (StatusCode == static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectFromExternalReconnect)) //4401
	{
		return false;
	}

	int32 LowestNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::DisconnectServerShutdown); //4000
	int32 HighestNumber = static_cast<int32>(AccelByte::EWebsocketErrorTypes::OutsideBoundaryOfDisconnection); //4400
	// Whether the StatusCode is between those lowest & highest number
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

	TSharedPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Internal subsystem pointer is invalid!"));
		return false;
	}

	const IOnlineSubsystem* NativeSubsystem = AccelByteSubsystemPtr->GetNativePlatformSubsystem();
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
	FString LoginQueueTicket = TEXT("");
	{
		FReadScopeLock Lock(LocalUserNumToLoginQueueTicketMapMtx);
		if (!LocalUserNumToLoginQueueTicketMap.Contains(LocalUserNum))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("local user queue ticket not found"));
			return false;
		}
		else
		{
			LoginQueueTicket = LocalUserNumToLoginQueueTicketMap[LocalUserNum];
		}
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte online subsystem is null"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginQueueCancelTicket>(AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, LoginQueueTicket);
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to cancel login queue!"));
	return true;
}

bool FOnlineIdentityAccelByte::ClaimLoginQueueTicket(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	FString LoginQueueTicket = TEXT("");
	{
		FReadScopeLock Lock(LocalUserNumToLoginQueueTicketMapMtx);
		if (!LocalUserNumToLoginQueueTicketMap.Contains(LocalUserNum))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("local user queue ticket not found"));
			return false;
		}
		else
		{
			LoginQueueTicket = LocalUserNumToLoginQueueTicketMap[LocalUserNum];
		}
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte online subsystem is null"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginQueueClaimTicket>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, LoginQueueTicket);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to claim login queue ticket!"));
	return true;
}

bool FOnlineIdentityAccelByte::VerifyLoginMfa(int32 LocalUserNum
	, EAccelByteLoginAuthFactorType FactorType, const FString& Code, const FString& MfaToken, bool bRememberDevice)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't verify login Mfa for dedicated server."));
		return false;
	}

	if (FactorType == EAccelByteLoginAuthFactorType::None)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Factor type can't be None."));
		return false;
	}

	if (Code.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't verify login with empty code."));
		return false;
	}

	if (MfaToken.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't verify login with empty Mfa token."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update password for user as our internal subsystem pointer was invalid."));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteVerifyLoginMfa>(
		AccelByteSubsystemPtr.Get(), LocalUserNum, FactorType, Code, MfaToken, bRememberDevice);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::UpdatePassword(int32 LocalUserNum, const FUpdatePasswordRequest& Request
	, EAccelByteLoginAuthFactorType FactorType, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't update password for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't update password as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	UpdatePassword(*UniquePlayerId, Request, FactorType, Code);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::UpdatePassword(const FUniqueNetId& UserId, const FUpdatePasswordRequest& Request
	, EAccelByteLoginAuthFactorType FactorType, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't update password for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update password for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdatePassword>(AccelByteSubsystemPtr.Get(), UserId, Request, FactorType, Code);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::GetMfaStatus(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't retrieve MFA status for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't retrieve MFA status as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	GetMfaStatus(*UniquePlayerId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::GetMfaStatus(const FUniqueNetId& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't retrieve MFA status for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get MFA status for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetMfaStatus>(AccelByteSubsystemPtr.Get(), UserId);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::EnableMfaBackupCodes(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable MFA Backup Codes for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable MFA Backup Codes as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	EnableMfaBackupCodes(*UniquePlayerId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::EnableMfaBackupCodes(const FUniqueNetId& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable MFA Backup Codes for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to enable MFA Backup Codes for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteEnableMfaBackupCodes>(AccelByteSubsystemPtr.Get(), UserId);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::DisableMfaBackupCodes(int32 LocalUserNum, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable MFA Backup Codes for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable MFA Backup Codes as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	DisableMfaBackupCodes(*UniquePlayerId, Code);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::DisableMfaBackupCodes(const FUniqueNetId& UserId, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable MFA Backup Codes for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to disable MFA Backup Codes for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDisableMfaBackupCodes>(AccelByteSubsystemPtr.Get(), UserId, Code);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::GenerateMfaBackupCode(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't generate MFA Backup Codes for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't generate MFA Backup Codes as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	GenerateMfaBackupCode(*UniquePlayerId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::GenerateMfaBackupCode(const FUniqueNetId& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't generate MFA Backup Codes for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate MFA Backup Codes for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes>(AccelByteSubsystemPtr.Get(), UserId);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::EnableMfaAuthenticator(int32 LocalUserNum, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable MFA Authenticator for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable MFA Authenticator as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	EnableMfaAuthenticator(*UniquePlayerId, Code);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::EnableMfaAuthenticator(const FUniqueNetId& UserId, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable MFA Authenticator for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to enable MFA Authenticator for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteEnableMfaAuthenticator>(AccelByteSubsystemPtr.Get(), UserId, Code);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::DisableMfaAuthenticator(int32 LocalUserNum, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable MFA Authenticator for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable MFA Authenticator as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	DisableMfaAuthenticator(*UniquePlayerId, Code);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::DisableMfaAuthenticator(const FUniqueNetId& UserId, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable MFA Authenticator for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to disable MFA Authenticator for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDisableMfaAuthenticator>(AccelByteSubsystemPtr.Get(), UserId, Code);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::GenerateMfaAuthenticatorSecretKey(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't generate MFA authenticator secret key for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't generate MFA authenticator secret key as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	GenerateMfaAuthenticatorSecretKey(*UniquePlayerId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::GenerateMfaAuthenticatorSecretKey(const FUniqueNetId& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't generate MFA authenticator secret key for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate MFA authenticator secret key for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateMfaAuthenticatorSecretKey>(AccelByteSubsystemPtr.Get(), UserId);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::EnableMfaEmail(int32 LocalUserNum, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable mfa email for dedicated server."));
		return false;
	}

	if (Code.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable mfa email, verification code is empty."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable mfa email as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	EnableMfaEmail(*UniquePlayerId, Code);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::EnableMfaEmail(const FUniqueNetId& UserId, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable mfa email for dedicated server."));
		return false;
	}

	if (Code.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't enable mfa email, verification code is empty."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to enable mfa email for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteEnableMfaEmail>(AccelByteSubsystemPtr.Get(), UserId, Code);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::DisableMfaEmail(int32 LocalUserNum, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable mfa email for dedicated server."));
		return false;
	}

	if (Code.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable mfa email, verification code is empty."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable mfa email as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	DisableMfaEmail(*UniquePlayerId, Code);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::DisableMfaEmail(const FUniqueNetId& UserId, const FString& Code)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable mfa email for dedicated server."));
		return false;
	}

	if (Code.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't disable mfa email, verification code is empty."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to disable mfa email for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDisableMfaEmail>(AccelByteSubsystemPtr.Get(), UserId, Code);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::SendMfaCodeToEmail(int32 LocalUserNum, const EAccelByteSendMfaEmailAction Action)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't send MFA code to email for dedicated server."));
		return false;
	}

	const FUniqueNetIdPtr UniquePlayerId = GetUniquePlayerId(LocalUserNum);

	if (!UniquePlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't send MFA code to email as UniquePlayerId for local user num %d is invalid."), LocalUserNum);
		return false;
	}

	SendMfaCodeToEmail(*UniquePlayerId, Action);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineIdentityAccelByte::SendMfaCodeToEmail(const FUniqueNetId& UserId, const EAccelByteSendMfaEmailAction Action)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't send MFA code to email for dedicated server."));
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send MFA code to email for user as our internal subsystem pointer was invalid."));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendMfaCodeToEmail>(AccelByteSubsystemPtr.Get(), UserId, Action);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

/** End FOnlineIdentityAccelByte */
