// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineWalletInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/Wallet/OnlineAsyncTaskAccelByteGetCurrencyList.h"
#include "AsyncTasks/Wallet/OnlineAsyncTaskAccelByteGetWalletInfo.h"
#include "AsyncTasks/Wallet/OnlineAsyncTaskAccelByteGetWalletTransactions.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/World.h"

FOnlineWalletAccelByte::FOnlineWalletAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{};

bool FOnlineWalletAccelByte::GetFromWorld(const UWorld* World, FOnlineWalletAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineWalletAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineWalletAccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetWalletInterface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineWalletAccelByte::GetCurrencyList(int32 LocalUserNum, bool bAlwaysRequestToService)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Currency List, LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (LocalUserId.IsValid())
			{
				AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetCurrencyList>(AccelByteSubsystemPtr.Get(), *LocalUserId, bAlwaysRequestToService);
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get currency list!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-currency-list-failed-userid-invalid");
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetCurrencyListCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsCurrencyList>{}, ErrorStr);

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-currency-list-failed-not-logged-in");
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetCurrencyListCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsCurrencyList>{}, ErrorStr);

	return false;
}

bool FOnlineWalletAccelByte::GetCurrencyFromCache(const FString& CurrencyCode, FAccelByteModelsCurrencyList& OutCurrency)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Currency, CurrencyCode: %s"), *CurrencyCode);
	FScopeLock ScopeLock(&CurrencyListLock);

	if (CurrencyCodeToCurrencyListMap.Num() > 0)
	{
		auto Currency = CurrencyCodeToCurrencyListMap.Find(CurrencyCode);
		if (Currency != nullptr)
		{
			OutCurrency = Currency->Get();
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Currency found for currency code %s"), *CurrencyCode);
			return true;
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Currency not found for currency code %s"), *CurrencyCode);
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Wallet Info, LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (LocalUserId.IsValid())
			{
				AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetWalletInfo>(AccelByteSubsystemPtr.Get(), *LocalUserId, CurrencyCode, bAlwaysRequestToService);
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get wallet info!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-wallet-info-failed-userid-invalid");
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetWalletInfoCompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfo{}, ErrorStr);

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-wallet-info-failed-not-logged-in");
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetWalletInfoCompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfo{}, ErrorStr);

	return false;
}

bool FOnlineWalletAccelByte::GetWalletInfoFromCache(int32 LocalUserNum, const FString& CurrencyCode, FAccelByteModelsWalletInfo& OutWalletInfo)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Wallet Info from Cache, LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		return false;
	}

	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		return false;
	}

	FScopeLock ScopeLock(&WalletInfoListLock);
	TMap<FString, TSharedRef<FAccelByteModelsWalletInfo>> CurrencyToWalletInfoMap = UserToWalletInfoMap.FindRef(LocalUserId.ToSharedRef());
	if (CurrencyToWalletInfoMap.Num() > 0)
	{
		TSharedRef<FAccelByteModelsWalletInfo>* WalletInfo = CurrencyToWalletInfoMap.Find(CurrencyCode);
		if (WalletInfo != nullptr)
		{
			OutWalletInfo = WalletInfo->Get();
			return true;
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	return false;
}

void FOnlineWalletAccelByte::AddWalletInfoToList(int32 LocalUserNum, const FString& CurrencyCode, const TSharedRef<FAccelByteModelsWalletInfo>& InWalletInfo)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (LocalUserId.IsValid())
			{
				FScopeLock ScopeLock(&WalletInfoListLock);
				auto CurrencyCodeToWalletInfo = UserToWalletInfoMap.FindRef(LocalUserId.ToSharedRef());
				CurrencyCodeToWalletInfo.Emplace(CurrencyCode, InWalletInfo);
				UserToWalletInfoMap.Emplace(LocalUserId.ToSharedRef(), CurrencyCodeToWalletInfo);
			}
		}
	}
}

bool FOnlineWalletAccelByte::ListWalletTransactionsByCurrencyCode(int32 LocalUserNum, const FString& CurrencyCode, int32 Offset, int32 Limit)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Wallet Transaction List, LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (LocalUserId.IsValid())
			{
				AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetWalletTransactions>(AccelByteSubsystemPtr.Get(), *LocalUserId, CurrencyCode, Offset, Limit);
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get wallet transaction list!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-wallet-transaction-list-failed-userid-invalid");
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetWalletTransactionsCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsWalletTransactionInfo>{}, ErrorStr);

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-wallet-transaction-list-failed-not-logged-in");
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetWalletTransactionsCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsWalletTransactionInfo>{}, ErrorStr);

	return false;
}
