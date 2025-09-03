// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
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

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetWalletInfoV2Completed, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsWalletInfoResponse& /*Response*/, const FString& /*Error*/);
typedef FOnGetWalletInfoV2Completed::FDelegate FOnGetWalletInfoV2CompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetWalletTransactionsCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsWalletTransactionInfo>& /*Response*/, const FString& /*Error*/);
typedef FOnGetWalletTransactionsCompleted::FDelegate FOnGetWalletTransactionsCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnWalletBalanceChangedNotification, int32 /*LocalUserNum*/, const FUniqueNetId& /*UserId*/, const FAccelByteModelsWalletBalanceChangedNotification& /*Notification*/);
typedef FOnWalletBalanceChangedNotification::FDelegate FOnWalletBalanceChangedNotificationDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnWalletStatusChangedNotification, int32 /*LocalUserNum*/, const FUniqueNetId& /*UserId*/, const FAccelByteModelsWalletStatusChangedNotification& /*Notification*/);
typedef FOnWalletStatusChangedNotification::FDelegate FOnWalletStatusChangedNotificationDelegate;

/**
 * Implementation of Wallet V2 service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineWalletV2AccelByte : public TSharedFromThis<FOnlineWalletV2AccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:

	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineWalletV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	TMap<FString, TSharedRef<FAccelByteModelsCurrencyList>> CurrencyCodeToCurrencyListMap;
	/** Critical sections for thread safe operation of CurrencyCodeToCurrencyListMap */
	mutable FCriticalSection CurrencyListLock;

	/*Map of WalletInfo of each user*/
	TUniqueNetIdMap <TMap<FString, TSharedRef<FAccelByteModelsWalletInfoResponse>>> UserToWalletInfoMapV2;
	/** Critical sections for thread safe operation of UserToWalletInfoMap */
	mutable FCriticalSection WalletInfoListLockV2;

	/**
	 * Method used by the Identity interface to register delegates for wallet notifications to this interface to get
	 * real-time updates from the Lobby websocket.
	 */
	virtual void RegisterRealTimeLobbyDelegates(int32 LocalUserNum);

public:
	virtual ~FOnlineWalletV2AccelByte() {};

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineWalletV2AccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineWalletV2AccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetCurrencyListCompleted, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsCurrencyList>& /*Response*/, const FString& /*Error*/);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetWalletInfoV2Completed, bool /*bWasSuccessful*/, const FAccelByteModelsWalletInfoResponse& /*Response*/, const FString& /*Error*/);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetWalletTransactionsCompleted, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsWalletTransactionInfo>& /*Response*/, const FString& /*Error*/);

	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnWalletBalanceChangedNotification, const FUniqueNetId& /*UserId*/, const FAccelByteModelsWalletBalanceChangedNotification& /*Notification*/);

	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnWalletStatusChangedNotification, const FUniqueNetId& /*UserId*/, const FAccelByteModelsWalletStatusChangedNotification& /*Notification*/);

	bool GetCurrencyList(int32 LocalUserNum, bool bAlwaysRequestToService = false);

	bool GetCurrencyFromCache(const FString& CurrencyCode, FAccelByteModelsCurrencyList& OutCurrency);

	bool GetAllCurrencyFromCache(TArray<FAccelByteModelsCurrencyList>& OutCurrencyList);

	void AddCurrencyToList(const FString& CurrencyCode, const TSharedRef<FAccelByteModelsCurrencyList>& InCurrencyList);
	
	bool GetWalletInfoByCurrencyCodeV2(int32 LocalUserNum, const FString& CurrencyCode, bool bAlwaysRequestToService = false);

	bool GetWalletInfoFromCacheV2(int32 LocalUserNum, const FString& CurrencyCode, FAccelByteModelsWalletInfoResponse& OutWalletInfo);

	void AddWalletInfoToListV2(int32 LocalUserNum, const FString& CurrencyCode, const TSharedRef<FAccelByteModelsWalletInfoResponse>& InWalletInfo);

	bool ListWalletTransactionsByCurrencyCode(int32 LocalUserNum, const FString& CurrencyCode, int32 Offset = 0, int32 Limit = 20);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineWalletV2AccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

	void OnWalletBalanceChangedNotificationReceived(const FAccelByteModelsWalletBalanceChangedNotification& Response, int32 InLocalUserNum);
	TMap<int32, FDelegateHandle> OnWalletBalanceChangedNotificationReceivedDelegateHandleMap;

	void OnWalletStatusChangedNotificationReceived(const FAccelByteModelsWalletStatusChangedNotification& Response, int32 InLocalUserNum);
	TMap<int32, FDelegateHandle> OnWalletStatusChangedNotificationReceivedDelegateHandleMap;
};
