// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "OnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord"
#define ONLINE_TASK_ERROR TEXT("request-failed-bulk-get-public-user-binary-records-error")

FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord(FOnlineSubsystemAccelByte* const InABInterface 
	, int32 InLocalUserNum
	, FString const & InKey
	, TArray<FString> const & InUserIds)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, TargetUserIds(InUserIds)
{
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Bulk get public user binary record"));

	TRY_PIN_SUBSYSTEM();

	if (Key.IsEmpty())
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public user binary record, Key empty!"));
		return;
	}

	if (TargetUserIds.Num() == 0)
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public user binary record, TargetUserId empty!"));
		return;
	}

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsListUserBinaryRecords>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::OnSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::OnError);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get current user binary record, access denied!"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-missing-interface");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get current user binary record, access denied!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		TaskOnlineError = EOnlineErrorResult::InvalidAuth;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		return;
	}

	if (!UserId.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::InvalidUser;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User is not found at user index '%d'!"), LocalUserNum);
		return;
	}

	API_FULL_CHECK_GUARD(BinaryCloudSave, TaskErrorStr);
	BinaryCloudSave->BulkGetPublicUserBinaryRecords(Key, TargetUserIds, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::Finalize() 
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineBinaryCloudSaveAccelBytePtr BinaryCloudSaveInterface = SubsystemPin->GetBinaryCloudSaveInterface();
	if (BinaryCloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			BinaryCloudSaveInterface->TriggerOnBulkGetUserBinaryRecordsCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), UserRecords);
		}
		else
		{
			BinaryCloudSaveInterface->TriggerOnBulkGetUserBinaryRecordsCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), UserRecords);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::OnSuccess(FAccelByteModelsListUserBinaryRecords const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	TaskOnlineError = EOnlineErrorResult::Success;
	UserRecords = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to bulk get public user binary records for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord::OnError(int32 Code, const FString& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = ONLINE_TASK_ERROR;
	UE_LOG_AB(Warning, TEXT("Failed to bulk get public user binary records! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_TASK_ERROR
#undef ONLINE_ERROR_NAMESPACE