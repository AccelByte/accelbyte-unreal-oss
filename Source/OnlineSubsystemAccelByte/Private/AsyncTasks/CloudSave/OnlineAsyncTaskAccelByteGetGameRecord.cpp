// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetGameRecord"

FOnlineAsyncTaskAccelByteGetGameRecord::FOnlineAsyncTaskAccelByteGetGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteGetGameRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting game record, LocalUserNum: %d"), LocalUserNum);

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (!CloudSaveInterface.IsValid())
	{
		ErrorStr = TEXT("request-failed-get-game-record-error");
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
				ErrorStr = TEXT("request-failed-get-game-record-error");
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, identity interface is invalid!"));
				return;
			}

			if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
			{
				ErrorCode = TEXT("401");
				ErrorStr = TEXT("request-failed-get-game-record-error");
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game user, not logged in!"));
				return;
			}

			FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
			ServerApiClient->ServerCloudSave.GetGameRecord(Key, OnGetGameRecordSuccessDelegate, OnGetGameRecordErrorDelegate);
		}
		else
		{
			API_CLIENT_CHECK_GUARD(ErrorStr);
			ApiClient->CloudSave.GetGameRecord(Key, OnGetGameRecordSuccessDelegate, OnGetGameRecordErrorDelegate);
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
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
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
			CloudSaveInterface->TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrorStr)), Key, GameRecord);
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
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get game record Success!"));
}

void FOnlineAsyncTaskAccelByteGetGameRecord::OnGetGameRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = TEXT("request-failed-get-game-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to get game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE