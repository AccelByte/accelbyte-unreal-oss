// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGenerateMfaBackupCodes.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes"

FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InLocalUserId)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	API_FULL_CHECK_GUARD(User, OnlineError);

	const auto OnSuccessDelegate = TDelegateUtils<THandler<FUser2FaBackupCode>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::OnSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::OnError);
	User->GenerateBackupCode(OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, bWasSuccessful: %s, ErrorMessage: %s")
		, *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineIdentityAccelBytePtr IdentityInterface;
	FOnlineIdentityAccelByte::GetFromSubsystem(SubsystemPin.Get(), IdentityInterface);

	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for generate Mfa Backup Codes as our identity interface invalid!"));
		return;
	}

	IdentityInterface->TriggerAccelByteOnGenerateMfaBackupCodesCompleteDelegates(LocalUserNum
		, *UserId
		, FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, OnlineError)
		, BackupCodes);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::OnTaskTimedOut()
{
	Super::OnTaskTimedOut();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(EOnlineErrorResult::RequestFailure))
		, FText::FromString(TEXT("Generate Mfa Backup Codes task timeout while waiting for response from backend.")));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::OnSuccess(const FUser2FaBackupCode& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	BackupCodes = Response;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateMfaBackupCodes::OnError(const int32 ErrorCode, const FString& ErrorMessage)
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
