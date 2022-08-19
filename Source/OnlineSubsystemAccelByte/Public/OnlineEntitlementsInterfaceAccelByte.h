// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineEntitlementsInterface.h"

using FEntitlementMap = TMap<FUniqueEntitlementId, TSharedRef<FOnlineEntitlement>>;
using FUserIDToEntitlementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FEntitlementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FEntitlementMap>>;

using FItemEntitlementMap = TMap<FString, TSharedRef<FOnlineEntitlement>>;
using FUserIDToItemEntitlementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FItemEntitlementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FItemEntitlementMap>>;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineEntitlementsAccelByte : public IOnlineEntitlements
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a entitlements interface instance */
	FOnlineEntitlementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	virtual void AddEntitlementToMap(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, TSharedRef<FOnlineEntitlement> Entitlement);

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

	virtual TSharedPtr<FOnlineEntitlement> GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId) override;
	virtual TSharedPtr<FOnlineEntitlement> GetItemEntitlement(const FUniqueNetId& UserId, const FString& ItemId) override;
	virtual void GetAllEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement>>& OutUserEntitlements) override;
	virtual bool QueryEntitlements(const FUniqueNetId& UserId, const FString& Namespace, const FPagedQuery& Page) override;

protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	FUserIDToEntitlementMap EntitlementMap;
	FUserIDToItemEntitlementMap ItemEntitlementMap;
	
	/** Critical sections for thread safe operation of EntitlementMap */
	mutable FCriticalSection EntitlementMapLock;

};
