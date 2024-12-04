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

FOnlineAsyncTaskAccelByteBulkGetUserRecord::FOnlineAsyncTaskAccelByteBulkGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FString& InKey, const TArray<FUniqueNetIdAccelByteUserRef>& InUniqueNetIds)
	:FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, UniqueNetIds(InUniqueNetIds)
{}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting user record"));

	OnGetUserRecordsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsUserRecord>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsSuccess);
	OnGetUserRecordsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsError);
	
	if (!IsRunningDedicatedServer())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, access denied!"));
		return;
	}

	if (UniqueNetIds.Num() <= 0)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, userId empty!"));
		return;
	}


	if (UniqueNetIds.Num() > MaxUserIdsLimit)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = TEXT("request-failed-get-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, userIds exceed limit!"));
		return;
	}

	const auto ABUserIds = ConvertUniqueNetIdsToAccelByteIds();
	const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	TArray<FString> ProcessedIds{};
	for (int i =0 ; i < ABUserIds.Num(); i++)
	{
		ProcessedIds.Add(ABUserIds[i]);
		if (ProcessedIds.Num() >= GameServerApi::ServerCloudSave::BulkMaxUserCount || (i == (ABUserIds.Num()-1)))
		{
			const auto Ids = ProcessedIds;
			ServerApiClient->ServerCloudSave.BulkGetUserRecord(Key, Ids, OnGetUserRecordsSuccessDelegate, OnGetUserRecordsErrorDelegate);
			{
				RequestCount.Increment();
			}
			ProcessedIds.Reset();
		}
	}

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
			CloudSaveInterface->TriggerOnBulkGetUserRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrorStr)), Key, FBulkGetUserRecordMap{});
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

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsSuccess(const TArray<FAccelByteModelsUserRecord>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UserRecords.Append(ConstructBulkGetRecordResponseModel(Result));
	{
		RequestCount.Decrement();
		if (RequestCount.GetValue() <= 0)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get user record Success!"));
}

void FOnlineAsyncTaskAccelByteBulkGetUserRecord::OnGetUserRecordsError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = (TEXT("request-failed-get-user-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to get user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);

	{
		RequestCount.Decrement();
		if (RequestCount.GetValue() <= 0)
		{
			//Still return success with 0 data
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
	}
}

TArray<FString> FOnlineAsyncTaskAccelByteBulkGetUserRecord::ConvertUniqueNetIdsToAccelByteIds()
{
	TArray<FString> ABUserIds;
	for (const auto& NetId : UniqueNetIds)
	{
		ABUserIds.Add(NetId->GetAccelByteId());
	}
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
			*NetId = FUniqueNetIdAccelByteUser::Create(UserRecord.UserId);
		}
		Records.Add(*NetId, UserRecord);
	}
	return Records;
}

#undef ONLINE_ERROR_NAMESPACE