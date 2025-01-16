// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

class UWorld;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetCurrencyListCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsCurrencyList>& /*Response*/, const FString& /*Error*/);
typedef FOnGetCurrencyListCompleted::FDelegate FOnGetCurrencyListCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetWalletInfoCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsWalletInfo& /*Response*/, const FString& /*Error*/);
typedef FOnGetWalletInfoCompleted::FDelegate FOnGetWalletInfoCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetWalletTransactionsCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsWalletTransactionInfo>& /*Response*/, const FString& /*Error*/);
typedef FOnGetWalletTransactionsCompleted::FDelegate FOnGetWalletTransactionsCompletedDelegate;

/**
 * Implementation of Wallet service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineWalletAccelByte : public TSharedFromThis<FOnlineWalletAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:

	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineWalletAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	TMap<FString, TSharedRef<FAccelByteModelsCurrencyList>> CurrencyCodeToCurrencyListMap;
	/** Critical sections for thread safe operation of CurrencyCodeToCurrencyListMap */
	mutable FCriticalSection CurrencyListLock;

	/*Map of WalletInfo of each user*/
	TUniqueNetIdMap <TMap<FString, TSharedRef<FAccelByteModelsWalletInfo>>> UserToWalletInfoMap;
	/** Critical sections for thread safe operation of UserToWalletInfoMap */
	mutable FCriticalSection WalletInfoListLock;

public:
	virtual ~FOnlineWalletAccelByte() {};

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineWalletAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineWalletAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetCurrencyListCompleted, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsCurrencyList>& /*Response*/, const FString& /*Error*/);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetWalletInfoCompleted, bool /*bWasSuccessful*/, const FAccelByteModelsWalletInfo& /*Response*/, const FString& /*Error*/);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetWalletTransactionsCompleted, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsWalletTransactionInfo>& /*Response*/, const FString& /*Error*/);

	bool GetCurrencyList(int32 LocalUserNum, bool bAlwaysRequestToService = false);

	bool GetCurrencyFromCache(const FString& CurrencyCode, FAccelByteModelsCurrencyList& OutCurrency);

	bool GetAllCurrencyFromCache(TArray<FAccelByteModelsCurrencyList>& OutCurrencyList);

	void AddCurrencyToList(const FString& CurrencyCode, const TSharedRef<FAccelByteModelsCurrencyList>& InCurrencyList);
	
	bool GetWalletInfoByCurrencyCode(int32 LocalUserNum, const FString& CurrencyCode, bool bAlwaysRequestToService = false);

	bool GetWalletInfoFromCache(int32 LocalUserNum, const FString& CurrencyCode, FAccelByteModelsWalletInfo& OutWalletInfo);

	void AddWalletInfoToList(int32 LocalUserNum, const FString& CurrencyCode, const TSharedRef<FAccelByteModelsWalletInfo>& InWalletInfo);

	bool ListWalletTransactionsByCurrencyCode(int32 LocalUserNum, const FString& CurrencyCode, int32 Offset = 0, int32 Limit = 20);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineWalletAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

};
