// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncMetaSubscription.h"

#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

FOnlineAsyncTaskAccelByteSyncMetaSubscription::FOnlineAsyncTaskAccelByteSyncMetaSubscription(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, const FAccelByteModelsSyncOculusSubscriptionRequest& Request
	, const FOnSyncMetaSubscriptionCompleteDelegate& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SyncRequest(Request)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteSyncMetaSubscription::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	API_FULL_CHECK_GUARD(Entitlement);

	auto OnSuccess = AccelByte::TDelegateUtils <THandler<TArray<FAccelByteModelsThirdPartySubscriptionTransactionInfo >>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncMetaSubscription::OnSyncOculusSubscriptionSuccess);
	auto OnError = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncMetaSubscription::OnSyncOculusSubscriptionFailed);
	Entitlement->SyncOculusSubscription(SyncRequest, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaSubscription::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	bool bDelegateTriggered = Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, SyncSubscriptionResponse, ErrorInfo);

	if (!bDelegateTriggered)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate as it is unbound."));
		return;
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaSubscription::OnSyncOculusSubscriptionSuccess(const TArray<FAccelByteModelsThirdPartySubscriptionTransactionInfo>& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	SyncSubscriptionResponse = Response;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaSubscription::OnSyncOculusSubscriptionFailed(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	ErrorInfo = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to sync Oculus subscription, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}
