// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReplaceGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteLoginServer.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteReplaceGameRecord"

FOnlineAsyncTaskAccelByteReplaceGameRecord::FOnlineAsyncTaskAccelByteReplaceGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, const FJsonObject& InGameRecordObj)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, GameRecordObj(InGameRecordObj)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Replacing game record, LocalUserNum: %d"), LocalUserNum);

	OnReplaceGameRecordSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceGameRecord::OnReplaceGameRecordSuccess);
	OnReplaceGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceGameRecord::OnReplaceGameRecordError);

	if (!IsRunningDedicatedServer())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = FText::FromString(TEXT("request-failed-replace-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace game record, access denied!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-replace-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace game record, identity interface is invalid!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-replace-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace game record, not logged in!"));
		return;
	}

	FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	ServerApiClient->ServerCloudSave.ReplaceGameRecord(Key, GameRecordObj, OnReplaceGameRecordSuccessDelegate, OnReplaceGameRecordErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::Finalize()
{
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGameRecordUpdatedPayload GameRecordUpdatedPayload{};
		GameRecordUpdatedPayload.Key = Key;
		GameRecordUpdatedPayload.SetBy = FAccelByteUtilities::GetUEnumValueAsString(ESetByMetadataRecord::SERVER);
		GameRecordUpdatedPayload.Strategy = TEXT("REPLACE");
		GameRecordUpdatedPayload.Value.JsonObject = MakeShared<FJsonObject>(GameRecordObj);
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGameRecordUpdatedPayload>(GameRecordUpdatedPayload));
	}
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnReplaceGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Success),Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnReplaceGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, ErrorStr),Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::OnReplaceGameRecordSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		FAccelByteModelsGameRecord GameRecord;
		GameRecord.UpdatedAt = FDateTime::Now().GetDate();
		GameRecord.Key = Key;
		GameRecord.SetBy = ESetByMetadataRecord::SERVER;
		FJsonObjectWrapper GameRecordObjWrapper{};
		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(MakeShared<FJsonObject>(GameRecordObj), Writer);
		GameRecordObjWrapper.JsonObjectFromString(OutputString);
		GameRecord.Value = GameRecordObjWrapper;
		CloudSaveInterface->AddGameRecordToMap(Key, MakeShared<FAccelByteModelsGameRecord>(GameRecord));
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to replace game record Success!"));
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::OnReplaceGameRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = FText::FromString(TEXT("request-failed-replace-game-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to replace game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE