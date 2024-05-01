// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

using FEntitlementMap = TMap<FUniqueEntitlementId, TSharedRef<FOnlineEntitlement>>;
using FUserIDToEntitlementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FEntitlementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FEntitlementMap>>;

using FItemEntitlementMap = TMap<FString, TSharedRef<FOnlineEntitlement>>;
using FUserIDToItemEntitlementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FItemEntitlementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FItemEntitlementMap>>;

using FEntitlementHistory = TMap<FString, TArray<FAccelByteModelsUserEntitlementHistory>>;
using FUserIDToEntitlementHistoryMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FEntitlementHistory, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FEntitlementHistory>>;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnConsumeEntitlementComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const TSharedPtr<FOnlineEntitlement>& /*Entitlement*/, const FOnlineError& /*Error*/);
typedef FOnConsumeEntitlementComplete::FDelegate FOnConsumeEntitlementCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetUserEntitlementHistoryComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsUserEntitlementHistory>& /*Entitlement*/, const FOnlineError& /*Error*/);
typedef FOnGetUserEntitlementHistoryComplete::FDelegate FOnGetUserEntitlementHistoryCompleteDelegate;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineEntitlementsAccelByte : public IOnlineEntitlements
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a entitlements interface instance */
	FOnlineEntitlementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	virtual void AddEntitlementToMap(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, TSharedRef<FOnlineEntitlement> Entitlement);

	virtual void AddUserEntitlementHistoryToMap(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FUniqueEntitlementId& EntitlementId, TArray<FAccelByteModelsUserEntitlementHistory> UserEntitlementHistory);

public:
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineEntitlementsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineEntitlementsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnConsumeEntitlementComplete, bool, const FUniqueNetId&, const TSharedPtr<FOnlineEntitlement>&, const FOnlineError&);
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnGetUserEntitlementHistoryComplete, int32, bool, const TArray<FAccelByteModelsUserEntitlementHistory>&, const FOnlineError&);

	virtual TSharedPtr<FOnlineEntitlement> GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId) override;
	virtual TSharedPtr<FOnlineEntitlement> GetItemEntitlement(const FUniqueNetId& UserId, const FString& ItemId) override;
	virtual void GetAllEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement>>& OutUserEntitlements) override;
	virtual bool QueryEntitlements(const FUniqueNetId& UserId, const FString& Namespace, const FPagedQuery& Page = FPagedQuery{}) override;
	void SyncPlatformPurchase(int32 LocalUserNum, FAccelByteModelsEntitlementSyncBase EntitlementSyncBase, const FOnRequestCompleted& CompletionDelegate = FOnRequestCompleted());
	void SyncDLC(const FUniqueNetId& InLocalUserId, const FOnRequestCompleted& CompletionDelegate);

	/**
	* Consume a user entitlement. Trigger FOnConsumeEntitlementComplete on complete
	*
	* @param UserId Id of the entilement owner
	* @param EntitlementId The id of the entitlement.
	* @param UseCount Number of consumed entitlement.
	* @param Options Options of consumed entitlements.
	* @param RequestId Request id(Optional), A unique tracking ID provided by the developer, can just left it empty if they don't want to track
	* When a request id is submitted, it will return original successful response
	*/
	void ConsumeEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId, int32 UseCount, TArray<FString> Options = {}, const FString& RequestId = {});

	/**
	* Get user entitlement history. Trigger FOnGetUserEntitlementHistoryComplete on complete
	*
	* @param LocalUserNum Index of user as the owner of the entitlement.
	* @param EntitlementId The id of the entitlement.
	* @param bForceUpdate (Optional) Set it to 'True' to get the history from backend.
	*/
	void GetUserEntitlementHistory(int32 LocalUserNum, const FUniqueEntitlementId& EntitlementId, bool bForceUpdate = true);

	/**
	 * This function returns a cache of data that was generated by the 'GetUserEntitlementHistory' function
	 *
	 * @param InUserId The entitlement owner user id.
	 * @param InEntitlementId The id of the entitlement.
	 * @param OutEntitlementHistory List of User Inventory Items found.
	 * It also returns a value that associated with the entitlement.
	 * 
	 * @returns boolean - List of User Inventory Items found
	 */
	virtual bool GetCachedUserEntitlementHistory(const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId
	, const FUniqueEntitlementId& InEntitlementId
	, TArray<FAccelByteModelsUserEntitlementHistory>& OutEntitlementHistory);

	/**
	 * This function removes a cache of data that was generated by the 'GetUserEntitlementHistory' function
	 *
	 * @param InUserId The entitlement owner user id.
	 * @param InEntitlementId The id of the entitlement.
	 *
	 * @returns boolean - List of User Inventory Items found
	 */
	virtual bool DeleteCachedUserEntitlementHistory(const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId
	, const FUniqueEntitlementId& InEntitlementId);

protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	FUserIDToEntitlementMap EntitlementMap;
	FUserIDToItemEntitlementMap ItemEntitlementMap;
	FUserIDToEntitlementHistoryMap UserEntitlementHistoryMap;
	
	/** Critical sections for thread safe operation of EntitlementMap */
	mutable FCriticalSection EntitlementMapLock;

};
