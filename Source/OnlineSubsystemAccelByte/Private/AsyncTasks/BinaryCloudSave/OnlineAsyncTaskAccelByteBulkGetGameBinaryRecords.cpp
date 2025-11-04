// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkGetGameBinaryRecords.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "OnlineAsyncTaskAccelByteBulkGetGameBinaryRecords"
#define ONLINE_TASK_ERROR TEXT("request-failed-bulk-get-game-binary-records-error")

FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords(FOnlineSubsystemAccelByte* const InABInterface 
	, int32 InLocalUserNum 
	, TArray<FString> const & InKeys)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Keys(InKeys)
{
}

void FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Bulk get game binary records"));

	TRY_PIN_SUBSYSTEM();

	if (Keys.Num() == 0)
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk get game binary records, Keys empty!"));
		return;
	}

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsListGameBinaryRecords>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::OnSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::OnError);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk get game binary records, access denied!"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-missing-interface");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk get game binary records, access denied!"));
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
	BinaryCloudSave->BulkGetGameBinaryRecords(Keys, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineBinaryCloudSaveAccelBytePtr BinaryCloudSaveInterface = SubsystemPin->GetBinaryCloudSaveInterface();
	if (BinaryCloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			BinaryCloudSaveInterface->TriggerOnBulkGetGameBinaryRecordsCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Result);
		}
		else
		{
			BinaryCloudSaveInterface->TriggerOnBulkGetGameBinaryRecordsCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Result);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::OnSuccess(FAccelByteModelsListGameBinaryRecords const& InResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	TaskOnlineError = EOnlineErrorResult::Success;
	Result = InResult;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get current user binary record for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords::OnError(int32 Code, const FString& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = ONLINE_TASK_ERROR;
	UE_LOG_AB(Warning, TEXT("Failed to get current user binary record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_TASK_ERROR
#undef ONLINE_ERROR_NAMESPACE