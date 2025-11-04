// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineWalletV2InterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"

#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/Wallet/OnlineAsyncTaskAccelByteGetCurrencyList.h"
#include "AsyncTasks/Wallet/OnlineAsyncTaskAccelByteGetWalletInfo.h"
#include "AsyncTasks/Wallet/OnlineAsyncTaskAccelByteGetWalletInfoV2.h"
#include "AsyncTasks/Wallet/OnlineAsyncTaskAccelByteGetWalletTransactions.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/World.h"
#include "OnlineSubsystemAccelByteLog.h"

FOnlineWalletV2AccelByte::FOnlineWalletV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{};

bool FOnlineWalletV2AccelByte::GetFromWorld(const UWorld* World, FOnlineWalletV2AccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineWalletV2AccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineWalletV2AccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetWalletV2Interface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineWalletV2AccelByte::GetCurrencyList(int32 LocalUserNum, bool bAlwaysRequestToService)
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

bool FOnlineWalletV2AccelByte::GetCurrencyFromCache(const FString& CurrencyCode, FAccelByteModelsCurrencyList& OutCurrency)
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

bool FOnlineWalletV2AccelByte::GetAllCurrencyFromCache(TArray<FAccelByteModelsCurrencyList>& OutCurrencyList)
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

void FOnlineWalletV2AccelByte::AddCurrencyToList(const FString& CurrencyCode, const TSharedRef<FAccelByteModelsCurrencyList>& InCurrencyList)
{
	FScopeLock ScopeLock(&CurrencyListLock);

	CurrencyCodeToCurrencyListMap.Add(CurrencyCode, InCurrencyList);
}

bool FOnlineWalletV2AccelByte::GetWalletInfoByCurrencyCodeV2(int32 LocalUserNum, const FString& CurrencyCode, bool bAlwaysRequestToService)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Wallet Info V2, LocalUserNum: %d"), LocalUserNum);

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
				AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetWalletInfoV2>(AccelByteSubsystemPtr.Get(), *LocalUserId, CurrencyCode, bAlwaysRequestToService);
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get wallet info V2!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-wallet-info-failed-userid-invalid");
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetWalletInfoV2CompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfoResponse{}, ErrorStr);

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-wallet-info-failed-not-logged-in");
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetWalletInfoV2CompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfoResponse{}, ErrorStr);

	return false;
}

bool FOnlineWalletV2AccelByte::GetWalletInfoFromCacheV2(int32 LocalUserNum, const FString& CurrencyCode, FAccelByteModelsWalletInfoResponse& OutWalletInfo)
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

	FScopeLock ScopeLock(&WalletInfoListLockV2);
	TMap<FString, TSharedRef<FAccelByteModelsWalletInfoResponse>> CurrencyToWalletInfoMap = UserToWalletInfoMapV2.FindRef(LocalUserId.ToSharedRef());
	if (CurrencyToWalletInfoMap.Num() > 0)
	{
		TSharedRef<FAccelByteModelsWalletInfoResponse>* WalletInfo = CurrencyToWalletInfoMap.Find(CurrencyCode);
		if (WalletInfo != nullptr)
		{
			OutWalletInfo = WalletInfo->Get();
			return true;
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	return false;
}

void FOnlineWalletV2AccelByte::AddWalletInfoToListV2(int32 LocalUserNum, const FString& CurrencyCode, const TSharedRef<FAccelByteModelsWalletInfoResponse>& InWalletInfoResponse)
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
				FScopeLock ScopeLock(&WalletInfoListLockV2);
				auto CurrencyCodeToWalletInfo = UserToWalletInfoMapV2.FindRef(LocalUserId.ToSharedRef());
				CurrencyCodeToWalletInfo.Emplace(CurrencyCode, InWalletInfoResponse);
				UserToWalletInfoMapV2.Emplace(LocalUserId.ToSharedRef(), CurrencyCodeToWalletInfo);
			}
		}
	}
}

bool FOnlineWalletV2AccelByte::ListWalletTransactionsByCurrencyCode(int32 LocalUserNum, const FString& CurrencyCode, int32 Offset, int32 Limit)
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

void FOnlineWalletV2AccelByte::OnWalletBalanceChangedNotificationReceived(const FAccelByteModelsWalletBalanceChangedNotification& Response, int32 InLocalUserNum)
{
	// Get our identity interface to retrieve the UniqueNetId for this user
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
		if (IdentityInterface->GetLoginStatus(InLocalUserNum) == ELoginStatus::LoggedIn)
		{
			const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(InLocalUserNum);
			if (LocalUserId.IsValid())
			{
				//Trigger the notification
				TriggerOnWalletBalanceChangedNotificationDelegates(InLocalUserNum, *LocalUserId.Get(), Response);
			}
		}
	}
}

void FOnlineWalletV2AccelByte::OnWalletStatusChangedNotificationReceived(const FAccelByteModelsWalletStatusChangedNotification & Response, int32 InLocalUserNum)
{
	// Get our identity interface to retrieve the UniqueNetId for this user
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
		if (IdentityInterface->GetLoginStatus(InLocalUserNum) == ELoginStatus::LoggedIn)
		{
			const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(InLocalUserNum);
			if (LocalUserId.IsValid())
			{
				//Trigger the notification
				TriggerOnWalletStatusChangedNotificationDelegates(InLocalUserNum, *LocalUserId.Get(), Response);
			}
		}
	}
}

void FOnlineWalletV2AccelByte::RegisterRealTimeLobbyDelegates(int32 LocalUserNum)
{
	// Get our identity interface to retrieve the API client for this user
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if (!ApiClient.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to register real-time lobby as an Api client could not be retrieved for user num %d!"), LocalUserNum);
		return;
	}

	const auto Lobby = ApiClient->GetLobbyApi().Pin();
	if (!Lobby.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to register real-time lobby as an Lobby Api could not be retrieved for user num %d!"), LocalUserNum);
		return;
	}

	// Set each delegate for the corresponding API client to be a new realtime delegate

	if (OnWalletBalanceChangedNotificationReceivedDelegateHandleMap.Find(LocalUserNum) == nullptr)
	{
		THandler<FAccelByteModelsWalletBalanceChangedNotification> OnBalanceChangedNotificationReceivedDelegate = THandler<FAccelByteModelsWalletBalanceChangedNotification>::CreateThreadSafeSP(AsShared(), &FOnlineWalletV2AccelByte::OnWalletBalanceChangedNotificationReceived, LocalUserNum);
		OnWalletBalanceChangedNotificationReceivedDelegateHandleMap.Add(LocalUserNum, Lobby->AddWalletBalanceChangedNotifDelegate(OnBalanceChangedNotificationReceivedDelegate));
	}

	if (OnWalletStatusChangedNotificationReceivedDelegateHandleMap.Find(LocalUserNum) == nullptr)
	{
		THandler<FAccelByteModelsWalletStatusChangedNotification> OnStatusChangedNotificationReceivedDelegate = THandler<FAccelByteModelsWalletStatusChangedNotification>::CreateThreadSafeSP(AsShared(), &FOnlineWalletV2AccelByte::OnWalletStatusChangedNotificationReceived, LocalUserNum);
		OnWalletStatusChangedNotificationReceivedDelegateHandleMap.Add(LocalUserNum, Lobby->AddWalletStatusChangedNotifDelegate(OnStatusChangedNotificationReceivedDelegate));
	}
}