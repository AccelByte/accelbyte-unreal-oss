// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction"

FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction::FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InLocalUserId
	, const FOnSyncSteamAbnormalIAPTransactionComplete& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, CompletionDelegate(InCompletionDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s"), *UserId->ToDebugString());

	API_FULL_CHECK_GUARD(Entitlement, OnlineError);

	const FVoidHandler OnSyncSteamAbnormalIAPTransactionSuccessDelegate =
		TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction::OnSyncSteamAbnormalIAPTransactionSuccess);

	const FErrorHandler OnSyncSteamAbnormalIAPTransactionErrorDelegate =
		TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction::OnSyncSteamAbnormalIAPTransactionError);
	
	Entitlement->SyncSteamAbnormalIAPTransaction(OnSyncSteamAbnormalIAPTransactionSuccessDelegate
		, OnSyncSteamAbnormalIAPTransactionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompletionDelegate.ExecuteIfBound(UserId.ToSharedRef().Get(), OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction::OnSyncSteamAbnormalIAPTransactionSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncSteamAbnormalIAPTransaction::OnSyncSteamAbnormalIAPTransactionError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("ErrorCode: %d; ErrorMessage: %s"), ErrorCode, *ErrorMessage);

	const FText ErrorText = FText::FromString(TEXT("sync-steam-abnormal-iap-transaction-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
