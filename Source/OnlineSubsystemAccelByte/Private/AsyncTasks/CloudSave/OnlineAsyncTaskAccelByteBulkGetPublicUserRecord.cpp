// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkGetPublicUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord"

FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const& InKey
	, TArray<FString> const& InUserIds)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, UserIds(InUserIds)
{
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting bulk public user record, UserId: %s"), *UserId->ToDebugString());

	TRY_PIN_SUBSYSTEM();

	if (UserIds.Num() <= 0)
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, userIds empty!"));
		return;
	}

	OnBulkGetPublicUserRecordSuccessDelegate = TDelegateUtils<THandler<FListAccelByteModelsUserRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::OnBulkGetPublicUserRecordSuccess);
	OnBulkGetPublicUserRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::OnBulkGetPublicUserRecordError);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, access denied!"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-missing-interface");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, access denied!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		TaskOnlineError = EOnlineErrorResult::InvalidAuth;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		return;
	}

	if (!UserId.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::InvalidUser;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User is not found at user index '%d'!"), LocalUserNum);
		return;
	}

	API_FULL_CHECK_GUARD(CloudSave, TaskErrorStr);
	CloudSave->BulkGetPublicUserRecord(Key, UserIds, OnBulkGetPublicUserRecordSuccessDelegate, OnBulkGetPublicUserRecordErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsPublicPlayerRecordGetSameRecordsFromUsersPayload PublicPlayerRecordGetSameRecordsFromUsersPayload{};
		PublicPlayerRecordGetSameRecordsFromUsersPayload.Key = Key;
		PublicPlayerRecordGetSameRecordsFromUsersPayload.UserIds = UserIds;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPublicPlayerRecordGetSameRecordsFromUsersPayload>(PublicPlayerRecordGetSameRecordsFromUsersPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), ListUserRecord);
		}
		else
		{
			for(const FString& RecordUserId : UserIds)
			{
				FAccelByteModelsUserRecord Record{};
				Record.Key = Key;
				Record.UserId = RecordUserId;
				ListUserRecord.Data.Add(Record);
			}
			CloudSaveInterface->TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), ListUserRecord);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::OnBulkGetPublicUserRecordSuccess(const FListAccelByteModelsUserRecord& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	ListUserRecord = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get bulk public user record for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord::OnBulkGetPublicUserRecordError(int32 Code, const FString& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = TEXT("request-failed-get-bulk-public-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to get bulk public user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE