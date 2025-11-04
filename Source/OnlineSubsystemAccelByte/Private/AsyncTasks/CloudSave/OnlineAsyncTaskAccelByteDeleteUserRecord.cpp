// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteDeleteUserRecord"

FOnlineAsyncTaskAccelByteDeleteUserRecord::FOnlineAsyncTaskAccelByteDeleteUserRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const& InKey
	, FString const& InTargetUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, TargetUserId(InTargetUserId)
{
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Deleting user record, UserId: %s"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), AccelByte::ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-delete-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, not implemented"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-delete-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, identity interface is invalid!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		TaskOnlineError = EOnlineErrorResult::AccessDenied;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-delete-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, not logged in!"));
		return;
	}

	OnDeleteUserRecordSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordSuccess);
	OnDeleteUserRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordError);
	
	if (IsDS.GetValue())
	{
		SERVER_API_CLIENT_CHECK_GUARD(TaskErrorStr);

		if (TargetUserId.IsEmpty() || TargetUserId == ACCELBYTE_INVALID_ID_VALUE)
		{
			TaskOnlineError = EOnlineErrorResult::InvalidParams;
			TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest);
			TaskErrorStr = TEXT("request-failed-delete-user-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, targetUserId is invalid!"));
			return;
		}
		
		ServerApiClient->ServerCloudSave.DeleteUserRecord(Key, *TargetUserId, false, OnDeleteUserRecordSuccessDelegate, OnDeleteUserRecordErrorDelegate);
	}
	else
	{
		if (UserId->GetAccelByteId() != TargetUserId)
		{
			TaskOnlineError = EOnlineErrorResult::NotImplemented;
			TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
			TaskErrorStr = TEXT("request-failed-delete-user-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, targetUserId is not the player!"));
			return;
		}
		API_FULL_CHECK_GUARD(CloudSave, TaskErrorStr);
		CloudSave->DeleteUserRecord(Key, OnDeleteUserRecordSuccessDelegate, OnDeleteUserRecordErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsPlayerRecordDeletedPayload PlayerRecordDeletedPayload{};
		PlayerRecordDeletedPayload.Key = Key;
		PlayerRecordDeletedPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TargetUserId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordDeletedPayload>(PlayerRecordDeletedPayload));
	}
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to delete user record for user '%s' Success!"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordError(int32 Code, FString const& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = TEXT("request-faileTasklete-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to delete user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE