// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineError.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetGameRecord"

FOnlineAsyncTaskAccelByteGetGameRecord::FOnlineAsyncTaskAccelByteGetGameRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGetGameRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting game record, UserId: %s"), *UserId->ToDebugString());

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (!CloudSaveInterface.IsValid())
	{
		ErrorStr = FText::FromString(TEXT("request-failed-get-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game record, cloud save interface is invalid!"));
		return;
	}

	OnGetGameRecordSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGameRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordSuccess);
	OnGetGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordError);
	
	if (!CloudSaveInterface->GetGameRecordFromCache(Key, GameRecord) || bAlwaysRequestToService)
	{
		if (IsRunningDedicatedServer())
		{
			const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
			if (!IdentityInterface.IsValid())
			{
				ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
				ErrorStr = FText::FromString(TEXT("request-failed-get-game-record-error"));
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, identity interface is invalid!"));
				return;
			}

			if (IdentityInterface->GetLoginStatus(*UserId) != ELoginStatus::LoggedIn)
			{
				ErrorCode = TEXT("401");
				ErrorStr = FText::FromString(TEXT("request-failed-get-game-record-error"));
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game user, not logged in!"));
				return;
			}

			FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
			ServerApiClient->ServerCloudSave.GetGameRecord(Key, OnGetGameRecordSuccessDelegate, OnGetGameRecordErrorDelegate);
		}
		else
		{
			ApiClient->CloudSave.GetGameRecord(Key, OnGetGameRecordSuccessDelegate, OnGetGameRecordErrorDelegate);
		}
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetGameRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key,  GameRecord);
		}
		else
		{
			CloudSaveInterface->TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, ErrorStr), Key, GameRecord);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordSuccess(const FAccelByteModelsGameRecord& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		CloudSaveInterface->AddGameRecordToMap(Key, MakeShared<FAccelByteModelsGameRecord>(Result));
	}
	GameRecord = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get game record for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = FText::FromString(TEXT("request-failed-get-game-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to get game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}