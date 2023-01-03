// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReplaceUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteReplaceUserRecord::FOnlineAsyncTaskAccelByteReplaceUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, const FJsonObject& InUserRecordObj, bool IsPublic)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, UserRecordObj(InUserRecordObj)
	, IsPublicRecord(IsPublic)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("replacing user record, UserId: %s"), *UserId->ToDebugString());

	OnReplaceUserRecordSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsSuccess);
	OnReplaceUserRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsError);
	
	if (IsRunningDedicatedServer())
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, identity interface is invalid!"));
			return;
		}

		if (IdentityInterface->GetLoginStatus(*UserId) != ELoginStatus::LoggedIn)
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, not logged in!"));
			return;
		}

		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		ServerApiClient->ServerCloudSave.ReplaceUserRecord(Key, ESetByMetadataRecord::SERVER, IsPublicRecord, UserId.Get()->GetAccelByteId(), UserRecordObj, OnReplaceUserRecordSuccessDelegate, OnReplaceUserRecordErrorDelegate);
	}
	else
	{
		ApiClient->CloudSave.ReplaceUserRecord(Key, IsPublicRecord, UserRecordObj, OnReplaceUserRecordSuccessDelegate, OnReplaceUserRecordErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, true, TEXT(""));
		}
		else
		{
			CloudSaveInterface->TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to replace user '%s' record Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-replace-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to replace user record! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}