// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkReplaceUserRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteBulkReplaceUserRecord"

FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::FOnlineAsyncTaskAccelByteBulkReplaceUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FString& InKey, const TMap<FUniqueNetIdAccelByteUserRef, FJsonObject>& InRequest)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, Request(InRequest)
{}

void FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("replacing user record"));

	OnReplaceUserRecordSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsBulkReplaceUserRecordResponse>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::OnReplaceUserRecordsSuccess);
	OnReplaceUserRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::OnReplaceUserRecordsError);
	
	if (!IsRunningDedicatedServer())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = TEXT("request-failed-replace-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, access denied!"));
		return;
	}

	if (Request.Num() <= 0)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = TEXT("request-failed-replace-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, request empty!"));
		return;
	}

	if (Request.Num() > MaxUserIdsLimit)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = TEXT("request-failed-replace-user-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace user record, request exceed limit!"));
		return;
	}

	const auto ReplaceRequest = ConstructBulkReplaceRequestModel();

	SERVER_API_CLIENT_CHECK_GUARD(ErrorStr);

	TArray<FAccelByteModelsBulkReplaceUserRecordRequestDetail> ProcessedIds{};

	for (int i = 0; i < ReplaceRequest.Data.Num(); i++)
	{
		ProcessedIds.Add(ReplaceRequest.Data[i]);
		if (ProcessedIds.Num() == GameServerApi::ServerCloudSave::BulkMaxUserCount || (i == (ReplaceRequest.Data.Num() - 1)))
		{
			FAccelByteModelsBulkReplaceUserRecordRequest SendRequest = { ProcessedIds };
			ServerApiClient->ServerCloudSave.BulkReplaceUserRecord(Key, SendRequest, OnReplaceUserRecordSuccessDelegate, OnReplaceUserRecordErrorDelegate);
			{
				RequestCount.Increment();
			}
			ProcessedIds.Reset();
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::Finalize()
{
	TRY_PIN_SUBSYSTEM();
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		for (const auto& RecordResult : Result)
		{
			if (RecordResult.Value.Success)
			{
				FAccelByteModelsPlayerRecordUpdatedPayload PlayerRecordUpdatedPayload{};
				PlayerRecordUpdatedPayload.Key = Key;
				PlayerRecordUpdatedPayload.UserId = RecordResult.Value.User_id;
				PlayerRecordUpdatedPayload.Strategy = TEXT("REPLACE");
				const auto* Object = Request.Find(RecordResult.Key);
				if (Object != nullptr)
				{
					PlayerRecordUpdatedPayload.Value.JsonObject = MakeShared<FJsonObject>(*Object);
				}
				PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerRecordUpdatedPayload>(PlayerRecordUpdatedPayload));
			}
		}
	}
}

void FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	TRY_PIN_SUBSYSTEM();
	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnBulkReplaceUserRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::Success), Key, Result);
		}
		else
		{
			CloudSaveInterface->TriggerOnBulkReplaceUserRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrorStr)), Key, Result);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::OnReplaceUserRecordsSuccess(const TArray<FAccelByteModelsBulkReplaceUserRecordResponse>& InResponse)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Result.Append(ConstructBulkReplaceResponseModel(InResponse));

	{
		RequestCount.Decrement();
		if (RequestCount.GetValue() <= 0)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to replace user record Success!"));
}

void FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::OnReplaceUserRecordsError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = TEXT("request-failed-replace-user-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to replace user record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);

	for (const auto& Record : Request)
	{
		Result.Add(Record.Key, FAccelByteModelsBulkReplaceUserRecordResponse{});
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	{
		RequestCount.Decrement();
		if (RequestCount.GetValue() <= 0)
		{
			//Still return success with 0 data
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
	}
}

FAccelByteModelsBulkReplaceUserRecordRequest FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::ConstructBulkReplaceRequestModel()
{
	FAccelByteModelsBulkReplaceUserRecordRequest ABRequest;
	for (const auto& Record : Request)
	{
		FAccelByteModelsBulkReplaceUserRecordRequestDetail Detail;
		Detail.User_id = Record.Key->GetAccelByteId();
		Detail.Value = FJsonObjectWrapper{};
		Detail.Value.JsonObject = MakeShared<FJsonObject>(Record.Value);
		ABRequest.Data.Add(Detail);
	}
	return ABRequest;
}

FBulkReplaceUserRecordMap FOnlineAsyncTaskAccelByteBulkReplaceUserRecord::ConstructBulkReplaceResponseModel(const TArray<FAccelByteModelsBulkReplaceUserRecordResponse> & Response)
{
	TArray<FUniqueNetIdAccelByteUserRef> UniqueNetIds;
	Request.GetKeys(UniqueNetIds);

	FBulkReplaceUserRecordMap ResultMap;
	for (const auto& RecordResult : Response)
	{
		auto NetId = UniqueNetIds.FindByPredicate([RecordResult](const FUniqueNetIdAccelByteUserRef& UniqueNetId)
			{
				return UniqueNetId->GetAccelByteId() == RecordResult.User_id;
			});
		if (NetId == nullptr)
		{
			ResultMap.Add(FUniqueNetIdAccelByteUser::Create(RecordResult.User_id), RecordResult);
		}
		else
		{
			ResultMap.Add(*NetId, RecordResult);
		}
	}
	return ResultMap;
}

#undef ONLINE_ERROR_NAMESPACE