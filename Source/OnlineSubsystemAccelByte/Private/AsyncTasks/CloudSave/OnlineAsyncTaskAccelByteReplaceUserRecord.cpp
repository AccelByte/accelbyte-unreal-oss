// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReplaceUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteReplaceUserRecord"

FOnlineAsyncTaskAccelByteReplaceUserRecord::FOnlineAsyncTaskAccelByteReplaceUserRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const& InKey
	, FJsonObject const& InUserRecordObj
	, bool IsPublic
	, FString const& InTargetUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, UserRecordObj(InUserRecordObj)
	, IsPublicRecord(IsPublic)
	, TargetUserId(InTargetUserId)
{
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("replacing user record, UserId: %s"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), AccelByte::ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-replace-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, not implemented"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-replace-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, identity interface is invalid!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		TaskOnlineError = EOnlineErrorResult::AccessDenied;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-replace-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, not logged in!"));
		return;
	}

	OnReplaceUserRecordSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsSuccess);
	OnReplaceUserRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsError);
	
	if (IsDS.GetValue())
	{
		SERVER_API_CLIENT_CHECK_GUARD(TaskErrorStr);
		
		ServerApiClient->ServerCloudSave.ReplaceUserRecord(Key, ESetByMetadataRecord::SERVER, IsPublicRecord, TargetUserId, UserRecordObj, OnReplaceUserRecordSuccessDelegate, OnReplaceUserRecordErrorDelegate);
		SetBy = FAccelByteUtilities::GetUEnumValueAsString(ESetByMetadataRecord::SERVER);
	}
	else
	{
		if (!UserId.IsValid())
		{
			TaskOnlineError = EOnlineErrorResult::InvalidUser;
			TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
			TaskErrorStr = TEXT("request-failed-get-user-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User is not found at user index '%d'!"), LocalUserNum);
			return;
		}
		API_FULL_CHECK_GUARD(CloudSave,TaskErrorStr);
		CloudSave->ReplaceUserRecord(Key, IsPublicRecord, UserRecordObj, OnReplaceUserRecordSuccessDelegate, OnReplaceUserRecordErrorDelegate);
		SetBy = FAccelByteUtilities::GetUEnumValueAsString(ESetByMetadataRecord::CLIENT);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsPlayerRecordUpdatedPayload PlayerRecordUpdatedPayload{};
		PlayerRecordUpdatedPayload.Key = Key;
		PlayerRecordUpdatedPayload.IsPublic = IsPublicRecord;
		PlayerRecordUpdatedPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TargetUserId;
		PlayerRecordUpdatedPayload.SetBy = SetBy;
		PlayerRecordUpdatedPayload.Strategy = TEXT("REPLACE");
		PlayerRecordUpdatedPayload.Value.JsonObject = MakeShared<FJsonObject>(UserRecordObj);
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordUpdatedPayload>(PlayerRecordUpdatedPayload));
	}
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to replace user '%s' record Success!"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsError(int32 Code, FString const& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = TEXT("request-failed-replace-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to replace user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE