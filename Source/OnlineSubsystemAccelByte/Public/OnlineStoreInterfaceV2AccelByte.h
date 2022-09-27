// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "Models/AccelByteEcommerceModels.h"

/** Typedef for a map of Offers to Item's Dynamic Data Map */
using FOfferToDynamicDataMap = TMap<FUniqueOfferId, TSharedRef<FAccelByteModelsItemDynamicData>>;
/** Typedef for a map of user IDs to Item's Dynamic Data Map */
using FUserIDToDynamicDataMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FOfferToDynamicDataMap>;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineStoreV2AccelByte : public IOnlineStoreV2
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a store interface instance */
	FOnlineStoreV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	virtual void ReplaceCategories(TArray<FOnlineStoreCategory> InCategories);
	virtual void EmplaceCategories(TArray<FOnlineStoreCategory> InCategories);
	/** Critical sections for thread safe operation of Categories */
	mutable FCriticalSection CategoriesLock;
	virtual void ReplaceOffers(TMap<FUniqueOfferId, FOnlineStoreOfferRef> InOffer);
	virtual void EmplaceOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferRef>& InOffer);
	virtual void ResetOffers();
	/** Critical sections for thread safe operation of Offers */
	mutable FCriticalSection OffersLock;
	virtual void EmplaceOfferDynamicData(const FUniqueNetId& InUserId, TSharedRef<FAccelByteModelsItemDynamicData> InDynamicData);
	/** Critical sections for thread safe operation of DynamicData */
	mutable FCriticalSection DynamicDataLock;

	int32 GetServiceLabel();
	void SetServiceLabel(int32 InServiceLabel);
public:
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineStoreV2AccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineStoreV2AccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	virtual void QueryCategories(const FUniqueNetId& UserId, const FOnQueryOnlineStoreCategoriesComplete& Delegate) override;
	virtual void QueryChildCategories(const FUniqueNetId& UserId, const FString& CategoryPath, const FOnQueryOnlineStoreCategoriesComplete& Delegate);
	virtual void GetCategories(TArray<FOnlineStoreCategory>& OutCategories) const override;
	virtual void GetCategory(const FString& CategoryPath, FOnlineStoreCategory& OutCategory) const;
	virtual void QueryOffersByFilter(const FUniqueNetId& UserId, const FOnlineStoreFilter& Filter, const FOnQueryOnlineStoreOffersComplete& Delegate) override;
	virtual void QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds, const FOnQueryOnlineStoreOffersComplete& Delegate) override;
	virtual void QueryOfferBySku(const FUniqueNetId& UserId, const FString& Sku, const FOnQueryOnlineStoreOffersComplete& Delegate);
	virtual void QueryOfferDynamicData(const FUniqueNetId& UserId, const FUniqueOfferId& OfferId, const FOnQueryOnlineStoreOffersComplete& Delegate);
	virtual void GetOffers(TArray<FOnlineStoreOfferRef>& OutOffers) const override;
	virtual TSharedPtr<FOnlineStoreOffer> GetOffer(const FUniqueOfferId& OfferId) const override;
	virtual TSharedPtr<FOnlineStoreOffer> GetOfferBySku(const FString& Sku) const;
	virtual TSharedPtr<FAccelByteModelsItemDynamicData> GetOfferDynamicData(const FUniqueNetId& UserId, const FUniqueOfferId& OfferId) const;
	
protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;
	TMap<FUniqueCategoryId, FOnlineStoreCategory> StoreCategories;
	TMap<FUniqueOfferId, FOnlineStoreOfferRef> StoreOffers;
	FUserIDToDynamicDataMap OffersDynamicData;

private:
	int32 ServiceLabel;
};
