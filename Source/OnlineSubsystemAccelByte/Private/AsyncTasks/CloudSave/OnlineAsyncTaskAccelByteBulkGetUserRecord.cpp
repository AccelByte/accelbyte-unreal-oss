// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkGetUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteBulkGetUserRecord"

FOnlineAsyncTaskAccelByteBulkGetUserRecord::FOnlineAsyncTaskAccelByteBulkGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface
	, FString const& InKey
	, TArray<FUniqueNetIdAccelByteUserRef> const& InUniqueNetIds)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Key(InKey)
	, UniqueNetIds(InUniqueNetIds)
{}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::Initialize()
{
	Super::Initialize();

	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting user record"));

	if (UniqueNetIds.Num() <= 0)
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, userId empty!"));
		return;
	}

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet() || !IsDS.GetValue())
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, not implemented!"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		TaskOnlineError = EOnlineErrorResult::MissingInterface;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-missing-interface");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, missing interface!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		TaskOnlineError = EOnlineErrorResult::AccessDenied;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		TaskErrorStr = TEXT("request-failed-unauthorized");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Server not authorized'!"));
		return;
	}

	OnGetUserRecordsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsUserRecord>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsSuccess);
	OnGetUserRecordsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsError);

	BulkGetUserRecords(Key);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	TRY_PIN_SUBSYSTEM();
	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnBulkGetUserRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::Success), Key, UserRecords);
		}
		else
		{
			CloudSaveInterface->TriggerOnBulkGetUserRecordCompletedDelegates(ONLINE_ERROR(TaskOnlineError, TaskErrorCode, FText::FromString(TaskErrorStr)), Key, FBulkGetUserRecordMap{});
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	TRY_PIN_SUBSYSTEM();
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		for (const auto& Record : UserRecords)
		{
			if (Record.Value.IsPublic)
			{
				FAccelByteModelsPublicPlayerRecordGetRecordPayload PublicPlayerRecordGetRecordPayload{};
				PublicPlayerRecordGetRecordPayload.UserId = Record.Value.UserId;
				PublicPlayerRecordGetRecordPayload.Key = Key;
				PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPublicPlayerRecordGetRecordPayload>(PublicPlayerRecordGetRecordPayload));
			}
			else
			{
				FAccelByteModelsPlayerRecordGetRecordsPayload PlayerRecordGetRecordsPayload{};
				PlayerRecordGetRecordsPayload.UserId = Record.Value.UserId;
				PlayerRecordGetRecordsPayload.Keys.Add(Key);
				PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordGetRecordsPayload>(PlayerRecordGetRecordsPayload));
			}
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::BulkGetUserRecords(FString const& InKey)
{
	TArray<FString> ABUserIds = ConvertUniqueNetIdsToAccelByteIds(GameServerApi::ServerCloudSave::BulkMaxUserCount);

	SERVER_API_CLIENT_CHECK_GUARD(TaskErrorStr);

	if (Offset > 0 && ABUserIds.Num() > 0)
	{
		ServerApiClient->ServerCloudSave.BulkGetUserRecord(InKey, ABUserIds, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
	}
	else
	{
		int32 Code = static_cast<int32>(AccelByte::ErrorCodes::StatusBadRequest);
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorCode = FString::Printf(TEXT("%d"), Code);
		TaskErrorStr = (TEXT("request-failed-get-user-record-error"));
		OnGetUserRecordsErrorDelegate.ExecuteIfBound(Code, TEXT("Request failed empty userIds"));
	}
}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsSuccess(const TArray<FAccelByteModelsUserRecord>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UserRecords.Append(ConstructBulkGetRecordResponseModel(Result));

	if (Offset < UniqueNetIds.Num())
	{
		BulkGetUserRecords(Key);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get user record Success!"));
}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsError(int32 Code, FString const& ErrorMessage)
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorCode = FString::Printf(TEXT("%d"), Code);
	TaskErrorStr = (TEXT("request-failed-get-user-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to get user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);

	//Still return success with 0 data
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

TArray<FString> FOnlineAsyncTaskAccelByteBulkGetUserRecord::ConvertUniqueNetIdsToAccelByteIds(int32 Limit)
{
	TArray<FString> ABUserIds;
	for (int32 Index = Offset, Count = 0; Index < UniqueNetIds.Num() && Count < Limit; Index++, Count++)
	{
		FUniqueNetIdAccelByteUserRef NetId = UniqueNetIds[Index];
		ABUserIds.Add(NetId->GetAccelByteId());
		Offset = Index;
	}
	Offset++;
	return ABUserIds;
}

FBulkGetUserRecordMap FOnlineAsyncTaskAccelByteBulkGetUserRecord::ConstructBulkGetRecordResponseModel(const TArray<FAccelByteModelsUserRecord>& Result)
{
	FBulkGetUserRecordMap Records;
	for (const auto& UserRecord : Result)
	{
		auto NetId = UniqueNetIds.FindByPredicate([UserRecord](const FUniqueNetIdAccelByteUserRef& UniqueNetId)
			{
				return UniqueNetId->GetAccelByteId() == UserRecord.UserId;
			});
		if (NetId == nullptr)
		{
			Records.Add(FUniqueNetIdAccelByteUser::Create(UserRecord.UserId), UserRecord);
		}
		else 
		{
			Records.Add(*NetId, UserRecord);
		}
	}
	return Records;
}

#undef ONLINE_ERROR_NAMESPACE