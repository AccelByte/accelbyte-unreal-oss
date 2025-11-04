// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateGameBinaryRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "OnlineAsyncTaskAccelByteCreateGameBinaryRecord"
#define ONLINE_TASK_ERROR TEXT("request-failed-create-game-binary-record-error")

FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::FOnlineAsyncTaskAccelByteCreateGameBinaryRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const & InKey
	, EAccelByteFileType const& InFileType
	, ESetByMetadataRecord const& InSetBy
	, FTTLConfig const& InTTLConfig)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, FileType(InFileType)
	, SetBy(InSetBy)
	, TTLConfig(InTTLConfig)
{
}

void FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Create game binary record"));

	TRY_PIN_SUBSYSTEM();


	if (Key.IsEmpty())
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = ONLINE_TASK_ERROR;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game binary record, Key empty!"));
		return;
	}

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || !IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game binary record, access denied!"));
		return;
	}

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsBinaryInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::OnSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::OnError);

	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-missing-interface");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game binary record, access denied!"));
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
	ServerApiClient->ServerBinaryCloudSave.CreateGameBinaryRecord(Key
		, FileType
		, SetBy
		, OnSuccessDelegate
		, OnErrorDelegate
		, TTLConfig);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineBinaryCloudSaveAccelBytePtr BinaryCloudSaveInterface = SubsystemPin->GetBinaryCloudSaveInterface();
	if (BinaryCloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			BinaryCloudSaveInterface->TriggerOnCreateGameBinaryRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Result);
		}
		else
		{
			BinaryCloudSaveInterface->TriggerOnCreateGameBinaryRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Result);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::OnSuccess(FAccelByteModelsBinaryInfo const & InResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Result = InResult;
	TaskOnlineError = EOnlineErrorResult::Success;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Create game binary record Success!"));
}

void FOnlineAsyncTaskAccelByteCreateGameBinaryRecord::OnError(int32 Code, const FString& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = ONLINE_TASK_ERROR;
	UE_LOG_AB(Warning, TEXT("Failed to create game binary record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_TASK_ERROR
#undef ONLINE_ERROR_NAMESPACE