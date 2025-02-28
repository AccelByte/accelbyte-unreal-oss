// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetAdminGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetAdminGameRecord"

FOnlineAsyncTaskAccelByteGetAdminGameRecord::FOnlineAsyncTaskAccelByteGetAdminGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteGetAdminGameRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting admin game record, LocalUserNum: %d"), LocalUserNum);

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (!CloudSaveInterface.IsValid())
	{
		ErrorStr = TEXT("request-failed-get-admin-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get admin game record, cloud save interface is invalid!"));
		return;
	}

	OnGetAdminGameRecordSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsAdminGameRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetAdminGameRecord::OnGetAdminGameRecordSuccess);
	OnGetAdminGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetAdminGameRecord::OnGetAdminGameRecordError);
	
	if (!CloudSaveInterface->GetAdminGameRecordFromCache(Key, GameRecord) || bAlwaysRequestToService)
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
			ErrorStr = TEXT("request-failed-get-admin-game-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, identity interface is invalid!"));
			return;
		}

		if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
		{
			ErrorCode = TEXT("401");
			ErrorStr = TEXT("request-failed-get-admin- game-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game user, not logged in!"));
			return;
		}

		SERVER_API_CLIENT_CHECK_GUARD(ErrorStr);
		
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
			CloudSaveInterface->TriggerOnGetAdminGameRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrorStr)), Key, GameRecord);
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
	}
	GameRecord = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get game record Success!"));
}

void FOnlineAsyncTaskAccelByteGetAdminGameRecord::OnGetAdminGameRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = TEXT("request-failed-get-admin-game-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to get admin game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE