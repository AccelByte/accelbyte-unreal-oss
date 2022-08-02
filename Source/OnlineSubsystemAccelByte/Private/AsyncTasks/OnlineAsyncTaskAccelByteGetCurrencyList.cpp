// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetCurrencyList.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineWalletInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteGetCurrencyList::FOnlineAsyncTaskAccelByteGetCurrencyList(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteGetCurrencyList::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting currency list, UserId: %s"), *UserId->ToDebugString());

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		if (WalletInterface->CurrencyCodeToCurrencyListMap.Num() == 0 || bAlwaysRequestToService)
		{	
			// Create delegates for successfully as well as unsuccessfully requesting to get currency list
			OnGetCurrencyListSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsCurrencyList>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetCurrencyList::OnGetCurrencyListSuccess);
			OnGetCurrencyListErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetCurrencyList::OnGetCurrencyListError);

			// Send off a request to get currency list, as well as connect our delegates for doing so
			ApiClient->Currency.GetCurrencyList(ApiClient->CredentialsRef->GetNamespace(), OnGetCurrencyListSuccessDelegate, OnGetCurrencyListErrorDelegate);
		}
		else
		{
			if (WalletInterface->GetAllCurrencyFromCache(CachedCurrencyList))
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			}
			else
			{
				ErrorStr = TEXT("request-failed-currency-list-invalid");
				UE_LOG_AB(Warning, TEXT("Failed to get currency list! currency list is invalid!"));
				CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			}
		}
	}
	else
	{
		ErrorStr = TEXT("request-failed-wallet-interface-invalid");
		UE_LOG_AB(Warning, TEXT("Failed to get currency list! wallet interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCurrencyList::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			WalletInterface->TriggerOnGetCurrencyListCompletedDelegates(LocalUserNum, bWasSuccessful, CachedCurrencyList, TEXT(""));
		}
		else
		{
			WalletInterface->TriggerOnGetCurrencyListCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsCurrencyList>{}, ErrorStr);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCurrencyList::OnGetCurrencyListSuccess(const TArray<FAccelByteModelsCurrencyList>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		for (auto Currency : Result)
		{
			WalletInterface->AddCurrencyToList(Currency.CurrencyCode, MakeShared<FAccelByteModelsCurrencyList>(Currency));
		}
		CachedCurrencyList = Result;
	}
	else
	{
		ErrorStr = TEXT("request-failed-wallet-interface-invalid");
		UE_LOG_AB(Warning, TEXT("Failed to get currency list! wallet interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get currency list for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteGetCurrencyList::OnGetCurrencyListError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-get-currency-list-error");
	UE_LOG_AB(Warning, TEXT("Failed to get currency list! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}