// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig"

FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig::FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig(FOnlineSubsystemAccelByte* const InABInterface, const FString& InKey, int32 InLocalUserNum)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, LocalUserNum(InLocalUserNum)
{
}

void FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Deleting ttl config of game record, LocalUserNum: %d"), LocalUserNum);

	OnDeleteGameRecordTTLConfigSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig::OnDeleteGameRecordTTLConfigSuccess);
	OnDeleteGameRecordTTLConfigErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig::OnDeleteGameRecordTTLConfigError);

	const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	ServerApiClient->ServerCloudSave.DeleteGameRecordTTLConfig(Key, OnDeleteGameRecordTTLConfigSuccessDelegate, OnDeleteGameRecordTTLConfigErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnDeleteGameRecordTTLConfigCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnDeleteGameRecordTTLConfigCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrorStr)), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig::OnDeleteGameRecordTTLConfigSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to delete ttl config of game record Success!"));
}

void FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig::OnDeleteGameRecordTTLConfigError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = TEXT("request-failed-delete-ttl-config-of-game-record");
	UE_LOG_AB(Warning, TEXT("Failed to delete ttl config of game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE