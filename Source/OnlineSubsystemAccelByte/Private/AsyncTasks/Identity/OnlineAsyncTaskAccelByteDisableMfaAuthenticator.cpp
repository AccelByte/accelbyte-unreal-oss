// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDisableMfaAuthenticator.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteDisableMfaAuthenticator"

FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::FOnlineAsyncTaskAccelByteDisableMfaAuthenticator(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId, const FString& InCode)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, Code(InCode)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	TRY_PIN_SUBSYSTEM();

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet())
	{
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::InvalidAuth
			, FString(TEXT("disable-mfa-authenticator-failed-user-not-logged-in"))
			, FText::FromString(TEXT("Failed to disable MFA authenticator, user is not logged in!")));
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to disable MFA authenticator, user is not logged in!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (IsDS.GetValue())
	{
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::NotImplemented
			, FString(TEXT("disable-mfa-authenticator-not-supported-on-dedicated-server"))
			, FText::FromString(TEXT("Failed to disable MFA authenticator, operation not supported on dedicated server!")));
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to disable MFA authenticator, operation not supported on dedicated server!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (Code.IsEmpty())
	{
		DisableMfaAuthenticator();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Code is empty continue disabling Mfa email."))
		return;
	}

	GetMfaToken();
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Backup code is not empty, retrieving mfa token."))
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, bWasSuccessful: %s, ErrorMessage: %s")
		, *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineIdentityAccelByte::GetFromSubsystem(SubsystemPin.Get(), IdentityInterface);

	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for disable Mfa Authenticator as our identity interface invalid!"));
		return;
	}

	IdentityInterface->TriggerAccelByteOnDisableMfaAuthenticatorCompleteDelegates(LocalUserNum
		, *UserId
		, FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, OnlineError));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnTaskTimedOut()
{
	Super::OnTaskTimedOut();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(EOnlineErrorResult::RequestFailure))
		, FText::FromString(TEXT("Disable Mfa Authenticator task timeout while waiting for response from backend.")));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::GetMfaToken()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());
    
	API_FULL_CHECK_GUARD(User, OnlineError);

	const auto OnSuccessDelegate = TDelegateUtils<THandler<FVerifyMfaResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnGetMfaTokenSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnGetMfaTokenError);
	User->VerifyMfaCode(EAccelByteLoginAuthFactorType::Authenticator, Code, OnSuccessDelegate, OnErrorDelegate);
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnGetMfaTokenSuccess(const FVerifyMfaResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	MfaToken = Response.MfaToken;
	DisableMfaAuthenticator();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnGetMfaTokenError(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, ErrorCode: %d, ErrorMessage: %s")
		, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(ErrorCode)
		, FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::DisableMfaAuthenticator()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	API_FULL_CHECK_GUARD(User, OnlineError);

	FDisableMfaAuthenticatorRequest Request;
	Request.MfaToken = MfaToken;
	
	const auto OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnDisableMfaAuthenticatorSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnDisableMfaAuthenticatorError);
	User->Disable2FaAuthenticator(OnSuccessDelegate, OnErrorDelegate, Request);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnDisableMfaAuthenticatorSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());
    
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDisableMfaAuthenticator::OnDisableMfaAuthenticatorError(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, ErrorCode: %d, ErrorMessage: %s")
	, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(ErrorCode)
		, FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
