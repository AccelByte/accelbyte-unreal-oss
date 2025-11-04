// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetUserRecord"

FOnlineAsyncTaskAccelByteGetUserRecord::FOnlineAsyncTaskAccelByteGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const& InKey
	, bool IsPublic
	, FString const& InRecordUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, RecordUserId(InRecordUserId)
	, IsPublicRecord(IsPublic)
{
}

void FOnlineAsyncTaskAccelByteGetUserRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting user record, UserId: %s"), *RecordUserId);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), AccelByte::ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, not implemented"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, identity interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		TaskOnlineError = EOnlineErrorResult::InvalidAuth;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, not logged in!"));
		return;
	}

	OnGetUserRecordsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsSuccess);
	OnGetUserRecordsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsError);
		
	if (IsDS.GetValue())
	{
		SERVER_API_CLIENT_CHECK_GUARD(TaskErrorStr);
		
		if (IsPublicRecord)
		{
			ServerApiClient->ServerCloudSave.GetPublicUserRecord(Key, RecordUserId, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
		else
		{
			ServerApiClient->ServerCloudSave.GetUserRecord(Key, RecordUserId, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
	}
	else
	{
		if (RecordUserId.IsEmpty())
		{
			RecordUserId = *UserId->GetAccelByteId();
		}
		if (IsPublicRecord)
		{
			API_FULL_CHECK_GUARD(CloudSave, TaskErrorStr);
			CloudSave->GetPublicUserRecord(Key, RecordUserId, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
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
			if (RecordUserId == *UserId->GetAccelByteId())
			{
				API_FULL_CHECK_GUARD(CloudSave, TaskErrorStr);
				CloudSave->GetUserRecord(Key, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
			}
			else
			{
				TaskOnlineError = EOnlineErrorResult::RequestFailure;
				TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
				TaskErrorStr = TEXT("request-failed-get-user-record-error");
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Game client can't get another user's private records!"));
				return;
			}
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError), Key, UserRecord);
		}
		else
		{
			UserRecord.Key = Key;
			UserRecord.UserId = RecordUserId;
			CloudSaveInterface->TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Key, UserRecord);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		if (IsPublicRecord)
		{
			FAccelByteModelsPublicPlayerRecordGetRecordPayload PublicPlayerRecordGetRecordPayload{};
			PublicPlayerRecordGetRecordPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
			PublicPlayerRecordGetRecordPayload.Key = Key;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPublicPlayerRecordGetRecordPayload>(PublicPlayerRecordGetRecordPayload));
		}
		else
		{
			FAccelByteModelsPlayerRecordGetRecordsPayload PlayerRecordGetRecordsPayload{};
			PlayerRecordGetRecordsPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
			PlayerRecordGetRecordsPayload.Keys.Add(Key);
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordGetRecordsPayload>(PlayerRecordGetRecordsPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsSuccess(const FAccelByteModelsUserRecord& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	TaskOnlineError = EOnlineErrorResult::Success;
	UserRecord = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get user '%s' record Success!"), *RecordUserId);
}

void FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsError(int32 Code, FString const& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = TEXT("request-failed-get-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to get user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE