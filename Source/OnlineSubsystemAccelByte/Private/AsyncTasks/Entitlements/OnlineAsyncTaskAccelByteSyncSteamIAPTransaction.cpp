// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncSteamIAPTransaction.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction"

FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction::FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InLocalUserId
	, const TSharedRef<FPurchaseReceipt>& InPurchaseReceipt
	, const FOnRequestCompleted& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PurchaseReceipt(InPurchaseReceipt)
	, CompletionDelegate(InCompletionDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s; OrderId: %s"), *UserId->ToDebugString(), *PurchaseReceipt->TransactionId);

	API_FULL_CHECK_GUARD(Entitlement, OnlineError);

	FAccelByteModelsSyncSteamIAPTransactionRequest Request{};
	Request.OrderId = PurchaseReceipt->TransactionId;

	const THandler<FAccelByteModelsSyncSteamIAPTransactionResponse> OnSyncSteamIAPTransactionSuccessDelegate =
		TDelegateUtils<THandler<FAccelByteModelsSyncSteamIAPTransactionResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction::OnSyncSteamIAPTransactionSuccess);

	const FErrorHandler OnSyncSteamIAPTransactionErrorDelegate =
		TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction::OnSyncSteamIAPTransactionError);
	
	Entitlement->SyncSteamIAPTransaction(Request
		, OnSyncSteamIAPTransactionSuccessDelegate
		, OnSyncSteamIAPTransactionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompletionDelegate.ExecuteIfBound(bWasSuccessful, OnlineError.ErrorCode);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction::OnSyncSteamIAPTransactionSuccess(const FAccelByteModelsSyncSteamIAPTransactionResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (Response.IapOrderStatus == EAccelByteEntitlementIAPOrderStatus::FAILED ||
		Response.IapOrderStatus == EAccelByteEntitlementIAPOrderStatus::REVOKE_FAILED)
	{
		const FText ErrorText = FText::FromString(TEXT("sync-steam-iap-transaction-request-failed"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncSteamIAPTransaction::OnSyncSteamIAPTransactionError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("ErrorCode: %d; ErrorMessage: %s"), ErrorCode, *ErrorMessage);

	const FText ErrorText = FText::FromString(TEXT("sync-steam-iap-transaction-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
