// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "OnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig"
#define ONLINE_TASK_ERROR TEXT("request-failed-delete-game-binary-record-TTL-config-error")

FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const & InKey)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
{
}

void FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Delete game binary record TTL config"));

	TRY_PIN_SUBSYSTEM();

	if (Key.IsEmpty())
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete game binary record TTL config, Key empty!"));
		return;
	}

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || !IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete game binary record TTL config, access denied!"));
		return;
	}

	OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::OnSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::OnError);

	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-missing-interface");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete game binary record, access denied!"));
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
	ServerApiClient->ServerBinaryCloudSave.DeleteGameBinaryRecordTTLConfig(Key, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineBinaryCloudSaveAccelBytePtr BinaryCloudSaveInterface = SubsystemPin->GetBinaryCloudSaveInterface();
	if (BinaryCloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			BinaryCloudSaveInterface->TriggerOnDeleteGameBinaryRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			BinaryCloudSaveInterface->TriggerOnDeleteGameBinaryRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::OnSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	TaskOnlineError = EOnlineErrorResult::Success;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Delete game binary record TTL config Success!"));
}

void FOnlineAsyncTaskAccelByteDeleteGameBinaryRecordTTLConfig::OnError(int32 Code, const FString& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = ONLINE_TASK_ERROR;
	UE_LOG_AB(Warning, TEXT("Failed to delete game binary record TTL config! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_TASK_ERROR
#undef ONLINE_ERROR_NAMESPACE