// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLogin.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Core/AccelByteUtilities.h"
#include "Models/AccelBytePredefinedEventModels.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "LoginQueue/OnlineAsyncTaskAccelByteLoginQueue.h"

using namespace AccelByte;

// According to https://demo.accelbyte.io/basic/apidocs/#/UserProfile/getMyProfileInfo, 11440 is "user profile not found"
// with this in mind, if we get this code back, we just want to create the user profile with default values, as this
// most likely means the user hasn't signed in yet
#define USER_PROFILE_NOT_FOUND_ERROR 11440

// in ms
#define STEAM_LOGIN_DELAY 5000

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteLogin"

FOnlineAsyncTaskAccelByteLogin::FOnlineAsyncTaskAccelByteLogin(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FOnlineAccountCredentials& InAccountCredentials
	, bool bInCreateHeadlessAccount)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
#if (PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC) && !UE_SERVER
	, OnGetAuthSessionTicketResponseCallback(this, &FOnlineAsyncTaskAccelByteLogin::OnGetAuthSessionTicketResponse)
#endif
	, LoginUserNum(InLocalUserNum)
	, AccountCredentials(InAccountCredentials)
	, bCreateHeadlessAccount(bInCreateHeadlessAccount)
{
	LocalUserNum = INVALID_CONTROLLERID;
}

void FOnlineAsyncTaskAccelByteLogin::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LoginUserNum);
	
	GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("LoginQueuePresentationThreshold"), LoginQueuePresentationThreshold, GEngineIni);

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
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (bWasSuccessful)
	{
		FAccelByteModelsLoginSuccededPayload LoginSuccededPayload{};
		LoginSuccededPayload.Namespace = ApiClient->CredentialsRef->GetNamespace();
		LoginSuccededPayload.UserId = ApiClient->CredentialsRef->GetUserId();
		LoginSuccededPayload.PlatformUserId = ApiClient->CredentialsRef->GetPlatformUserId();
		LoginSuccededPayload.PlatformId = PlatformId;
		LoginSuccededPayload.DeviceId = FAccelByteUtilities::GetDeviceId();
		PredefinedEventInterface->SendEvent(LoginUserNum, MakeShared<FAccelByteModelsLoginSuccededPayload>(LoginSuccededPayload));
	}
	else
	{
		FAccelByteModelsLoginFailedPayload LoginFailedPayload{};
		LoginFailedPayload.Namespace = ApiClient->CredentialsRef->GetNamespace();
		LoginFailedPayload.PlatformId = PlatformId;
		PredefinedEventInterface->SendEvent(LoginUserNum, MakeShared<FAccelByteModelsLoginFailedPayload>(LoginFailedPayload));
	}

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
	LoginWithSpecificSubsystem(IOnlineSubsystem::GetByPlatform());
}

//Used by child asynctask (SimultaneousLogin)
void FOnlineAsyncTaskAccelByteLogin::LoginWithSpecificSubsystem(FString InSubsystemName)
{
	FName SubsystemName(InSubsystemName);
	LoginWithSpecificSubsystem(IOnlineSubsystem::Get(SubsystemName));
}

void FOnlineAsyncTaskAccelByteLogin::LoginWithSpecificSubsystem(IOnlineSubsystem* SpecificSubsystem)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (SpecificSubsystem == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot login with native subsystem as none was set!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Check whether we officially support login with this OSS, otherwise subsequent calls will fail
	if (!Subsystem->IsNativeSubsystemSupported(SpecificSubsystem->GetSubsystemName()) && !bByPassSupportedNativeSubsystemCheck)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Native subsystem is not supported by AccelByte OSS for passthrough authentication! Subsystem name: %s"), *SpecificSubsystem->GetSubsystemName().ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = SpecificSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not retrieve identity interface from native subsystem."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// If the native subsystem reports not logged in, try and open the native login UI first. Native OSSes for platforms
	// like GDK will report NotLoggedIn if there is not a user logged in on the particular controller index.
	bool bLoginUIOpened = false;
	const ELoginStatus::Type SubsystemLoginStatus = IdentityInterface->GetLoginStatus(LoginUserNum);
	if (SubsystemLoginStatus != ELoginStatus::LoggedIn)
	{
		const IOnlineExternalUIPtr NativeExternalUI = SpecificSubsystem->GetExternalUIInterface();
		if (NativeExternalUI.IsValid() && !bRetryLoginSkipExternalUI)
		{
			FOnLoginUIClosedDelegate LoginUIClosed = TDelegateUtils<FOnLoginUIClosedDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnSpecificSubysystemLoginUIClosed, SpecificSubsystem);
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
	FOnLoginCompleteDelegate IdentityLoginComplete = TDelegateUtils<FOnLoginCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnSpecificSubysystemLoginComplete, SpecificSubsystem);
	IdentityInterface->AddOnLoginCompleteDelegate_Handle(LoginUserNum, IdentityLoginComplete);
	
	// Add Type credential parameter for EOS.
	FOnlineAccountCredentials AccountCredential;
	if (SpecificSubsystem->GetSubsystemName().ToString().Equals(TEXT("EOS"), ESearchCase::IgnoreCase))
	{
		FAccelByteUtilities::GetValueFromCommandLineSwitch(TEXT("AUTH_TYPE"), AccountCredential.Type);
		FAccelByteUtilities::GetValueFromCommandLineSwitch(TEXT("AUTH_LOGIN"), AccountCredential.Id);
		FAccelByteUtilities::GetValueFromCommandLineSwitch(TEXT("AUTH_PASSWORD"), AccountCredential.Token);
	}

	this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, AccountCredential, IdentityInterface]()
		{
			IdentityInterface->Login(this->LoginUserNum, AccountCredential);
		}));
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending login request to native subsystem!"));
}

void FOnlineAsyncTaskAccelByteLogin::OnSpecificSubysystemLoginUIClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex, const FOnlineError& SubsystemError, IOnlineSubsystem* SpecificSubsystem)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UniqueId: %s; ControllerIndex: %d; Error: %s"), ((UniqueId != nullptr) ? *UniqueId->ToString() : TEXT("nullptr")), ControllerIndex, *SubsystemError.ErrorMessage.ToString());
	FString SpecifiedSubsystemName = *SpecificSubsystem->GetSubsystemName().ToString();
	const FString ErrorStrIfPrerequisiteNotMet = FString::Printf(TEXT("login-failed-%s-subsystem"), *SpecifiedSubsystemName);
	const FString ErrorVerbosityReasonIfPrerequisteNotMet = FString::Printf(TEXT("Login with %s subsystem failed as the subsystem's identity interface instance was invalid!"), *SpecifiedSubsystemName);

	// Check whether the unique ID instance from the specified subsystem is valid, both in its instance as well as its contents
	if (!UniqueId.IsValid() || !UniqueId->IsValid())
	{
		ErrorStr = ErrorStrIfPrerequisiteNotMet;
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorVerbosityReasonIfPrerequisteNotMet);
		if(bRetryLoginSkipExternalUI)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			return;
		}
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Log, TEXT("Retry logging in without External UI"));
		bRetryLoginSkipExternalUI = true;
		LoginWithSpecificSubsystem(SpecificSubsystem);
		return;
	}

	if (SpecificSubsystem == nullptr)
	{
		ErrorStr = ErrorStrIfPrerequisiteNotMet;
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorVerbosityReasonIfPrerequisteNotMet);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = SpecificSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		ErrorStr = ErrorVerbosityReasonIfPrerequisteNotMet;
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorVerbosityReasonIfPrerequisteNotMet);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnLoginCompleteDelegate IdentityLoginComplete = TDelegateUtils<FOnLoginCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnSpecificSubysystemLoginComplete, SpecificSubsystem);
	IdentityInterface->AddOnLoginCompleteDelegate_Handle(ControllerIndex, IdentityLoginComplete);
	
	// Add Type credential parameter for EOS.
	if(SpecificSubsystem->GetSubsystemName().ToString().Equals(TEXT("EOS"),ESearchCase::IgnoreCase))
	{
		if(AccountCredentials.Type.IsEmpty())
		{
			AccountCredentials.Type = TEXT("AccountPortal");
		}
		this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, IdentityInterface, ControllerIndex]()
			{
				IdentityInterface->Login(ControllerIndex, this->AccountCredentials);
			}));
	}
	else
	{
		this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, IdentityInterface, ControllerIndex]()
			{
				IdentityInterface->Login(ControllerIndex, FOnlineAccountCredentials());
			}));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending login request to specific subsystem!"));
}

void FOnlineAsyncTaskAccelByteLogin::OnSpecificSubysystemLoginComplete(int32 NativeLocalUserNum, bool bWasNativeLoginSuccessful, const FUniqueNetId& NativeUserId, const FString& NativeError, IOnlineSubsystem* SpecificSubsystem)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("NativeLocalUserNum: %d; bWasSuccessful: %s; UserId: %s"), NativeLocalUserNum, LOG_BOOL_FORMAT(bWasNativeLoginSuccessful), *NativeUserId.ToDebugString());

	if (!bWasNativeLoginSuccessful)
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed. Native Error: %s"), *NativeError);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	if (SpecificSubsystem == nullptr)
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as the subsystem instance was nullptr!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	const IOnlineIdentityPtr IdentityInterface = SpecificSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		ErrorStr = TEXT("login-failed-native-subsystem");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as the native identity interface instance was nullptr!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Clear the delegate for our login as it will be invalid once this task ends
	IdentityInterface->ClearOnLoginCompleteDelegates(LoginUserNum, this);

	// Set the login type for this request to be the login type corresponding to the native subsystem
	const UEnum* LoginTypeEnum = StaticEnum<EAccelByteLoginType>();
	LoginType = FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(SpecificSubsystem->GetSubsystemName());
	if (LoginType == EAccelByteLoginType::None && !bByPassSupportedNativeSubsystemCheck)
	{
		ErrorStr = TEXT("login-failed-invalid-type");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Login with native subsystem failed as an invalid type was provided for the native subsystem. AccelByte subsystem may not support login with this subsystem type."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	bLoginPerformed = false;
	FString PlatformToken = FGenericPlatformHttp::UrlEncode(IdentityInterface->GetAuthToken(LoginUserNum));
	
	FOnlineAccountCredentials CopyCreds{};
	CopyCreds.Id = NativePlatformCredentials.Id;//Other Subsystem doesn't rely much to the Id
	if (LoginType == EAccelByteLoginType::None && bByPassSupportedNativeSubsystemCheck)
	{
		CopyCreds.Type = SpecificSubsystem->GetSubsystemName().ToString();
	}
	else
	{
		CopyCreds.Type = LoginTypeEnum->GetNameStringByValue(static_cast<int64>(LoginType));
	}
	CopyCreds.Token = PlatformToken;

	if (bStoreNativePlatformCredentialOnSubsystemLoginComplete)
	{
		// Store the unique ID of the user that we are authenticating with
		NativePlatformPlayerId = NativeUserId.AsShared();

		// Construct credentials for the login type from the native subsystem, complete with the type set correctly to its name
		NativePlatformCredentials.Type = LoginTypeEnum->GetNameStringByValue(static_cast<int64>(LoginType));
		NativePlatformCredentials.Token = PlatformToken;
	}

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this, CopyCreds]()
	{
		if (!bLoginPerformed)
		{
			PerformLogin(CopyCreds);
			bLoginPerformed = true;
		}
	});

	if (SpecificSubsystem->GetSubsystemName().ToString().Equals(TEXT("STEAM"), ESearchCase::IgnoreCase))
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
	if (!bLoginPerformed && LoginType == EAccelByteLoginType::Steam)
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

	const AccelByte::THandler<FAccelByteModelsLoginQueueTicketInfo> OnLoginSuccessDelegate = TDelegateUtils<AccelByte::THandler<FAccelByteModelsLoginQueueTicketInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnLoginSuccessV4);
	const AccelByte::FOAuthErrorHandler OnLoginErrorOAuthDelegate = TDelegateUtils<AccelByte::FOAuthErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLogin::OnLoginErrorOAuth);
	switch (LoginType)
	{
	case EAccelByteLoginType::DeviceId:
		ApiClient->User.LoginWithDeviceIdV4(OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, bCreateHeadlessAccount);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with Device ID."));
		PlatformId = FAccelByteUtilities::GetUEnumValueAsString(EAccelBytePlatformType::Device);
		break;
	case EAccelByteLoginType::AccelByte:
		ApiClient->User.LoginWithUsernameV4(Credentials.Id, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with AccelByte credentials."));
		break;
	case EAccelByteLoginType::Xbox:
		ApiClient->User.LoginWithOtherPlatformV4(EAccelBytePlatformType::Live, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, bCreateHeadlessAccount);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with XSTS token from native online subsystem."));
		PlatformId = FAccelByteUtilities::GetUEnumValueAsString(EAccelBytePlatformType::Live);
		break;
	case EAccelByteLoginType::PS4:
		ApiClient->User.LoginWithOtherPlatformV4(EAccelBytePlatformType::PS4, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, bCreateHeadlessAccount);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with PS4 auth token from native online subsystem."));
		PlatformId = FAccelByteUtilities::GetUEnumValueAsString(EAccelBytePlatformType::PS4);
		break;
	case EAccelByteLoginType::PS5:
		ApiClient->User.LoginWithOtherPlatformV4(EAccelBytePlatformType::PS5, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, bCreateHeadlessAccount);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with PS5 auth token from native online subsystem."));
		PlatformId = FAccelByteUtilities::GetUEnumValueAsString(EAccelBytePlatformType::PS5);
		break;
	case EAccelByteLoginType::Launcher:
		ApiClient->User.LoginWithLauncherV4(OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with AccelByte launcher auth token."));
		break;
	case EAccelByteLoginType::PublisherCode:
	{
		// Allow developer to override the token code if there's another way to obtain it
		FString Code = Credentials.Token;

		// If the code is not specified, obtain the code from the default mechanism
		if (Code.IsEmpty())
		{
			Code = FAccelByteUtilities::GetAuthorizationCode();
		}

		// If the code is not found, it means the game is not launched by launcher
		if (Code.IsEmpty())
		{
			ErrorStr = TEXT("login-failed-request-incomplete");
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Cannot login with type '%s' as it require AuthorizationGameCode from launcher!"), *Credentials.Type);
			break;
		}

		ApiClient->User.GenerateGameTokenV4(Code, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with AccelByte launcher auth token."));
		break;
	}
	case EAccelByteLoginType::Steam:
		ApiClient->User.LoginWithOtherPlatformV4(EAccelBytePlatformType::Steam, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, bCreateHeadlessAccount);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with Steam auth token from native online subsystem."));
		PlatformId = FAccelByteUtilities::GetUEnumValueAsString(EAccelBytePlatformType::Steam);
		break;
	case EAccelByteLoginType::RefreshToken:
		ApiClient->User.LoginWithRefreshTokenV4(OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, Credentials.Token);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with refresh token from native online subsystem."));
		break;
	case EAccelByteLoginType::EOS:
		ApiClient->User.LoginWithOtherPlatformV4(EAccelBytePlatformType::EpicGames, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, bCreateHeadlessAccount);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with Epic Games from native online subsystem."));
		PlatformId = FAccelByteUtilities::GetUEnumValueAsString(EAccelBytePlatformType::EpicGames);
		break;
	case EAccelByteLoginType::CachedToken:
		ApiClient->User.TryReloginV4(Credentials.Id, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with cached refresh token for the specified PlatformUserID."));
		break;
	case EAccelByteLoginType::OIDC:
		ApiClient->User.LoginWithOtherPlatformIdV4(Credentials.Id, Credentials.Token, OnLoginSuccessDelegate, OnLoginErrorOAuthDelegate, bCreateHeadlessAccount);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending async task to login with OIDC for Id %s."), *Credentials.Id);
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
	Account->SetUserCountry(ApiClient->CredentialsRef->GetAccountUserData().Country);
	Account->SetDisplayName(ApiClient->CredentialsRef->GetUserDisplayName());
	Account->SetAccessToken(ApiClient->CredentialsRef->GetAccessToken());
	Account->SetPlatformUserId(ApiClient->CredentialsRef->GetPlatformUserId());
	Account->SetUniqueDisplayName(ApiClient->CredentialsRef->GetUniqueDisplayName());
	
	// Retrieve the platform user information array from the account user data.
	const TArray<FAccountUserPlatformInfo>& PlatformInfos = ApiClient->CredentialsRef->GetAccountUserData().PlatformInfos;

	// Iterate over platform user information retrieved from the account data.
	for (const FAccountUserPlatformInfo& Info : PlatformInfos)
	{
		FOnlinePlatformUserAccelByte localPlatformUser;
		localPlatformUser.SetDisplayName(Info.PlatformDisplayName);
		localPlatformUser.SetAvatarUrl(Info.PlatformAvatarUrl);
		localPlatformUser.SetPlatformId(Info.PlatformId);
		localPlatformUser.SetPlatformGroup(Info.PlatformGroup);
		localPlatformUser.SetPlatformUserId(Info.PlatformUserId);

		// Prepare the linked platform user information for caching.
		FAccelByteLinkedUserInfo LinkedUserInfo;
		TArray<FAccelByteLinkedUserInfo> LinkedPlatformUsers;

		LinkedUserInfo.DisplayName = Info.PlatformDisplayName;
		LinkedUserInfo.PlatformId = Info.PlatformId;
		LinkedUserInfo.AvatarUrl = Info.PlatformAvatarUrl;
		LinkedPlatformUsers.Add(LinkedUserInfo);

		// Add each platform user to the related account.
		Account->AddPlatformUser(localPlatformUser);

		// Ensure that UserCache is valid before attempting to use it, update the user cache with the linked platform user information.
		if (UserCache.IsValid())
		{
			UserCache->AddLinkedPlatformInfoToCache(*UserId.Get(), LinkedPlatformUsers); 
		}
	}

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

void FOnlineAsyncTaskAccelByteLogin::OnLoginSuccessV4(const FAccelByteModelsLoginQueueTicketInfo& TicketInfo)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Login v4 done, LoginUserNum: %d, TicketId: %s"), LoginUserNum, *TicketInfo.Ticket);

	if(TicketInfo.Ticket.IsEmpty())
	{
		OnLoginSuccess();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to queue login ticket, Identity interface is invalid!"));
		return;
	}

	// Only trigger login queued delegate if estimated waiting time is above presentation threshold
	if(TicketInfo.EstimatedWaitingTimeInSeconds > LoginQueuePresentationThreshold)
	{
		IdentityInterface->TriggerAccelByteOnLoginQueuedDelegates(LoginUserNum, TicketInfo);
	}
	
	bShouldUseTimeout = false;
	bLoginInQueue = true;
	this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, &TicketInfo]()
	{
		Subsystem->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteLoginQueue>(Subsystem, LoginUserNum, TicketInfo);
	}));
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLogin::Tick()
{
	Super::Tick();

	if(!bLoginInQueue)
	{
		return;
	}

	if(ApiClient->CredentialsRef->IsSessionValid())
	{
		OnLoginSuccess();
	}
}

void FOnlineAsyncTaskAccelByteLogin::OnLoginErrorOAuth(int32 InErrorCode, const FString& ErrorMessage, const FErrorOAuthInfo& ErrorObject)
{
	UE_LOG_AB(Warning, TEXT("Failed to login to the AccelByte backend! Error Code: %d; Error Message: %s"), InErrorCode, *ErrorMessage);
	ErrorCode = InErrorCode;
	ErrorOAuthObject = ErrorObject; 
	ErrorStr = ErrorMessage;
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