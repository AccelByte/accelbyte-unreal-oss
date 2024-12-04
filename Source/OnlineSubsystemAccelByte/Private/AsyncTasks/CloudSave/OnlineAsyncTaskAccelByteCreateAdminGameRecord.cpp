// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateAdminGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteLoginServer.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteCreateAdminGameRecord"

FOnlineAsyncTaskAccelByteCreateAdminGameRecord::FOnlineAsyncTaskAccelByteCreateAdminGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, const FJsonObject& InGameRecordObj, const TArray<FString> InTags, const FTTLConfig& InTTLConfig)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, GameRecordObj(InGameRecordObj)
	, Tags(InTags)
	, TTLConfig(InTTLConfig)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteCreateAdminGameRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Creating game record, LocalUserNum: %d"), LocalUserNum);

	OnCreateAdminGameRecordSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsAdminGameRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateAdminGameRecord::OnCreateAdminGameRecordSuccess);
	OnCreateAdminGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateAdminGameRecord::OnCreateAdminGameRecordError);

	if (!IsRunningDedicatedServer())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = FText::FromString(TEXT("request-failed-create-admin-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game record, access denied!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-create-admin-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create admin game record, identity interface is invalid!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-create-admin-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create admin game record, not logged in!"));
		return;
	}

	FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	ServerApiClient->ServerCloudSave.CreateAdminGameRecord(Key, GameRecordObj, OnCreateAdminGameRecordSuccessDelegate, OnCreateAdminGameRecordErrorDelegate, Tags, TTLConfig);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateAdminGameRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnModifyAdminGameRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::Success),Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnModifyAdminGameRecordCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, ErrorStr),Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateAdminGameRecord::OnCreateAdminGameRecordSuccess(const FAccelByteModelsAdminGameRecord& Result)
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		CloudSaveInterface->AddAdminGameRecordToMap(Key, MakeShared<FAccelByteModelsAdminGameRecord>(Result));
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to create admin game record Success!"));
}

void FOnlineAsyncTaskAccelByteCreateAdminGameRecord::OnCreateAdminGameRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = FText::FromString(TEXT("request-failed-create-admin-game-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to create admin game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE