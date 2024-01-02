// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReplaceUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteReplaceUserRecord"

FOnlineAsyncTaskAccelByteReplaceUserRecord::FOnlineAsyncTaskAccelByteReplaceUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, const FJsonObject& InUserRecordObj, bool IsPublic, int32 InLocalUserNum, const FString& InTargetUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, UserRecordObj(InUserRecordObj)
	, IsPublicRecord(IsPublic)
	, TargetUserId(InTargetUserId)
	, LocalUserNum(InLocalUserNum)
{
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("replacing user record, UserId: %s"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);

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

		if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, not logged in!"));
			return;
		}

		const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		ServerApiClient->ServerCloudSave.ReplaceUserRecord(Key, ESetByMetadataRecord::SERVER, IsPublicRecord, TargetUserId, UserRecordObj, OnReplaceUserRecordSuccessDelegate, OnReplaceUserRecordErrorDelegate);
		SetBy = FAccelByteUtilities::GetUEnumValueAsString(ESetByMetadataRecord::SERVER);
	}
	else
	{
		ApiClient->CloudSave.ReplaceUserRecord(Key, IsPublicRecord, UserRecordObj, OnReplaceUserRecordSuccessDelegate, OnReplaceUserRecordErrorDelegate);
		SetBy = FAccelByteUtilities::GetUEnumValueAsString(ESetByMetadataRecord::CLIENT);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::Finalize()
{
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsPlayerRecordUpdatedPayload PlayerRecordUpdatedPayload{};
		PlayerRecordUpdatedPayload.Key = Key;
		PlayerRecordUpdatedPayload.IsPublic = IsPublicRecord;
		PlayerRecordUpdatedPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TargetUserId;
		PlayerRecordUpdatedPayload.SetBy = SetBy;
		PlayerRecordUpdatedPayload.Strategy = TEXT("REPLACE");
		PlayerRecordUpdatedPayload.Value.JsonObject = MakeShared<FJsonObject>(UserRecordObj);
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordUpdatedPayload>(PlayerRecordUpdatedPayload));
	}
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, ErrorStr), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to replace user '%s' record Success!"), UserId.IsValid() ? *UserId->ToDebugString() : *TargetUserId);
}

void FOnlineAsyncTaskAccelByteReplaceUserRecord::OnReplaceUserRecordsError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = FText::FromString(TEXT("request-failed-replace-user-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to replace user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE