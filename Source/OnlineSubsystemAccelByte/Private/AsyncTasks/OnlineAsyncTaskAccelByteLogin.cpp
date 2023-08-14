// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLogin.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Core/AccelByteUtilities.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"

// According to https://demo.accelbyte.io/basic/apidocs/#/UserProfile/getMyProfileInfo, 11440 is "user profile not found"
// with this in mind, if we get this code back, we just want to create the user profile with default values, as this
// most likely means the user hasn't signed in yet
#define USER_PROFILE_NOT_FOUND_ERROR 11440

// in ms
#define STEAM_LOGIN_DELAY 2000

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLogin"

FOnlineAsyncTaskAccelByteLogin::FOnlineAsyncTaskAccelByteLogin(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FOnlineAccountCredentials& InAccountCredentials)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
#if (PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC) && !UE_SERVER
	, OnGetAuthSessionTicketResponseCallback(this, &FOnlineAsyncTaskAccelByteLogin::OnGetAuthSessionTicketResponse)
#endif
	, LoginUserNum(InLocalUserNum)
	, AccountCredentials(InAccountCredentials)
{
	LocalUserNum = INVALID_CONTROLLERID;
}

void FOnlineAsyncTaskAccelByteLogin::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LoginUserNum);
	if (Subsystem->IsMultipleLocalUsersEnabled())
	{
		ApiClient = FMultiRegistry::GetApiClient(FString::Printf(TEXT("%d"), LoginUserNum));
	}
	else
	{
		ApiClient = FMultiRegistry::GetApiClient();
	}
	ApiClient->CredentialsRef->SetClientCredentials(FRegistry::Settings.ClientId, FRegistry::Settings.ClientSecret);

	// If all members of the AccountCredentials struct are blank, then this should be treated as a pass through login to
	// the native subsystem, if one is set. Intended for headless auth with consoles.
	if (AccountCredentials.Type.IsEmpty() && AccountCredentials.Id.IsEmpty() && AccountCredentials.Token.IsEmpty())
	{
		LoginWithNativeSubsystem();
		return;
	}

	if (!AccountCredentials.Type.IsEmpty())
	{
		// If we have something set for our credentials type, then try and parse the type and use that for login
		const UEnum* LoginTypeEnum = StaticEnum<EAccelByteLoginType>();
		const int64 Result = LoginTypeEnum->GetValueByNameString(AccountCredentials.Type, EGetByNameFlags::None);

		// Check whether our enum value search was a valid result and if not, throw an error and fail the task
		if (Result == INDEX_NONE || Result >= UINT8_MAX)
		{
			ErrorStr = TEXT("login-failed-invalid-type");
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to login with type '%s' as it was invalid!"), *AccountCredentials.Type);
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}
		
		LoginType = static_cast<EAccelByteLoginType>(Result);
	}
	else
	{
		// Otherwise, if we have a blank type in our credentials, just default to using AccelByte email/password
		LoginType = EAccelByteLoginType::AccelByte;
		UE_LOG_AB(Log, TEXT("Login type passed to Login call was blank - using AccelByte email/password type as default!"));
	}

	const FString LauncherCode = FAccelByteUtilities::GetAuthorizationCode();
	if (!LauncherCode.IsEmpty())
	{
		LoginType = EAccelByteLoginType::Launcher;
	}

	PerformLogin(AccountCredentials);	

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Trying to login with type '%s'!"), *AccountCredentials.Type);
}

void FOnlineAsyncTaskAccelByteLogin::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Subsystem->SetLocalUserNumCached(LoginUserNum);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLogin::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	const TSharedRef<const FUniqueNetIdAccelByteUser> ReturnId = (bWasSuccessful) ? UserId.ToSharedRef() : FUniqueNetIdAccelByteUser::Invalid();
	if (IdentityInterface.IsValid())
	{
		IdentityInterface->SetLoginStatus(LoginUserNum, bWasSuccessful ? ELoginStatus::LoggedIn : ELoginStatus::NotLoggedIn);
		if (bWasSuccessful && !ApiClient->CredentialsRef->IsComply())
		{
			const FOnlineAgreementAccelBytePtr AgreementInt = Subsystem->GetAgreementInterface();
			if (ensure(AgreementInt.IsValid()))
			{
				AgreementInt->TriggerOnUserNotCompliedDelegates(LoginUserNum);
			}
			else
			{
				// Notifying identity interface that this login failed because our account has not accepted legal agreements
				// Normally, this would not be a failure, but because the agreement interface is invalid and the player thus cannot
				// ever accept the agreement, we want to at least gracefully error out.
				ErrorStr = TEXT("login-failed-agreement-not-accepted");
				IdentityInterface->TriggerOnLoginCompleteDelegates(LoginUserNum, false, ReturnId.Get(), ErrorStr);
				ErrorOAuthObject.Error = ErrorStr;
				ErrorOAuthObject.ErrorMessage= ErrorStr;
				ErrorOAuthObject.Error_description = ErrorStr;
				IdentityInterface->TriggerOnLoginWithOAuthErrorCompleteDelegates(LoginUserNum, false, ReturnId.Get(), ErrorOAuthObject);
				IdentityInterface->TriggerAccelByteOnLoginCompleteDelegates(LoginUserNum, bWasSuccessful, ReturnId.Get(), ONLINE_ERROR_ACCELBYTE(ErrorStr));
			}
		}
		else
		{
			IdentityInterface->TriggerOnLoginCompleteDelegates(LoginUserNum, bWasSuccessful, ReturnId.Get(), ErrorStr);
			IdentityInterface->TriggerOnLoginWithOAuthErrorCompleteDelegates(LoginUserNum, bWasSuccessful, ReturnId.Get(), ErrorOAuthObject);
			IdentityInterface->TriggerAccelByteOnLoginCompleteDelegates(LoginUserNum, bWasSuccessful, ReturnId.Get(), ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorStr), bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLogin::LoginWithNativeSubsystem()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot login with native subsystem as none was set!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Check whether we officially support login with this OSS, otherwise subsequent calls will fail
	if (!Subsystem->IsNativeSubsystemSupported(NativeSubsystem->GetSubsystemName()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Native subsystem is not supported by AccelByte OSS for passthrough authentication! Subsystem name: %s"), *NativeSubsystem->GetSubsystemName().ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const IOnlineIdentityPtr NativeIdentityInterface = NativeSubsystem->GetIdentityInterface();
	if (!NativeIdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not retrieve identity interface from native subsystem."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// If the native subsystem reports not logged in, try and open the native login UI first. Native OSSes for platforms
	// like GDK will report NotLoggedIn if there is not a user logged in on the particular controller index.
	bool bLoginUIOpened = false;
	const ELoginStatus::Type NativeLoginStatus = NativeIdentityInterface->GetLoginStatus(LoginUserNum);
	if (NativeLoginStatus != ELoginStatus::LoggedIn)
	{
		const IOnlineExternalUIPtr NativeExternalUI = NativeSubsystem->GetExternalUIInterface();
		if (NativeExternalUI.IsValid() && !bRetryLoginSkipExternalUI)
		{
			FOnLoginUIClosedDelegate LoginUIClosed = TDelegateUtils<FOnLoginUIClosedDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnNativeLoginUIClosed);
			bLoginUIOpened = NativeExternalUI->ShowLoginUI(LoginUserNum, true, false, LoginUIClosed);
		}
	}

	// If we successfully opened the system UI for logging in on the native OSS, then we just want to bail out and allow the
	// handler for the native OSS to take over from here
	if (bLoginUIOpened && !bRetryLoginSkipExternalUI)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Login UI opened for native platform, allowing user to select account to log in to."));
		return;
	}
	
	// If we don't have a log in UI, then just send off a request to log in with blank credentials (default native account)
	FOnLoginCompleteDelegate NativeLoginComplete = TDelegateUtils<FOnLoginCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnNativeLoginComplete);
	NativeIdentityInterface->AddOnLoginCompleteDelegate_Handle(LoginUserNum, NativeLoginComplete);
	
	// Add Type credential parameter for EOS.
	FOnlineAccountCredentials AccountCredential;
	if (NativeSubsystem->GetSubsystemName().ToString().Equals(TEXT("EOS"), ESearchCase::IgnoreCase))
	{
		FAccelByteUtilities::GetValueFromCommandLineSwitch(TEXT("AUTH_TYPE"), AccountCredential.Type);
		FAccelByteUtilities::GetValueFromCommandLineSwitch(TEXT("AUTH_LOGIN"), AccountCredential.Id);
		FAccelByteUtilities::GetValueFromCommandLineSwitch(TEXT("AUTH_PASSWORD"), AccountCredential.Token);
	}

	NativeIdentityInterface->Login(LoginUserNum, AccountCredential);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending login request to native subsystem!"));
}

void FOnlineAsyncTaskAccelByteLogin::OnNativeLoginUIClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex, const FOnlineError& NativeError)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UniqueId: %s; ControllerIndex: %d; Error: %s"), ((UniqueId != nullptr) ? *UniqueId->ToString() : TEXT("nullptr")), ControllerIndex, *NativeError.ErrorMessage.ToString());

	// Check whether the unique ID instance from the native subsystem is valid, both in its instance as well as its contents
	if (!UniqueId.IsValid() || !UniqueId->IsValid())
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed. Error: %s"), *NativeError.ErrorRaw);
		if(bRetryLoginSkipExternalUI)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			return;
		}
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Log, TEXT("Retry logging in without External UI"));
		bRetryLoginSkipExternalUI = true;
		LoginWithNativeSubsystem();
		return;
	}

	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as the native subsystem could not be found!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const IOnlineIdentityPtr NativeIdentityInterface = NativeSubsystem->GetIdentityInterface();
	if (!NativeIdentityInterface.IsValid())
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as the native identity interface instance was invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnLoginCompleteDelegate NativeLoginComplete = TDelegateUtils<FOnLoginCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnNativeLoginComplete);
	NativeIdentityInterface->AddOnLoginCompleteDelegate_Handle(ControllerIndex, NativeLoginComplete);
	
	// Add Type credential parameter for EOS.
	if(NativeSubsystem->GetSubsystemName().ToString().Equals(TEXT("EOS"),ESearchCase::IgnoreCase))
	{
		if(AccountCredentials.Type.IsEmpty())
		{
			AccountCredentials.Type = TEXT("AccountPortal");
		}
		NativeIdentityInterface->Login(ControllerIndex, AccountCredentials);
	}
	else
	{
		NativeIdentityInterface->Login(ControllerIndex, FOnlineAccountCredentials());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending login request to native subsystem!"));
}

void FOnlineAsyncTaskAccelByteLogin::OnNativeLoginComplete(int32 NativeLocalUserNum, bool bWasNativeLoginSuccessful, const FUniqueNetId& NativeUserId, const FString& NativeError)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("NativeLocalUserNum: %d; bWasSuccessful: %s; UserId: %s"), NativeLocalUserNum, LOG_BOOL_FORMAT(bWasNativeLoginSuccessful), *NativeUserId.ToString());

	if (!bWasNativeLoginSuccessful)
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed. Native Error: %s"), *NativeError);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as the subsystem instance was nullptr!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	const IOnlineIdentityPtr NativeIdentityInterface = NativeSubsystem->GetIdentityInterface();
	if (!NativeIdentityInterface.IsValid())
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as the native identity interface instance was nullptr!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Clear the delegate for our login as it will be invalid once this task ends
	NativeIdentityInterface->ClearOnLoginCompleteDelegates(LoginUserNum, this);

	// Set the login type for this request to be the login type corresponding to the native subsystem
	const UEnum* LoginTypeEnum = StaticEnum<EAccelByteLoginType>();
	LoginType = FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(NativeSubsystem->GetSubsystemName());
	if (LoginType == EAccelByteLoginType::None)
	{
		ErrorStr = TEXT("login-failed-invalid-type");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as an invalid type was provided for the native subsystem. AccelByte subsystem may not support login with this subsystem type."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	bLoginPerformed = false;
	// Store the unique ID of the user that we are authenticating with
	NativePlatformPlayerId = NativeUserId.AsShared();

	// Construct credentials for the login type from the native subsystem, complete with the type set correctly to its name
	NativePlatformCredentials.Type = LoginTypeEnum->GetNameStringByValue(static_cast<int64>(LoginType));
	NativePlatformCredentials.Token = FGenericPlatformHttp::UrlEncode(NativeIdentityInterface->GetAuthToken(LoginUserNum));

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this, NativeIdentityInterface]()
	{
		if (!bLoginPerformed)
		{
			PerformLogin(NativePlatformCredentials);
			bLoginPerformed = true;
		}
	});

	if (NativeSubsystem->GetSubsystemName().ToString().Equals(TEXT("STEAM"), ESearchCase::IgnoreCase))
	{
		TimerObject.StartIn(STEAM_LOGIN_DELAY, TimerDelegate);
	}
	else
	{
		TimerDelegate.ExecuteIfBound();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending login request to AccelByte backend for native user!"));
}

#if (PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC) && !UE_SERVER
void FOnlineAsyncTaskAccelByteLogin::OnGetAuthSessionTicketResponse(GetAuthSessionTicketResponse_t* CallBackParam)
{
	UE_LOG_AB(Log, TEXT("Get Auth Session Ticket Response, Result %d"), CallBackParam->m_eResult);
	if (!bLoginPerformed)
	{
		TimerObject.Stop();
		PerformLogin(NativePlatformCredentials);
		bLoginPerformed = true;
	}
}
#endif

void FOnlineAsyncTaskAccelByteLogin::PerformLogin(const FOnlineAccountCredentials& Credentials)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginType: %s"), *Credentials.Type);

	const AccelByte::FVoidHandler OnLoginSuccessDelegate = TDelegateUtils<AccelByte::FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnLoginSuccess);
	const AccelByte::FOAuthErrorHandler OnLoginErrorOAuthDelegate = TDelegateUtils<AccelByte::FOAuthErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnLoginErrorOAuth);
	switch (LoginType)
	{
	case EAccelByteLoginType::DeviceId:
		ApiClient->User.LoginWithDeviceId(OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with Device ID."));
		break;
	case EAccelByteLoginType::AccelByte:
		ApiClient->User.LoginWithUsernameV3(Credentials.Id, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with AccelByte credentials."));
		break;
	case EAccelByteLoginType::Xbox:
		ApiClient->User.LoginWithOtherPlatform(EAccelBytePlatformType::Live, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with XSTS token from native online subsystem."));
		break;
	case EAccelByteLoginType::PS4:
		ApiClient->User.LoginWithOtherPlatform(EAccelBytePlatformType::PS4, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with PS4 auth token from native online subsystem."));
		break;
	case EAccelByteLoginType::PS5:
		ApiClient->User.LoginWithOtherPlatform(EAccelBytePlatformType::PS5, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with PS5 auth token from native online subsystem."));
		break;
	case EAccelByteLoginType::Launcher:
		ApiClient->User.LoginWithLauncher(OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with AccelByte launcher auth token."));
		break;
	case EAccelByteLoginType::Steam:
		ApiClient->User.LoginWithOtherPlatform(EAccelBytePlatformType::Steam, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with Steam auth token from native online subsystem."));
		break;
	case EAccelByteLoginType::RefreshToken:
		ApiClient->User.LoginWithRefreshToken(Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with refresh token from native online subsystem."));
		break;
	case EAccelByteLoginType::EOS:
		ApiClient->User.LoginWithOtherPlatform(EAccelBytePlatformType::EpicGames, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with Epic Games from native online subsystem."));
		break;
	case EAccelByteLoginType::CachedToken:
		ApiClient->User.TryRelogin(Credentials.Id, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with cached refresh token for the specified PlatformUserID."));
		break;
	default:
	case EAccelByteLoginType::None:
		ErrorStr = TEXT("login-failed-invalid-type");
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Cannot login with type '%s' as it is not supported by our subsystem!"), *Credentials.Type);
		break;
	}
}

void FOnlineAsyncTaskAccelByteLogin::OnLoginSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Create a new user ID instance for the user that we just logged in as
	FAccelByteUniqueIdComposite CompositeId;
	CompositeId.Id = ApiClient->CredentialsRef->GetUserId();

	// Add platform information to composite ID for login
	if (NativePlatformPlayerId.IsValid())
	{
		CompositeId.PlatformType = NativePlatformPlayerId->GetType().ToString();
		CompositeId.PlatformId = NativePlatformPlayerId->ToString();
	}

	UserId = FUniqueNetIdAccelByteUser::Create(CompositeId);
	if (!UserId.IsValid())
	{
		return;
	}

	// Also create an account instance for them, this will be fed back to the identity interface after login
	Account = MakeShared<FUserOnlineAccountAccelByte>(UserId.ToSharedRef());
	Account->SetDisplayName(ApiClient->CredentialsRef->GetUserDisplayName());
	Account->SetAccessToken(ApiClient->CredentialsRef->GetAccessToken());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		return;
	}
		
	IdentityInterface->AddNewAuthenticatedUser(LoginUserNum, UserId.ToSharedRef(), Account.ToSharedRef());
	AccelByte::FMultiRegistry::RemoveApiClient(UserId->GetAccelByteId());
	AccelByte::FMultiRegistry::RegisterApiClient(UserId->GetAccelByteId(), ApiClient);
	AccelByte::FMultiRegistry::RemoveApiClient(FString::Printf(TEXT("%d"), LoginUserNum));

	// Grab our user interface and kick off a task to get information about the newly logged in player from it, namely
	// their avatar URL. No need to register a delegate to update the account from the query, the query task will check
	// if an account instance exists for the player in the identity interface, and if so will update it.
	FOnlineUserAccelBytePtr UserInterface = nullptr;
	if (!FOnlineUserAccelByte::GetFromSubsystem(Subsystem, UserInterface))
	{
		return;
	}

	// Bind delegate for Query UserProfile
	UserInterface->AddOnQueryUserProfileCompleteDelegate_Handle(LoginUserNum, FOnQueryUserProfileCompleteDelegate::CreateThreadSafeSP(UserInterface.Get(), &FOnlineUserAccelByte::PostLoginBulkGetUserProfileCompleted));
	UserInterface->QueryUserInfo(LoginUserNum, { UserId.ToSharedRef() });

	// If we are using V2 sessions, send a request to update stored platform data in the session service for native sync and crossplay
#if AB_USE_V2_SESSIONS
	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface))
	{
		SessionInterface->InitializePlayerAttributes(UserId.ToSharedRef().Get());	
	}
#endif

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLogin::OnLoginErrorOAuth(int32 InErrorCode, const FString& ErrorMessage, const FErrorOAuthInfo& ErrorObject)
{
	UE_LOG_AB(Warning, TEXT("Failed to login to the AccelByte backend! Error Code: %d; Error Message: %s"), InErrorCode, *ErrorMessage);
	ErrorCode = InErrorCode;
	ErrorOAuthObject = ErrorObject;
	ErrorStr = TEXT("login-failed-connect");
	if (ErrorCode == 400 || ErrorCode == 403) // Permanent ban
	{
		if (ErrorObject.UserBan.Reason != EBanReason::EMPTY)
		{
			ErrorStr = TEXT("user-banned");
		}
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE