// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteDeleteUserRecord"

FOnlineAsyncTaskAccelByteDeleteUserRecord::FOnlineAsyncTaskAccelByteDeleteUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, int32 InLocalUserNum, const FString& InTargetUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, TargetUserId(InTargetUserId)
	, LocalUserNum(InLocalUserNum)
{
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Deleting user record, UserId: %s"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);

	OnDeleteUserRecordSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordSuccess);
	OnDeleteUserRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordError);
	
	if (IsRunningDedicatedServer())
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
			ErrorStr = TEXT("request-failed-delete-user-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, identity interface is invalid!"));
			return;
		}

		if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
		{
			ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
			ErrorStr = TEXT("request-failed-delete-user-record-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user record, not logged in!"));
			return;
		}

		SERVER_API_CLIENT_CHECK_GUARD(ErrorStr);
		
		ServerApiClient->ServerCloudSave.DeleteUserRecord(Key, *TargetUserId, false, OnDeleteUserRecordSuccessDelegate, OnDeleteUserRecordErrorDelegate);
	}
	else
	{
		API_FULL_CHECK_GUARD(CloudSave,ErrorStr);
		CloudSave->DeleteUserRecord(Key, OnDeleteUserRecordSuccessDelegate, OnDeleteUserRecordErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsPlayerRecordDeletedPayload PlayerRecordDeletedPayload{};
		PlayerRecordDeletedPayload.Key = Key;
		PlayerRecordDeletedPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TargetUserId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordDeletedPayload>(PlayerRecordDeletedPayload));
	}
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrorStr)), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to delete user record for user '%s' Success!"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);
}

void FOnlineAsyncTaskAccelByteDeleteUserRecord::OnDeleteUserRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = TEXT("request-failed-delete-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to delete user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE