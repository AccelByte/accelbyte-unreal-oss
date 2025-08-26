// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRefreshPlatformToken.h" 

#include "OnlineSubsystemAccelByteUtils.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRefreshPlatformToken::FOnlineAsyncTaskAccelByteRefreshPlatformToken(
	FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, FName SubsystemName)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));
	LocalUserNum = InLocalUserNum;
	RefreshSubsystemName = SubsystemName;
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

bool FOnlineAsyncTaskAccelByteRefreshPlatformToken::CheckAndGetOnlineIdentityInterface(IOnlineIdentityPtr& OutIdentityInterface, EAccelBytePlatformType& OutPlatformType)
{
	const IOnlineSubsystem* TargetedSubsystem = IOnlineSubsystem::Get(RefreshSubsystemName);

	//Preliminary assume the current async task will fail
	this->ErrorMessage = TEXT("failed-refresh-platform-token");
	this->ErrorCode = static_cast<int32>(AccelByte::ErrorCodes::PlatformInternalServerErrorException);

	if (TargetedSubsystem == nullptr)
	{
		FString RefreshSubsystemNameAsString = RefreshSubsystemName.ToString();
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Refresh platform token failed as the online subsystem [%s] instance was nullptr!"), *RefreshSubsystemNameAsString);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Incomplete);
		return false;
	}
	const IOnlineIdentityPtr IdentityInterface = TargetedSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Refresh platform token failed as the identity interface instance was nullptr!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Incomplete);
		return false;
	}
	const UEnum* LoginTypeEnum = StaticEnum<EAccelByteLoginType>();
	EAccelByteLoginType LoginType = FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(TargetedSubsystem->GetSubsystemName());
	if (LoginType == EAccelByteLoginType::None
		|| LoginType == EAccelByteLoginType::AccelByte
		|| LoginType == EAccelByteLoginType::RefreshToken
		|| LoginType == EAccelByteLoginType::DeviceId
		|| LoginType == EAccelByteLoginType::CachedToken
		)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Refresh platform token failed as an invalid type was provided for the subsystem. AccelByte subsystem does not support platform token refresh for these subsystem type."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return false;
	}

	//The condition are already checked, reset the async task error info
	OutIdentityInterface = IdentityInterface;
	OutPlatformType = ConvertOSSTypeToAccelBytePlatformType(LoginType);
	this->ErrorMessage = TEXT("");
	this->ErrorCode = 0;
	return true;
}

void FOnlineAsyncTaskAccelByteRefreshPlatformToken::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize()"));
	Super::Initialize();

	IOnlineIdentityPtr NativeIdentityInterface = nullptr;
	EAccelBytePlatformType PlatformTypeForIAM = EAccelBytePlatformType::None;
	if (!CheckAndGetOnlineIdentityInterface(NativeIdentityInterface, PlatformTypeForIAM))
	{
		return;
	}
	
	//Special handling for EOS
	if (PlatformTypeForIAM == EAccelBytePlatformType::EpicGames)
	{
		FString PlatformToken = FGenericPlatformHttp::UrlEncode(NativeIdentityInterface->GetAuthToken(this->LocalUserNum));
		ApiClientRefreshToBackend(PlatformTypeForIAM, PlatformToken);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	// Need to use AutoLogin instead of GetAuthToken to get a new or refreshed token
	NativeLoginComplete = TDelegateUtils<FOnLoginCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshPlatformToken::OnNativeAutoLoginResponse);
	NativeLoginCompleteHandle = NativeIdentityInterface->AddOnLoginCompleteDelegate_Handle(this->LocalUserNum, NativeLoginComplete);
	NativeIdentityInterface->AutoLogin(this->LocalUserNum);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteRefreshPlatformToken::OnNativeAutoLoginResponse(int32 NativeLocalUserNum, bool bWasNativeLoginSuccessful, const FUniqueNetId& NativeUserId, const FString& NativeError)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FOnlineAsyncTaskAccelByteRefreshPlatformToken::OnNativeAutoLoginResponse"));

	IOnlineIdentityPtr NativeIdentityInterface = nullptr;
	EAccelBytePlatformType PlatformTypeForIAM = EAccelBytePlatformType::None;
	if (!CheckAndGetOnlineIdentityInterface(NativeIdentityInterface, PlatformTypeForIAM))
	{
		return;
	}
	NativeIdentityInterface->OnLoginCompleteDelegates->Remove(this->NativeLoginCompleteHandle);

	if (!bWasNativeLoginSuccessful)
	{
		this->ErrorMessage = TEXT("failed-refresh-platform-token");
		this->ErrorCode = static_cast<int32>(AccelByte::ErrorCodes::PlatformInternalServerErrorException);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Incomplete);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Refresh platform token failed since it can't generate the new platform Auth Token using AutoLogin()!"));
		return;
	}

	FString PlatformToken = FGenericPlatformHttp::UrlEncode(NativeIdentityInterface->GetAuthToken(this->LocalUserNum));
	ApiClientRefreshToBackend(PlatformTypeForIAM, PlatformToken);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshPlatformToken::ApiClientRefreshToBackend(EAccelBytePlatformType PlatformTypeForIAM, const FString& PlatformToken)
{
	auto OnSuccess = TDelegateUtils<THandler<FPlatformTokenRefreshResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshPlatformToken::HandleSuccess);
	auto OnError = TDelegateUtils<FOAuthErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshPlatformToken::HandleError);

	API_FULL_CHECK_GUARD(User, ErrorMessage);
	User->RefreshPlatformToken(PlatformTypeForIAM, PlatformToken, OnSuccess, OnError);
}

void FOnlineAsyncTaskAccelByteRefreshPlatformToken::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	
	Super::TriggerDelegates();
	const auto IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		const FOnlineErrorAccelByte OnlineError = bWasSuccessful ? ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success) :
			ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorMessage));
		IdentityInterface->TriggerAccelByteOnPlatformTokenRefreshedCompleteDelegates(LocalUserNum, bWasSuccessful, Result, RefreshSubsystemName, ErrorInfo);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshPlatformToken::HandleSuccess(const FPlatformTokenRefreshResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	Result = Response;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshPlatformToken::HandleError(int32 InCode, const FString& InMessage, const FErrorOAuthInfo& InErrorInfo)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	ErrorCode = InCode;
	ErrorMessage = InMessage;
	ErrorInfo = InErrorInfo;
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE