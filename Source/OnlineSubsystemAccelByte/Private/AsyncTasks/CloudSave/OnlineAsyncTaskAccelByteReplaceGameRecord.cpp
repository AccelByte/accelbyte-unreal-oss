// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReplaceGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteLoginServer.h"

FOnlineAsyncTaskAccelByteReplaceGameRecord::FOnlineAsyncTaskAccelByteReplaceGameRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, const FJsonObject& InGameRecordObj)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, GameRecordObj(InGameRecordObj)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Replacing game record, UserId: %s"), *UserId->ToDebugString());

	OnReplaceGameRecordSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceGameRecord::OnReplaceGameRecordSuccess);
	OnReplaceGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceGameRecord::OnReplaceGameRecordError);

	if (!IsRunningDedicatedServer())
	{
		ErrorStr = TEXT("request-failed-replace-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace game record, access denied!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorStr = TEXT("request-failed-replace-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace game record, identity interface is invalid!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(*UserId) != ELoginStatus::LoggedIn)
	{
		ErrorStr = TEXT("request-failed-replace-game-record-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace game record, not logged in!"));
		return;
	}

	FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	ServerApiClient->ServerCloudSave.ReplaceGameRecord(Key, GameRecordObj, OnReplaceGameRecordSuccessDelegate, OnReplaceGameRecordErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = Subsystem->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnReplaceGameRecordCompletedDelegates(LocalUserNum, true, TEXT(""));
		}
		else
		{
			CloudSaveInterface->TriggerOnReplaceGameRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
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
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to replace game record for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteReplaceGameRecord::OnReplaceGameRecordError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-replace-game-record-error");
	UE_LOG_AB(Warning, TEXT("Failed to replace game record! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}