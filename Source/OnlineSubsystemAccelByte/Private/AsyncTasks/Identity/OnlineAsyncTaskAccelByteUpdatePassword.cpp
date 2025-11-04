// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdatePassword.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteUpdatePassword"

FOnlineAsyncTaskAccelByteUpdatePassword::FOnlineAsyncTaskAccelByteUpdatePassword(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId,
	const FUpdatePasswordRequest& InUpdatePasswordRequest, EAccelByteLoginAuthFactorType InFactorType,
	const FString& InCode)
		: FOnlineAsyncTaskAccelByte(InABSubsystem)
		, UpdatePasswordRequest(InUpdatePasswordRequest)
		, FactorType(InFactorType)
		, Code(InCode)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteUpdatePassword::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	if (FactorType == EAccelByteLoginAuthFactorType::None)
	{
		UpdatePassword();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Factor type is none continue updating password."))
		return;
	}

	GetMfaToken();
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Factor type is not none, retrieving mfa token."))
}

void FOnlineAsyncTaskAccelByteUpdatePassword::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, bWasSuccessful: %s, ErrorMessage: %s")
		, *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineIdentityAccelByte::GetFromSubsystem(SubsystemPin.Get(), IdentityInterface);

	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for update password as our identity interface invalid!"));
		return;
	}

	IdentityInterface->TriggerAccelByteOnUpdatePasswordCompleteDelegates(LocalUserNum
		, *UserId
		, FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, OnlineError));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePassword::OnTaskTimedOut()
{
	Super::OnTaskTimedOut();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(EOnlineErrorResult::RequestFailure))
		, FText::FromString(TEXT("Update password task timeout while waiting for response from backend.")));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePassword::GetMfaToken()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	API_FULL_CHECK_GUARD(User, OnlineError);

	const auto OnSuccessDelegate = TDelegateUtils<THandler<FVerifyMfaResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePassword::OnGetMfaTokenSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePassword::OnGetMfaTokenError);
	User->VerifyMfaCode(FactorType, Code, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteUpdatePassword::OnGetMfaTokenSuccess(const FVerifyMfaResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	UpdatePasswordRequest.MfaToken = Response.MfaToken;
	UpdatePassword();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePassword::OnGetMfaTokenError(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, ErrorCode: %d, ErrorMessage: %s")
		, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(ErrorCode)
		, FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePassword::UpdatePassword()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	API_FULL_CHECK_GUARD(User, OnlineError);

	const auto OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePassword::OnUpdatePasswordSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePassword::OnUpdatePasswordError);
	User->UpdatePassword(UpdatePasswordRequest, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteUpdatePassword::OnUpdatePasswordSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());
    
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePassword::OnUpdatePasswordError(const int32 ErrorCode, const FString& ErrorMessage)
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
