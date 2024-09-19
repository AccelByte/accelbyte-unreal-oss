// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

// Client Caches
using FEntitlementMap = TMap<FUniqueEntitlementId, TSharedRef<FOnlineEntitlement>>;
using FUserIDToEntitlementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FEntitlementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FEntitlementMap>>;

using FItemEntitlementMap = TMap<FString, TSharedRef<FOnlineEntitlement>>;
using FUserIDToItemEntitlementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FItemEntitlementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FItemEntitlementMap>>;

using FCurrentUserEntitlementHistory = TMap<FString, TArray<FAccelByteModelsBaseUserEntitlementHistory>>;
using FCurrentUserIDToEntitlementHistoryMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FCurrentUserEntitlementHistory, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FCurrentUserEntitlementHistory>>;

// Server Caches
using FEntitlementHistory = TMap<FString, TArray<FAccelByteModelsUserEntitlementHistory>>;
using FUserIDToEntitlementHistoryMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FEntitlementHistory, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FEntitlementHistory>>;

// Client Delegates
DECLARE_MULTICAST_DELEGATE_FourParams(FOnConsumeEntitlementComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const TSharedPtr<FOnlineEntitlement>& /*Entitlement*/, const FOnlineError& /*Error*/);
typedef FOnConsumeEntitlementComplete::FDelegate FOnConsumeEntitlementCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetCurrentUserEntitlementHistoryComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsBaseUserEntitlementHistory>& /*Entitlement History*/, const FOnlineError& /*Error*/);
typedef FOnGetCurrentUserEntitlementHistoryComplete::FDelegate FOnGetCurrentUserEntitlementHistoryCompleteDelegate;

// Server Delegates
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetUserEntitlementHistoryComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsUserEntitlementHistory>& /*Entitlement History*/, const FOnlineError& /*Error*/);
typedef FOnGetUserEntitlementHistoryComplete::FDelegate FOnGetUserEntitlementHistoryCompleteDelegate;

#define ENTITLEMENT_ATTR_KEY_SKU TEXT("SKU")

struct ONLINESUBSYSTEMACCELBYTE_API FOnlineEntitlementAccelByte : FOnlineEntitlement
{
public:
	/**
	 * 
	 * @param AttrName Attribute name key
	 * @param OutAttrValue Output value of the attribute associated with the name
	 * @return boolean if OutAttrValue has value
	 */
	virtual bool GetAttribute(const FString& AttrName, FString& OutAttrValue) const override;

	/**
	 * 
	 * @return Entitlement Info available from AccelByte backend
	 */
	virtual FAccelByteModelsEntitlementInfo GetBackendEntitlementInfo() const;
	
PACKAGE_SCOPE:
	TMap<FString, FString> ExtraAttributes;

	FAccelByteModelsEntitlementInfo BackendEntitlementInfo;

	/** Set attribute value, allow overwrite if the attribute name already exist */
	virtual bool SetAttribute(const FString& AttrName, const FString& AttrValue);

	virtual bool SetBackendEntitlementInfo(const FAccelByteModelsEntitlementInfo Info);
	
	virtual ~FOnlineEntitlementAccelByte() override;
};

class ONLINESUBSYSTEMACCELBYTE_API FOnlineEntitlementsAccelByte : public IOnlineEntitlements
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a entitlements interface instance */
	FOnlineEntitlementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	virtual void AddEntitlementToMap(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, TSharedRef<FOnlineEntitlement> Entitlement);

	virtual void AddUserEntitlementHistoryToMap(const FUniqueNetIdAccelByteUserRef& UserId, const FUniqueEntitlementId& EntitlementId, TArray<FAccelByteModelsUserEntitlementHistory> UserEntitlementHistory);

	virtual void AddCurrentUserEntitlementHistoryToMap(const FUniqueNetIdAccelByteUserRef& UserId, TArray<FAccelByteModelsBaseUserEntitlementHistory> CurrentUserEntitlementHistory);

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

	// Client Delegates
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnConsumeEntitlementComplete, bool, const FUniqueNetId&, const TSharedPtr<FOnlineEntitlement>&, const FOnlineError&);
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnGetCurrentUserEntitlementHistoryComplete, int32, bool, const TArray<FAccelByteModelsBaseUserEntitlementHistory>&, const FOnlineError&);

	// Server Delegates
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnGetUserEntitlementHistoryComplete, int32, bool, const TArray<FAccelByteModelsUserEntitlementHistory>&, const FOnlineError&);

	virtual TSharedPtr<FOnlineEntitlement> GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId) override;
	virtual TSharedPtr<FOnlineEntitlement> GetItemEntitlement(const FUniqueNetId& UserId, const FString& ItemId) override;
	virtual void GetAllEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement>>& OutUserEntitlements) override;
	virtual bool QueryEntitlements(const FUniqueNetId& UserId, const FString& Namespace, const FPagedQuery& Page = FPagedQuery{}) override;
	
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
	
	/*
	 * Sync platform purchase, can handle 3rd party platforms like Steam, Epic, Playstation, and Xbox
	 * @param LocalUserNum LocalUserNum of user that making the call
	 * @param EntitlementSyncBase The sync request information
	 * @param CompletionDelegate Will be triggered after the request completed
	 */
	void SyncPlatformPurchase(int32 LocalUserNum, FAccelByteModelsEntitlementSyncBase EntitlementSyncBase, const FOnRequestCompleted& CompletionDelegate = FOnRequestCompleted());

	/*
	 * Sync platform purchase using purchase receipt, only support for GooglePlay & iOS AppStore purchase for now.
	 * @param LocalUserNum LocalUserNum of user that making the call
	 * @param Receipt The receipt you get after purchase from the native subsystem
	 * @param CompletionDelegate Will be triggered after the request completed
	 */
	void SyncPlatformPurchase(int32 LocalUserNum, const TSharedRef<FPurchaseReceipt>& Receipt, const FOnRequestCompleted& CompletionDelegate = FOnRequestCompleted());

	/*
	 * Sync platform purchase for googleplay
	 * @param LocalUserNum LocalUserNum of user that making the call
	 * @param Request The sync request information
	 * @param CompletionDelegate Will be triggered after the request completed
	 */
	void SyncPlatformPurchase(int32 LocalUserNum, const FAccelByteModelsPlatformSyncMobileGoogle& Request, const FOnRequestCompleted& CompletionDelegate = FOnRequestCompleted());

	/*
	 * Sync platform's DLC, can handle 3rd party platforms like Steam, Epic, Playstation, and Xbox
	 * @param LocalUserNum LocalUserNum of user that making the call
	 * @param CompletionDelegate Will be triggered after the request completed
	 */
	void SyncDLC(const FUniqueNetId& InLocalUserId, const FOnRequestCompleted& CompletionDelegate);

