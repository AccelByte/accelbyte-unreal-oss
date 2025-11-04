// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncMetaQuestDLC.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSyncMetaQuestDLC::FOnlineAsyncTaskAccelByteSyncMetaQuestDLC(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestDLC::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	API_FULL_CHECK_GUARD(Entitlement);

	const FVoidHandler OnSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncMetaQuestDLC::OnSyncOculusDLCSuccess);
	const FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncMetaQuestDLC::OnSyncOculusDLCFailed);
	Entitlement->SyncOculusDLC(OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestDLC::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	bool bDelegateTriggered = Delegate.ExecuteIfBound(bWasSuccessful, ErrorInfo);

	if (!bDelegateTriggered)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate as it is unbound."));
		return;
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestDLC::OnSyncOculusDLCSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestDLC::OnSyncOculusDLCFailed(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	ErrorInfo = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to sync oculus DLC, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}
