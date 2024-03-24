// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSimultaneousLogin.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Core/AccelByteUtilities.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSimultaneousLogin"

FOnlineAsyncTaskAccelByteSimultaneousLogin::FOnlineAsyncTaskAccelByteSimultaneousLogin(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FOnlineAccountCredentials& InAccountCredentials
	, bool bInCreateHeadlessAccount)
	: FOnlineAsyncTaskAccelByteLogin(InABSubsystem, InLocalUserNum, InAccountCredentials, bInCreateHeadlessAccount)
{
	LocalUserNum = INVALID_CONTROLLERID;
	bByPassSupportedNativeSubsystemCheck = true;
}

bool FOnlineAsyncTaskAccelByteSimultaneousLogin::IsInitializeAllowed()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Checking Initialization"));

	if (!AccountCredentials.Type.Contains(FAccelByteUtilities::GetUEnumValueAsString(EAccelByteLoginType::Simultaneous)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to do simultaneous login but the type was invalid!"));
		return false;
	}

	if (SecondaryPlatformName.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to do simultaneous login, no secondary platform specified in the DefaultEngine.ini.\n Please set the value through\n[OnlineSubsystemAccelByte]\n SecondaryPlatformName =...."));
		return false; 
	}

	if (!Subsystem->IsNativeSubsystemSupported(FName(SecondaryPlatformName)) && !bByPassSupportedNativeSubsystemCheck)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to do simultaneous login, specified secondary platform is not supported (%s)"), *SecondaryPlatformName);
		return false;
	}

	if (IOnlineSubsystem::Get(FName(SecondaryPlatformName)) == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to do simultaneous login, specified secondary platform is not found in the engine (%s)"), *SecondaryPlatformName);
		return false;
	}

	if (!IOnlineSubsystem::Get(FName(SecondaryPlatformName))->IsLoaded())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to do simultaneous login, specified secondary platform is not loaded (%s)"), *SecondaryPlatformName);
		return false;
	}

	return true;
}

void FOnlineAsyncTaskAccelByteSimultaneousLogin::Initialize()
{
	Super::Initialize();
	CurrentAsyncTaskState = ESimultaneousLoginAsyncTaskState::Initialized;
	GConfig->GetString(TEXT("OnlineSubsystemAccelByte"), TEXT("SecondaryPlatformName"), SecondaryPlatformName, GEngineIni);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LoginUserNum);
	
	if (!IsInitializeAllowed())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to initialize!"));
		return;
	}

	if (Subsystem->IsMultipleLocalUsersEnabled())
	{
		ApiClient = FMultiRegistry::GetApiClient(FString::Printf(TEXT("%d"), LoginUserNum));
	}
	else
	{
		ApiClient = FMultiRegistry::GetApiClient();
	}
	ApiClient->CredentialsRef->SetClientCredentials(FRegistry::Settings.ClientId, FRegistry::Settings.ClientSecret);
	
	LoginWithNativeSubsystem();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Trying to login with type '%s'!"), *AccountCredentials.Type);
}

void FOnlineAsyncTaskAccelByteSimultaneousLogin::PostProcessTriggerDelegates()
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	const TSharedRef<const FUniqueNetIdAccelByteUser> ReturnId = (bWasSuccessful) ? UserId.ToSharedRef() : FUniqueNetIdAccelByteUser::Invalid();
	if (!IdentityInterface.IsValid())
	{
		//nothing else can be done
		return;
	}

	ESimultaneousLoginResult SimultaneousLoginResult{};
	
	if (!bWasSuccessful)
	{
		//Set the error string based on the last known state
		//Because ErrorStr from parent Login AsyncTask only being set when receiving response from OnLoginErrorOAuth
		switch (CurrentAsyncTaskState)
		{
		case ESimultaneousLoginAsyncTaskState::Initialized:
			ErrorStr = "Native platform login incomplete";
			SimultaneousLoginResult = ESimultaneousLoginResult::NativePlatformLoginFail;
			ErrorOAuthObject.ErrorCode = 400;
			ErrorOAuthObject.ErrorMessage = ErrorStr;
			break;
		case ESimultaneousLoginAsyncTaskState::NativePlatformLoginDone:
			ErrorStr = "Secondary platform login incomplete";
			SimultaneousLoginResult = ESimultaneousLoginResult::SecondaryPlatformLoginFail;
			ErrorOAuthObject.ErrorCode = 400;
			ErrorOAuthObject.ErrorMessage = ErrorStr;
			break;
		case ESimultaneousLoginAsyncTaskState::SecondaryPlatformLoginDone:
			ErrorStr = "Fail to execute simultaneous platform login or linking";
			SimultaneousLoginResult = ESimultaneousLoginResult::ConflictAccountLinking; //Even though have a higher chance it will be overriden by ErrorOAuthObject.ErrorCode 
			ErrorOAuthObject.ErrorCode = 400;
			ErrorOAuthObject.ErrorMessage = ErrorStr;
			break;
		default:
			break;
		}
		
		//HTTP Response Code
		switch (ErrorCode)
		{
		case 400:
			SimultaneousLoginResult = ESimultaneousLoginResult::ServiceMisconfiguration;
			break;
		case 401:
			SimultaneousLoginResult = ESimultaneousLoginResult::InvalidTicket;
			break;
		case 409:
			//IAM Error Code
			SimultaneousLoginResult = static_cast<ESimultaneousLoginResult>(ErrorOAuthObject.ErrorCode);
			ErrorOAuthObject.ErrorCode = 409;
			break;
		default:
			SimultaneousLoginResult = ESimultaneousLoginResult::Unknown;
			break;
		}
	}
	else
	{
		SimultaneousLoginResult = ESimultaneousLoginResult::Success;
	}

	IdentityInterface->TriggerAccelByteOnSimultaneousLoginCompleteDelegates(LoginUserNum, bWasSuccessful, SimultaneousLoginResult, ReturnId.Get(), ErrorOAuthObject);
}

void FOnlineAsyncTaskAccelByteSimultaneousLogin::TriggerDelegates()
{
	//Parent class'
	FOnlineAsyncTaskAccelByteLogin::TriggerDelegates();
	PostProcessTriggerDelegates();
}

// This perform login occurs twice because we triggers LoginWithNativeSubsystem + LoginWithSpecificSubsystem
void FOnlineAsyncTaskAccelByteSimultaneousLogin::PerformLogin(const FOnlineAccountCredentials& Credentials)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("First native platform login success: %s"), *Credentials.Type);
	if (CurrentAsyncTaskState == ESimultaneousLoginAsyncTaskState::Initialized)
	{
		CurrentAsyncTaskState = ESimultaneousLoginAsyncTaskState::NativePlatformLoginDone;
		NativePlatformTicket = Credentials.Token;

		//We SHOULD NOT store the secondary platform login result
		//Because it will override the native (previous) result
		bStoreNativePlatformCredentialOnSubsystemLoginComplete = false;
		LoginWithSpecificSubsystem(SecondaryPlatformName);
	}
	else if (CurrentAsyncTaskState == ESimultaneousLoginAsyncTaskState::NativePlatformLoginDone)
	{
		CurrentAsyncTaskState = ESimultaneousLoginAsyncTaskState::SecondaryPlatformLoginDone;
		SecondaryPlatformTicket = Credentials.Token;

		const THandler<FAccelByteModelsLoginQueueTicketInfo> OnLoginSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsLoginQueueTicketInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSimultaneousLogin::OnLoginSuccessV4);
		const FOAuthErrorHandler OnLoginErrorOAuthDelegate = TDelegateUtils<FOAuthErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSimultaneousLogin::OnLoginErrorOAuth);

		ApiClient->User.LoginWithSimultaneousPlatformV4(
			NativePlatformCredentials.Type.ToLower(),
			NativePlatformTicket,
			Credentials.Type.ToLower(),
			SecondaryPlatformTicket,
			OnLoginSuccessDelegate,
			OnLoginErrorOAuthDelegate);
		
		PlatformId = NativePlatformCredentials.Type;
	}
}

#undef ONLINE_ERROR_NAMESPACE