#pragma region Client - User Entitlement History

	/**
	* Get user entitlement history inside the cache. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for player/user.
	*
	* @param UserId User id of the entitlement owner.
	* @param EntitlementId The id of the entitlement.
	*/
	void GetCurrentUserEntitlementHistory(const FUniqueNetId& UserId
		, const FUniqueEntitlementId& EntitlementId);

	/**
	* Get user entitlement history. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for player/user.
	*
	* @param UserId User id of the entitlement owner.
	* @param EntitlementClass (Optional) The entitlement class or type to get.
	* @param StartDate (Optional) Defines the preferred starting date or time to query user entitlement history.
	* @param EndDate (Optional) Defines the preferred ending date or time to query user entitlement history.
	* @param Page (Optional) Number of content per page to retrieve and page to retrieve. Default value start (offset) : 0, count (limit) : 100.
	*/
	void GetCurrentUserEntitlementHistory(const FUniqueNetId& UserId
		, const EAccelByteEntitlementClass& EntitlementClass = EAccelByteEntitlementClass::NONE
		, FDateTime StartDate = 0
		, FDateTime EndDate = 0
		, const FPagedQuery& Page = FPagedQuery{});

	/**
	* Get user entitlement history. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for player/user.
	*
	* @param UserId User id of the entitlement owner.
	* @param bForceUpdate (Optional) Set it to 'True' to get the history from backend.
	* @param EntitlementId (Optional) If bForceUpdate = false, set entitlement id to get history from cache.
	* @param EntitlementClass (Optional) The entitlement class or type to get.
	* @param StartDate (Optional) Defines the preferred starting date or time to query user entitlement history.
	* @param EndDate (Optional) Defines the preferred ending date or time to query user entitlement history.
	* @param Page (Optional) Number of content per page to retrieve and page to retrieve. Default value start (offset) : 0, count (limit) : 100.
	*/
	void GetCurrentUserEntitlementHistory(const FUniqueNetId& UserId
		, bool bForceUpdate = true
		, const FUniqueEntitlementId& EntitlementId = TEXT("")
		, const EAccelByteEntitlementClass& EntitlementClass = EAccelByteEntitlementClass::NONE
		, FDateTime StartDate = 0
		, FDateTime EndDate = 0
		, const FPagedQuery& Page = FPagedQuery{});

	/**
	* Get user entitlement history inside the cache. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for player/user.
	*
	* @param LocalUserNum Index of user as the owner of the entitlement.
	* @param EntitlementId The id of the entitlement.
	*/
	void GetCurrentUserEntitlementHistory(int32 LocalUserNum
		, const FUniqueEntitlementId& EntitlementId);

	/**
	* Get user entitlement history. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for player/user.
	*
	* @param LocalUserNum Index of user as the owner of the entitlement.
	* @param EntitlementClass (Optional) The entitlement class or type to get.
	* @param StartDate (Optional) Defines the preferred starting date or time to query user entitlement history.
	* @param EndDate (Optional) Defines the preferred ending date or time to query user entitlement history.
	* @param Page (Optional) Number of content per page to retrieve and page to retrieve. Default value start (offset) : 0, count (limit) : 100.
	*/
	void GetCurrentUserEntitlementHistory(int32 LocalUserNum
		, const EAccelByteEntitlementClass& EntitlementClass = EAccelByteEntitlementClass::NONE
		, FDateTime StartDate = 0
		, FDateTime EndDate = 0
		, const FPagedQuery& Page = FPagedQuery{});

	/**
	* Get user entitlement history. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for player/user.
	*
	* @param LocalUserNum Index of user as the owner of the entitlement.
	* @param bForceUpdate (Optional) Set it to 'True' to get the history from backend.
	* @param EntitlementId (Optional) If bForceUpdate = false, set entitlement id to get history from cache.
	* @param EntitlementClass (Optional) The entitlement class or type to get.
	* @param StartDate (Optional) Defines the preferred starting date or time to query user entitlement history.
	* @param EndDate (Optional) Defines the preferred ending date or time to query user entitlement history.
	* @param Page (Optional) Number of content per page to retrieve and page to retrieve. Default value start (offset) : 0, count (limit) : 100.
	*/
	void GetCurrentUserEntitlementHistory(int32 LocalUserNum
		, bool bForceUpdate = true
		, const FUniqueEntitlementId& EntitlementId = TEXT("")
		, const EAccelByteEntitlementClass& EntitlementClass = EAccelByteEntitlementClass::NONE
		, FDateTime StartDate = 0
		, FDateTime EndDate = 0
		, const FPagedQuery& Page = FPagedQuery{});

#pragma endregion 

#pragma region Server - User Entitlement History

	/**
	* Get user entitlement history. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for dedicated server.
	*
	* @param LocalUserNum Index of user as the owner of the entitlement.
	* @param EntitlementId The id of the entitlement.
	* @param bForceUpdate (Optional) Set it to 'True' to get the history from backend.
	*/
	void GetUserEntitlementHistory(int32 LocalUserNum, const FUniqueEntitlementId& EntitlementId, bool bForceUpdate = true);

	/**
	* Get user entitlement history. Trigger FOnGetUserEntitlementHistoryComplete on complete
	* Note: This endpoint supports only for dedicated server.
	*
	* @param UserId Id of the entilement owner
	* @param EntitlementId The id of the entitlement.
	* @param bForceUpdate (Optional) Set it to 'True' to get the history from backend.
	*/
	void GetUserEntitlementHistory(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId, bool bForceUpdate = true);

#pragma endregion 

#pragma region User Entitlement Cache

	/**
	 * This function returns a cache of data that was generated by the 'GetCurrentUserEntitlementHistory' function
	 *
	 * @param InUserId The entitlement owner user id.
	 * @param InEntitlementId The id of the entitlement.
	 * @param OutEntitlementHistory List of User Inventory Items found.
	 * It also returns a value that associated with the entitlement.
	 *
	 * @returns boolean - Result after getting current user entitlement history inside the cache
	 */
	virtual bool GetCachedCurrentUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId
		, const FUniqueEntitlementId& InEntitlementId
		, TArray<FAccelByteModelsBaseUserEntitlementHistory>& OutEntitlementHistory);

	/**
	 * This function removes a cache of data that was generated by the 'GetCurrentUserEntitlementHistory' function
	 *
	 * @param InUserId The entitlement owner user id.
	 * @param InEntitlementId The id of the entitlement.
	 *
	 * @returns boolean - Result after deleting current user entitlement history inside the cache
	 */
	virtual bool DeleteCachedCurrentUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId
		, const FUniqueEntitlementId& InEntitlementId);

#pragma endregion

#pragma region Server Entitlement Cache

	/**
	 * This function returns a cache of data that was generated by the 'GetUserEntitlementHistory' function
	 *
	 * @param InUserId The entitlement owner user id.
	 * @param InEntitlementId The id of the entitlement.
	 * @param OutEntitlementHistory List of user entilement history found.
	 * It also returns a value that associated with the entitlement.
	 * 
	 * @returns boolean - Result after looking user entitlement history inside the cache
	 */
	virtual bool GetCachedUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId
	, const FUniqueEntitlementId& InEntitlementId
	, TArray<FAccelByteModelsUserEntitlementHistory>& OutEntitlementHistory);

	/**
	 * This function removes a cache of data that was generated by the 'GetUserEntitlementHistory' function
	 *
	 * @param InUserId The entitlement owner user id.
	 * @param InEntitlementId The id of the entitlement.
	 *
	 * @returns boolean - Result after deleting user entitlement history inside the cache
	 */
	virtual bool DeleteCachedUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId
	, const FUniqueEntitlementId& InEntitlementId);

#pragma endregion

protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	FUserIDToEntitlementMap EntitlementMap;
	FUserIDToItemEntitlementMap ItemEntitlementMap;
	FCurrentUserIDToEntitlementHistoryMap CurrentUserEntitlementHistoryMap;
	FUserIDToEntitlementHistoryMap UserEntitlementHistoryMap;

	TMap<FString, TArray<FAccelByteModelsBaseUserEntitlementHistory>> ExtractUserEntitlementId(TArray<FAccelByteModelsBaseUserEntitlementHistory> InCurrentUserEntitlementHistory);
	
	/** Critical sections for thread safe operation of EntitlementMap */
	mutable FCriticalSection EntitlementMapLock;

};
