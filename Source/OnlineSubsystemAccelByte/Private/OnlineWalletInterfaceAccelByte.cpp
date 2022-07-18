// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineWalletInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteGetCurrencyList.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteGetWalletInfo.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteGetWalletTransactions.h"

bool FOnlineWalletAccelByte::GetCurrencyList(int32 LocalUserNum, bool bAlwaysRequestToService)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Get Currency List, LocalUserNum: %d"), LocalUserNum);

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetCurrencyList>(AccelByteSubsystem, *UserIdPtr.Get(), bAlwaysRequestToService);
				AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get currency list!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-currency-list-failed-userid-invalid");
				AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetCurrencyListCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsCurrencyList>{}, ErrorStr);

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-currency-list-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetCurrencyListCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsCurrencyList>{}, ErrorStr);

	return false;
}

bool FOnlineWalletAccelByte::GetCurrencyFromCache(const FString& CurrencyCode, FAccelByteModelsCurrencyList& OutCurrency)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Get Currency, CurrencyCode: %s"), *CurrencyCode);
	FScopeLock ScopeLock(&CurrencyListLock);

	if (CurrencyCodeToCurrencyListMap.Num() > 0)
	{
		auto Currency = CurrencyCodeToCurrencyListMap.Find(CurrencyCode);
		if (Currency != nullptr)
		{
			OutCurrency = Currency->Get();
			AB_OSS_INTERFACE_TRACE_END(TEXT("Currency found for currency code %s"), *CurrencyCode);
			return true;
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Currency not found for currency code %s"), *CurrencyCode);
	return false;
}

bool FOnlineWalletAccelByte::GetAllCurrencyFromCache(TArray<FAccelByteModelsCurrencyList>& OutCurrencyList)
{
	FScopeLock ScopeLock(&CurrencyListLock);

	if (CurrencyCodeToCurrencyListMap.Num() > 0)
	{
		for (const auto& Currency : CurrencyCodeToCurrencyListMap)
		{
			OutCurrencyList.Add(Currency.Value.Get());
		}
		return true;
	}

	return false;
}

void FOnlineWalletAccelByte::AddCurrencyToList(const FString& CurrencyCode, const TSharedRef<FAccelByteModelsCurrencyList>& InCurrencyList)
{
	FScopeLock ScopeLock(&CurrencyListLock);

	CurrencyCodeToCurrencyListMap.Add(CurrencyCode, InCurrencyList);
}

bool FOnlineWalletAccelByte::GetWalletInfoByCurrencyCode(int32 LocalUserNum, const FString& CurrencyCode, bool bAlwaysRequestToService)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Get Wallet Info, LocalUserNum: %d"), LocalUserNum);

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetWalletInfo>(AccelByteSubsystem, *UserIdPtr.Get(), CurrencyCode, bAlwaysRequestToService);
				AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get wallet info!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-wallet-info-failed-userid-invalid");
				AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetWalletInfoCompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfo{}, ErrorStr);

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-wallet-info-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetWalletInfoCompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfo{}, ErrorStr);

	return false;
}

bool FOnlineWalletAccelByte::GetWalletInfoFromCache(int32 LocalUserNum, const FString& CurrencyCode, FAccelByteModelsWalletInfo& OutWalletInfo)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		FScopeLock ScopeLock(&WalletInfoListLock);
		TMap<FString, TSharedRef<FAccelByteModelsWalletInfo>> CurrencyToWalletInfoMap = UserToWalletInfoMap.FindRef(IdentityInterface->GetUniquePlayerId(LocalUserNum)->AsShared());
		if (CurrencyToWalletInfoMap.Num() > 0)
		{
			TSharedRef<FAccelByteModelsWalletInfo>* WalletInfo = CurrencyToWalletInfoMap.Find(CurrencyCode);
			if (WalletInfo != nullptr)
			{
				OutWalletInfo = WalletInfo->Get();
				return true;
			}
		}
	}

	return false;
}

void FOnlineWalletAccelByte::AddWalletInfoToList(int32 LocalUserNum, const FString& CurrencyCode, const TSharedRef<FAccelByteModelsWalletInfo>& InWalletInfo)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (UserIdPtr.IsValid())
			{
				FScopeLock ScopeLock(&WalletInfoListLock);
				auto CurrencyCodeToWalletInfo = UserToWalletInfoMap.FindRef(UserIdPtr.ToSharedRef());
				CurrencyCodeToWalletInfo.Emplace(CurrencyCode, InWalletInfo);
				UserToWalletInfoMap.Emplace(UserIdPtr.ToSharedRef(), CurrencyCodeToWalletInfo);
			}
		}
	}
}

bool FOnlineWalletAccelByte::ListWalletTransactionsByCurrencyCode(int32 LocalUserNum, const FString& CurrencyCode, int32 Offset, int32 Limit)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Get Wallet Transaction List, LocalUserNum: %d"), LocalUserNum);

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetWalletTransactions>(AccelByteSubsystem, *UserIdPtr.Get(), CurrencyCode, Offset, Limit);
				AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get wallet transaction list!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-wallet-transaction-list-failed-userid-invalid");
				AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetWalletTransactionsCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsWalletTransactionInfo>{}, ErrorStr);

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-wallet-transaction-list-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetWalletTransactionsCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsWalletTransactionInfo>{}, ErrorStr);

	return false;
}
