// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineError.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteDeleteUserRecord"

FOnlineAsyncTaskAccelByteDeleteUserRecord::FOnlineAsyncTaskAccelByteDeleteUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Deleting user record, UserId: %s"), *UserId->ToDebugString());

	OnDeleteUserRecordSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordSuccess);
	OnDeleteUserRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordError);
	
	if (IsRunningDedicatedServer())
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
			ErrorStr = FText::FromString(TEXT("request-failed-delete-user-record-error"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, identity interface is invalid!"));
			return;
		}

		if (IdentityInterface->GetLoginStatus(*UserId) != ELoginStatus::LoggedIn)
		{
			ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
			ErrorStr = FText::FromString(TEXT("request-failed-delete-user-record-error"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, not logged in!"));
			return;
		}

		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		ServerApiClient->ServerCloudSave.DeleteUserRecord(Key, *UserId.Get()->GetAccelByteId(), false, OnDeleteUserRecordSuccessDelegate, OnDeleteUserRecordErrorDelegate);
	}
	else
	{
		ApiClient->CloudSave.DeleteUserRecord(Key, OnDeleteUserRecordSuccessDelegate, OnDeleteUserRecordErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, ErrorStr), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to delete user record for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = FText::FromString(TEXT("request-failed-delete-user-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to delete user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}