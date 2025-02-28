// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReplaceAdminGameRecord.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteLoginServer.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteReplaceAdminGameRecord"

FOnlineAsyncTaskAccelByteReplaceAdminGameRecord::FOnlineAsyncTaskAccelByteReplaceAdminGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, const FJsonObject& InGameRecordObj, const FTTLConfig& InTTLConfig)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Key(InKey)
	, GameRecordObj(InGameRecordObj)
	, TTLConfig(InTTLConfig)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteReplaceAdminGameRecord::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Replacing game record, LocalUserNum: %d"), LocalUserNum);

	OnReplaceAdminGameRecordSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsAdminGameRecord>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceAdminGameRecord::OnReplaceAdminGameRecordSuccess);
	OnReplaceAdminGameRecordErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReplaceAdminGameRecord::OnReplaceAdminGameRecordError);

	if (!IsRunningDedicatedServer())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		ErrorStr = FText::FromString(TEXT("request-failed-replace-admin-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace game record, access denied!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-replace-admin-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace admin game record, identity interface is invalid!"));
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		ErrorStr = FText::FromString(TEXT("request-failed-replace-admin-game-record-error"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to replace admin game record, not logged in!"));
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD(ErrorStr);
	
	ServerApiClient->ServerCloudSave.ReplaceAdminGameRecord(Key, GameRecordObj, OnReplaceAdminGameRecordSuccessDelegate, OnReplaceAdminGameRecordErrorDelegate, TTLConfig);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReplaceAdminGameRecord::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

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

void FOnlineAsyncTaskAccelByteReplaceAdminGameRecord::OnReplaceAdminGameRecordSuccess(const FAccelByteModelsAdminGameRecord& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	const FOnlineCloudSaveAccelBytePtr CloudSaveInterface = SubsystemPin->GetCloudSaveInterface();
	if (CloudSaveInterface.IsValid())
	{
		CloudSaveInterface->AddAdminGameRecordToMap(Key, MakeShared<FAccelByteModelsAdminGameRecord>(Result));
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to replace admin game record Success!"));
}

void FOnlineAsyncTaskAccelByteReplaceAdminGameRecord::OnReplaceAdminGameRecordError(int32 Code, const FString& ErrorMessage)
{
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorStr = FText::FromString(TEXT("request-failed-replace-admin-game-record-error"));
	UE_LOG_AB(Warning, TEXT("Failed to replace admin game record! Error Code: %d; Error Message: %s"), Code, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE