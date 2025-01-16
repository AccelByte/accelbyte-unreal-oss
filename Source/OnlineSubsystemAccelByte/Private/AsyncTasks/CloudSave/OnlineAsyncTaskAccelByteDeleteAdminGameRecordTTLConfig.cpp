// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig"

FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig::FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig(FOnlineSubsystemAccelByte* const InABInterface, const FString& InKey, int32 InLocalUserNum)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, LocalUserNum(InLocalUserNum)
{
}

void FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Deleting ttl config of admin game record, LocalUserNum: %d"), LocalUserNum);

	OnDeleteAdminGameRecordTTLConfigSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig::OnDeleteAdminGameRecordTTLConfigSuccess);
	OnDeleteAdminGameRecordTTLConfigErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig::OnDeleteAdminGameRecordTTLConfigError);

	SERVER_API_CLIENT_CHECK_GUARD()
	ServerApiClient->ServerCloudSave.DeleteAdminGameRecordTTLConfig(Key, OnDeleteAdminGameRecordTTLConfigSuccessDelegate, OnDeleteAdminGameRecordTTLConfigErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			CloudSaveInterface->TriggerOnDeleteAdminGameRecordTTLConfigCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::Success), Key);
		}
		else
		{
			CloudSaveInterface->TriggerOnDeleteAdminGameRecordTTLConfigCompletedDelegates(ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrorStr)), Key);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig::OnDeleteAdminGameRecordTTLConfigSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to delete ttl config of admin game record Success!"));
}

void FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig::OnDeleteAdminGameRecordTTLConfigError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = TEXT("request-failed-delete-ttl-config-of-admin-game-record");
	UE_LOG_AB(Warning, TEXT("Failed to delete ttl config of admin game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE