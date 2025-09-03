// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetAdminGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetAdminGameRecord"

FOnlineAsyncTaskAccelByteGetAdminGameRecord::FOnlineAsyncTaskAccelByteGetAdminGameRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const& InKey
	, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface
		, InLocalUserNum
		, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::UseTimeout) + ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Key(InKey)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
}

void FOnlineAsyncTaskAccelByteGetAdminGameRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting admin game record, LocalUserNum: %d"), LocalUserNum);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || !IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-create-admin-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game record, access denied!"));
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

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (!CloudSaveInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-admin-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get admin game record, cloud save interface is invalid!"));
		return;
	}

	OnGetAdminGameRecordSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsAdminGameRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetAdminGameRecord::OnGetAdminGameRecordSuccess);
	OnGetAdminGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetAdminGameRecord::OnGetAdminGameRecordError);
	
	if (!CloudSaveInterface->GetAdminGameRecordFromCache(Key, GameRecord) || bAlwaysRequestToService)
	{
		SERVER_API_CLIENT_CHECK_GUARD(TaskErrorStr);
		
		ServerApiClient->ServerCloudSave.QueryAdminGameRecordsByKey(Key, OnGetAdminGameRecordSuccessDelegate, OnGetAdminGameRecordErrorDelegate);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetAdminGameRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnGetAdminGameRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::Success), Key,  GameRecord);
		}
		else
		{
			CloudSaveInterface->TriggerOnGetAdminGameRecordCompletedDelegates(ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Key, GameRecord);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetAdminGameRecord::OnGetAdminGameRecordSuccess(const FAccelByteModelsAdminGameRecord& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		CloudSaveInterface->AddAdminGameRecordToMap(Key, MakeShared<FAccelByteModelsAdminGameRecord>(Result));
		GameRecord = Result;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		OnGetAdminGameRecordErrorDelegate.ExecuteIfBound(static_cast<int32>(AccelByte::ErrorCodes::StatusBadRequest), TEXT("Missing interfaces"));
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get game record Success!"));
}

void FOnlineAsyncTaskAccelByteGetAdminGameRecord::OnGetAdminGameRecordError(int32 Code, const FString& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = TEXT("request-failed-get-admin-game-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to get admin game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE