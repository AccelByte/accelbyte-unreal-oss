// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetUserRecord"

FOnlineAsyncTaskAccelByteGetUserRecord::FOnlineAsyncTaskAccelByteGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum, const FUniqueNetId& InLocalUserId, const FString& InKey, bool IsPublic, const FString& InRecordUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, RecordUserId(InRecordUserId)
	, IsPublicRecord(IsPublic)
{
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteGetUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting user record, UserId: %s"), *RecordUserId);

	OnGetUserRecordsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsSuccess);
	OnGetUserRecordsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsError);
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-get-user-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, identity interface is invalid!"));
		return;
	}

	auto LoginStatus = UserId.IsValid() ? IdentityInterface->GetLoginStatus(*UserId) : IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-get-user-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game user, not logged in!"));
		return;
	}

	if (IsRunningDedicatedServer())
	{
		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		if (IsPublicRecord)
		{
			ServerApiClient->ServerCloudSave.GetPublicUserRecord(Key, RecordUserId, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
		else
		{
			ServerApiClient->ServerCloudSave.GetUserRecord(Key, RecordUserId, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
	}
	else
	{
		if (RecordUserId.IsEmpty())
		{
			RecordUserId = *UserId->GetAccelByteId();
		}
		if (IsPublicRecord)
		{
			ApiClient->CloudSave.GetPublicUserRecord(Key, RecordUserId, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
		else
		{
			if (RecordUserId == *UserId->GetAccelByteId())
			{
				ApiClient->CloudSave.GetUserRecord(Key, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
			}
			else
			{
				ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
				ErrorStr = FText::FromString(TEXT("request-failed-get-user-record-error"));
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Game client can't get another user's private records!"));
				return;
			}
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key, UserRecord);
		}
		else
		{
			UserRecord.Key = Key;
			UserRecord.UserId = RecordUserId;
			CloudSaveInterface->TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, ErrorStr), Key, UserRecord);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserRecord::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		if (IsPublicRecord)
		{
			FAccelByteModelsPublicPlayerRecordGetRecordPayload PublicPlayerRecordGetRecordPayload{};
			PublicPlayerRecordGetRecordPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
			PublicPlayerRecordGetRecordPayload.Key = Key;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPublicPlayerRecordGetRecordPayload>(PublicPlayerRecordGetRecordPayload));
		}
		else
		{
			FAccelByteModelsPlayerRecordGetRecordsPayload PlayerRecordGetRecordsPayload{};
			PlayerRecordGetRecordsPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
			PlayerRecordGetRecordsPayload.Keys.Add(Key);
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordGetRecordsPayload>(PlayerRecordGetRecordsPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsSuccess(const FAccelByteModelsUserRecord& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UserRecord = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get user '%s' record Success!"), *RecordUserId);
}

void FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = FText::FromString(TEXT("request-failed-get-user-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to get user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE