// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteEnableMfaAuthenticator.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteEnableMfaAuthenticator"

FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::FOnlineAsyncTaskAccelByteEnableMfaAuthenticator(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId, const FString& InCode)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, Code(InCode)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	API_FULL_CHECK_GUARD(User, OnlineError);

	const auto OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::OnEnableMfaAuthenticatorSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::OnEnableMfaAuthenticatorError);
	User->Enable2FaAuthenticator(OnSuccessDelegate, OnErrorDelegate, Code);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, bWasSuccessful: %s, ErrorMessage: %s")
		, *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineIdentityAccelByte::GetFromSubsystem(SubsystemPin.Get(), IdentityInterface);

	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for enable Mfa Authenticator as our identity interface invalid!"));
		return;
	}

	IdentityInterface->TriggerAccelByteOnEnableMfaAuthenticatorCompleteDelegates(LocalUserNum
		, *UserId
		, FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, OnlineError));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::OnTaskTimedOut()
{
	Super::OnTaskTimedOut();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(EOnlineErrorResult::RequestFailure))
		, FText::FromString(TEXT("Enable Mfa Authenticator task timeout while waiting for response from backend.")));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::OnEnableMfaAuthenticatorSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());
    
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnableMfaAuthenticator::OnEnableMfaAuthenticatorError(const int32 ErrorCode,	const FString& ErrorMessage)
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
