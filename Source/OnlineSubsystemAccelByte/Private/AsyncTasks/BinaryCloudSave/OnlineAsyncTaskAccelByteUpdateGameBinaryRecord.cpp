// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateGameBinaryRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "OnlineAsyncTaskAccelByteUpdateGameBinaryRecord"
#define ONLINE_TASK_ERROR TEXT("request-failed-update-game-binary-record-file-error")

FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const& InKey
	, EAccelByteFileType InContentType
	, FString const& InFileLocation)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, ContentType(InContentType)
	, FileLocation(InFileLocation)
{
}

void FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Update game binary record file"));

	TRY_PIN_SUBSYSTEM();

	if (Key.IsEmpty())
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update game binary record file, Key empty!"));
		return;
	}

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || !IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update game binary record file, access denied!"));
		return;
	}

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGameBinaryRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::OnSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::OnError);

	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-missing-interface");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update game binary record, access denied!"));
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
	
	SERVER_API_CLIENT_CHECK_GUARD();
	ServerApiClient->ServerBinaryCloudSave.UpdateGameBinaryRecord(Key, ContentType, FileLocation, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	const FOnlineBinaryCloudSaveAccelBytePtr BinaryCloudSaveInterface = SubsystemPin->GetBinaryCloudSaveInterface();
	if (BinaryCloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			BinaryCloudSaveInterface->TriggerOnUpdateGameBinaryRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Result);
		}
		else
		{
			BinaryCloudSaveInterface->TriggerOnUpdateGameBinaryRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)),  Result);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::OnSuccess(FAccelByteModelsGameBinaryRecord const& InResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	TaskOnlineError = EOnlineErrorResult::Success;
	Result = InResult;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to update game binary record file Success!"));
}

void FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord::OnError(int32 Code, const FString& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = ONLINE_TASK_ERROR;
	UE_LOG_AB(Warning, TEXT("Failed to update game binary record file! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_TASK_ERROR
#undef ONLINE_ERROR_NAMESPACE