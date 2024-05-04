// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetWalletTransactions.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineWalletInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetWalletTransactions::FOnlineAsyncTaskAccelByteGetWalletTransactions(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InCurrencyCode, int32 InOffset, int32 InLimit)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, ErrorStr(TEXT(""))
	, CachedWalletTransactions(TArray<FAccelByteModelsWalletTransactionInfo>{})
	, CurrencyCode(InCurrencyCode)
	, Offset(InOffset)
	, Limit(InLimit)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGetWalletTransactions::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting wallet transaction list, UserId: %s"), *UserId->ToDebugString());

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		// Create delegates for successfully as well as unsuccessfully requesting to get wallet transaction list
		OnGetWalletTransactionsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsWalletTransactionPaging>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetWalletTransactions::OnGetWalletTransactionsSuccess);
		OnGetWalletTransactionsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetWalletTransactions::OnGetWalletTransactionsError);

		// Send off a request to get wallet transaction list, as well as connect our delegates for doing so
		API_CLIENT_CHECK_GUARD(ErrorStr);
		ApiClient->Wallet.ListWalletTransactionsByCurrencyCode(CurrencyCode, OnGetWalletTransactionsSuccessDelegate, OnGetWalletTransactionsErrorDelegate);
	}
	else
	{
		ErrorStr = TEXT("request-failed-wallet-interface-invalid");
		UE_LOG_AB(Warning, TEXT("Failed to get wallet transaction list! wallet interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetWalletTransactions::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		WalletInterface->TriggerOnGetWalletTransactionsCompletedDelegates(LocalUserNum, bWasSuccessful, CachedWalletTransactions, ErrorStr);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetWalletTransactions::OnGetWalletTransactionsSuccess(const FAccelByteModelsWalletTransactionPaging& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		CachedWalletTransactions = Result.Data;
	}
	else
	{
		ErrorStr = TEXT("request-failed-wallet-interface-invalid");
		UE_LOG_AB(Warning, TEXT("Failed to get wallet transaction list! wallet interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get wallet transaction list for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteGetWalletTransactions::OnGetWalletTransactionsError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-get-wallet-transactions-error");
	UE_LOG_AB(Warning, TEXT("Failed to get wallet transaction list! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}