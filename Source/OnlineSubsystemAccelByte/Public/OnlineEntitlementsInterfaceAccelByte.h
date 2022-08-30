// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Models/AccelByteEcommerceModels.h"

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
	virtual TSharedPtr<FOnlineEntitlement> GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId) override;
	virtual TSharedPtr<FOnlineEntitlement> GetItemEntitlement(const FUniqueNetId& UserId, const FString& ItemId) override;
	virtual void GetAllEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement>>& OutUserEntitlements) override;
	virtual bool QueryEntitlements(const FUniqueNetId& UserId, const FString& Namespace, const FPagedQuery& Page) override;
	void SyncPlatformPurchase(int32 LocalUserNum, FAccelByteModelsEntitlementSyncBase EntitlementSyncBase, const FOnRequestCompleted& CompletionDelegate = FOnRequestCompleted());
	void SyncDLC(const FUniqueNetId& InLocalUserId, const FOnRequestCompleted& CompletionDelegate);

protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	FUserIDToEntitlementMap EntitlementMap;
	FUserIDToItemEntitlementMap ItemEntitlementMap;
	/** Critical sections for thread safe operation of EntitlementMap */
	mutable FCriticalSection EntitlementMapLock;
};
