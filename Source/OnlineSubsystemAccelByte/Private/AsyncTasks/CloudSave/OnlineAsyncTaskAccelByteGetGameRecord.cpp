// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetGameRecord"

FOnlineAsyncTaskAccelByteGetGameRecord::FOnlineAsyncTaskAccelByteGetGameRecord(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FString const& InKey
	, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Key(InKey)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
}

void FOnlineAsyncTaskAccelByteGetGameRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting game record, LocalUserNum: %d"), LocalUserNum);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), AccelByte::ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game record, not implemented"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game record, identity interface is invalid!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		TaskOnlineError = EOnlineErrorResult::AccessDenied;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-get-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game record, not logged in!"));
		return;
	}

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (!CloudSaveInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game record, cloud save interface is invalid!"));
		return;
	}

	OnGetGameRecordSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGameRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordSuccess);
	OnGetGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordError);
	
	if (!CloudSaveInterface->GetGameRecordFromCache(Key, GameRecord) || bAlwaysRequestToService)
	{
		if (IsDS.GetValue())
		{

			SERVER_API_CLIENT_CHECK_GUARD(TaskErrorStr);
			
			ServerApiClient->ServerCloudSave.GetGameRecord(Key, OnGetGameRecordSuccessDelegate, OnGetGameRecordErrorDelegate);
		}
		else
		{
			if (!UserId.IsValid())
			{
				TaskOnlineError = EOnlineErrorResult::InvalidUser;
				TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
				TaskErrorStr = TEXT("request-failed-get-game-record-error");
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User is not found at user index '%d'!"), LocalUserNum);
				return;
			}
			API_FULL_CHECK_GUARD(CloudSave,TaskErrorStr);
			CloudSave->GetGameRecord(Key, OnGetGameRecordSuccessDelegate, OnGetGameRecordErrorDelegate);
		}
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetGameRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGameRecordGetRecordByKeyPayload GameRecordGetRecordByKeyPayload{};
		GameRecordGetRecordByKeyPayload.Key = Key;
		GameRecordGetRecordByKeyPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGameRecordGetRecordByKeyPayload>(GameRecordGetRecordByKeyPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetGameRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key,  GameRecord);
		}
		else
		{
			CloudSaveInterface->TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Key, GameRecord);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordSuccess(const FAccelByteModelsGameRecord& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		CloudSaveInterface->AddGameRecordToMap(Key, MakeShared<FAccelByteModelsGameRecord>(Result));
		GameRecord = Result;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		OnGetGameRecordErrorDelegate.ExecuteIfBound(static_cast<int32>(AccelByte::ErrorCodes::StatusBadRequest), TEXT("Missing interfaces"));
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get game record Success!"));
}

void FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordError(int32 Code, FString const& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = TEXT("request-failed-get-game-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to get game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE