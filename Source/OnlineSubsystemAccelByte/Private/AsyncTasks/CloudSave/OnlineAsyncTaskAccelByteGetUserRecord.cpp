// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteGetUserRecord::FOnlineAsyncTaskAccelByteGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, bool IsPublic)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, IsPublicRecord(IsPublic)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteGetUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting user record, UserId: %s"), *UserId->ToDebugString());

	OnGetUserRecordsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsSuccess);
	OnGetUserRecordsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsError);
	
	if (IsRunningDedicatedServer())
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			ErrorStr = TEXT("request-failed-get-user-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, identity interface is invalid!"));
			return;
		}

		if (IdentityInterface->GetLoginStatus(*UserId) != ELoginStatus::LoggedIn)
		{
			ErrorStr = TEXT("request-failed-get-user-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game user, not logged in!"));
			return;
		}

		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient(UserId.Get()->GetAccelByteId());
		if (IsPublicRecord)
		{
			ServerApiClient->ServerCloudSave.GetPublicUserRecord(Key, UserId.Get()->GetAccelByteId(), OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
		else
		{
			ServerApiClient->ServerCloudSave.GetUserRecord(Key, UserId.Get()->GetAccelByteId(), OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
	}
	else
	{
		if (IsPublicRecord)
		{
			ApiClient->CloudSave.GetPublicUserRecord(Key, *UserId->GetAccelByteId(), OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
		}
		else
		{
			ApiClient->CloudSave.GetUserRecord(Key, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
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
			CloudSaveInterface->TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, true, UserRecord, TEXT(""));
		}
		else
		{
			CloudSaveInterface->TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, false, UserRecord, ErrorStr);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsSuccess(const FAccelByteModelsUserRecord& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UserRecord = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get user '%s' record Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteGetUserRecord::OnGetUserRecordsError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-get-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to get user record! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}