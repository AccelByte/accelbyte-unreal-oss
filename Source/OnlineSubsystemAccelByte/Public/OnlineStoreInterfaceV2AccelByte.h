// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "Models/AccelByteEcommerceModels.h" 
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineError.h"
#include "OnlineErrorAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/Platform/AccelBytePlatformHandleModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

/** Typedef for a map of Offers to Item's Dynamic Data Map */
using FOfferToDynamicDataMap = TMap<FUniqueOfferId, TSharedRef<FAccelByteModelsItemDynamicData>>;
/** Typedef for a map of user IDs to Item's Dynamic Data Map */
using FUserIDToDynamicDataMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FOfferToDynamicDataMap>;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetEstimatedPriceComplete, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsEstimatedPrices>& /* Result */, const FOnlineErrorAccelByte& /*Error*/);
typedef FOnGetEstimatedPriceComplete::FDelegate FOnGetEstimatedPriceCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetItemsByCriteriaComplete, bool /*bWasSuccessful*/, const FAccelByteModelsItemPagingSlicedResult& /* Result */, const FOnlineErrorAccelByte& /*Error*/);
typedef FOnGetItemsByCriteriaComplete::FDelegate FOnGetItemsByCriteriaCompleteDelegate;

DECLARE_DELEGATE_FourParams(FOnQueryActiveSectionsComplete, bool /*bWasSuccessful*/, const TArray<FString>& /* SectionIds */, const TArray<FUniqueOfferId>& /*OfferIds*/, const FString& /*Error*/);

DECLARE_DELEGATE_SixParams(FOnQueryStorefrontComplete, bool /*bWasSuccessful*/, const TArray<FString>& /* ViewIds */, const TArray<FString>& /* SectionIds */, const TArray<FUniqueOfferId>& /*OfferIds*/, const TArray<FString>& /* ItemMappingIds */, const FString& /*Error*/);

/**
 * AccelByte's Offer entry for display from online store
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineStoreOfferAccelByte : public FOnlineStoreOffer
{
public: 
	FOnlineStoreOfferAccelByte() : FOnlineStoreOffer()
		, RegionData({})
		, Language(TEXT(""))
		, Sku(TEXT(""))
		, Flexible (false)
		, Sellable (false)
		, Stackable (false)
		, Purchasable (true)
		, Listable (false) 
		, SectionExclusive (false)
		, SaleConfig ({})
		, LootBoxConfig ({})
		, OptionBoxConfig ({})
	{
	}

	FOnlineStoreOfferAccelByte(const FOnlineStoreOffer& InStoreOffer) : FOnlineStoreOffer()
		, RegionData({})
		, Language(TEXT(""))
		, Sku(TEXT(""))
		, Flexible(false)
		, Sellable(false)
		, Stackable(false)
		, Purchasable(true)
		, Listable(false)
		, SectionExclusive(false)
		, SaleConfig({})
		, LootBoxConfig({})
		, OptionBoxConfig({})
	{
		OfferId = InStoreOffer.OfferId;
		Title = InStoreOffer.Title;
		Description = InStoreOffer.Description;
		LongDescription = InStoreOffer.LongDescription;
		RegularPriceText = InStoreOffer.RegularPriceText;
		RegularPrice = InStoreOffer.RegularPrice;
		PriceText = InStoreOffer.PriceText;
		NumericPrice = InStoreOffer.NumericPrice;
		CurrencyCode = InStoreOffer.CurrencyCode;
		ReleaseDate = InStoreOffer.ReleaseDate;
		ExpirationDate = InStoreOffer.ExpirationDate;
		DiscountType = InStoreOffer.DiscountType;
		DynamicFields = InStoreOffer.DynamicFields;
	}

	virtual ~FOnlineStoreOfferAccelByte()
	{
	}

	/** Region Data Info of the Offer */
	TArray<FAccelByteModelsItemRegionDataItem> RegionData;

	/** language of the Offer */
	FString Language;

	/** Sku of the Offer */
	FString Sku;

	/** True, if the Offer is a flexible bundle item */
	bool Flexible;

	/** True, if the Offer is a sellable item */
	bool Sellable;

	/** True, if the Offer is a stackable item */
	bool Stackable;

	/**True, if the Offer is a purchasable item */
	bool Purchasable;

	/** True, if the Offer is a listable item */
	bool Listable;
	
	/** STrue, if the Offer is a section exclusive item */
	bool SectionExclusive;

	/** @return True if offer can be purchased */
	virtual bool IsPurchaseable() const override
	{
		return Purchasable;
	}
		
	/** Sale Configuration of the Offer */
	FAccelByteModelsItemSaleConfig SaleConfig{};

	/** Loot Box configuration of the Offer */
	FAccelByteModelsItemLootBoxConfig LootBoxConfig{};

	/** Option Box configuration of the Offer */
	FAccelByteModelItemOptionBoxConfig OptionBoxConfig{};

	/** Customized offer properties **/
	FJsonObject Ext{};

	void SetItem(const FAccelByteModelsItemInfo& Item)
	{
		OfferId = Item.ItemId;
		NumericPrice = Item.RegionData[0].DiscountedPrice;
		RegularPrice = Item.RegionData[0].Price;
		CurrencyCode = Item.RegionData[0].CurrencyCode;
		if (Item.RegionData[0].CurrencyType == EAccelByteItemCurrencyType::VIRTUAL)
		{
			PriceText = FText::Format(FTextFormat::FromString("{0} {1}"), FText::AsNumber(Item.RegionData[0].DiscountedPrice), FText::FromString(Item.RegionData[0].CurrencyCode));
			RegularPriceText = FText::Format(FTextFormat::FromString("{0} {1}"), FText::AsNumber(Item.RegionData[0].Price), FText::FromString(Item.RegionData[0].CurrencyCode));
		}
		Title = FText::FromString(Item.Title);
		RegionData = Item.RegionData;
		Language = Item.Language;
		Sku = Item.Sku;
		Flexible = Item.Flexible;
		Sellable = Item.Sellable;
		Stackable = Item.Stackable;
		Purchasable = Item.Purchasable;
		Listable = Item.Listable;
		SectionExclusive = Item.SectionExclusive;
		SaleConfig = Item.SaleConfig;
		LootBoxConfig = Item.LootBoxConfig;
		OptionBoxConfig = Item.OptionBoxConfig;
		if (Item.Images.Num() > 0)
		{
			DynamicFields.Add(TEXT("IconUrl"), Item.Images[0].ImageUrl);
		}
		DynamicFields.Add(TEXT("Region"), Item.Region);
		DynamicFields.Add(TEXT("IsConsumable"), Item.EntitlementType == EAccelByteEntitlementType::CONSUMABLE ? TEXT("true") : TEXT("false"));
		DynamicFields.Add(TEXT("Category"), Item.CategoryPath);
		DynamicFields.Add(TEXT("Name"), Item.Name);
		DynamicFields.Add(TEXT("ItemType"), FAccelByteUtilities::GetUEnumValueAsString(Item.ItemType));
		DynamicFields.Add(TEXT("Sku"), Item.Sku);
		if (Item.ItemType == EAccelByteItemType::COINS)
		{
			DynamicFields.Add(TEXT("TargetCurrencyCode"), Item.TargetCurrencyCode);
		}
		Ext = *Item.Ext.JsonObject;
	}
};
typedef TSharedRef<FOnlineStoreOfferAccelByte> FOnlineStoreOfferAccelByteRef;

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
	virtual void ReplaceOffers(TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef> InOffer);
	virtual void EmplaceOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferRef>& InOffer);
	virtual void EmplaceOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef>& InOffer);
	virtual void ResetOffers();
	/** Critical sections for thread safe operation of Offers */
	mutable FCriticalSection OffersLock;
	virtual void EmplaceOfferDynamicData(const FUniqueNetId& InUserId, TSharedRef<FAccelByteModelsItemDynamicData> InDynamicData);
	/** Critical sections for thread safe operation of DynamicData */
	mutable FCriticalSection DynamicDataLock;

	int32 GetServiceLabel();
	void SetServiceLabel(int32 InServiceLabel);

	virtual void EmplaceSections(const FUniqueNetId& UserId, const TMap<FString, TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>>& InSections);
	virtual void EmplaceOffersBySection(const FUniqueNetId& UserId, const TMultiMap<FString, FOnlineStoreOfferAccelByteRef> InOffersBySection);
	/** Critical sections for thread safe operation of Sections */
	mutable FCriticalSection SectionsLock;

	virtual void EmplaceDisplays(const TMap<FString, TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>>& InDisplays);
	virtual void EmplaceItemMappings(const TMap<FString, TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>>& InMappings);
	/** Critical sections for thread safe operation of Displays */
	mutable FCriticalSection DisplayLock;
	/** Critical sections for thread safe operation of ItemMappings */
	mutable FCriticalSection ItemMappingLock;

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
	virtual void QueryOffersByFilter(const FUniqueNetId& UserId, const FOnlineStoreFilter& Filter, bool AutoCalcEstimatedPrice, const FOnQueryOnlineStoreOffersComplete& Delegate);
	virtual void QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds, const FString& StoreId, bool AutoCalcEstimatedPrice, const FOnQueryOnlineStoreOffersComplete& Delegate);
	virtual void QueryOfferBySku(const FUniqueNetId& UserId, const FString& Sku, const FOnQueryOnlineStoreOffersComplete& Delegate, bool AutoCalcEstimatedPrice = false);
	virtual void QueryOfferDynamicData(const FUniqueNetId& UserId, const FUniqueOfferId& OfferId, const FOnQueryOnlineStoreOffersComplete& Delegate);
	virtual void GetOffers(TArray<FOnlineStoreOfferRef>& OutOffers) const override;
	virtual void GetOffers(TArray<FOnlineStoreOfferAccelByteRef>& OutOffers) const;
	virtual TSharedPtr<FOnlineStoreOffer> GetOffer(const FUniqueOfferId& OfferId) const override;
	virtual TSharedPtr<FOnlineStoreOfferAccelByte> GetOfferAccelByte(const FUniqueOfferId& OfferId) const;
	virtual TSharedPtr<FOnlineStoreOffer> GetOfferBySku(const FString& Sku) const;
	virtual TSharedPtr<FOnlineStoreOfferAccelByte> GetOfferBySkuAccelByte(const FString& Sku) const;
	virtual TSharedPtr<FAccelByteModelsItemDynamicData> GetOfferDynamicData(const FUniqueNetId& UserId, const FUniqueOfferId& OfferId) const;

#pragma region PlatformIAP
	void QueryPlatformOfferBySku(const FUniqueNetId& UserId, const TArray<FString>& Skus, const FOnQueryOnlineStoreOffersComplete& Delegate);
	void GetPlatformOffers(TArray<FOnlineStoreOfferRef>& OutOffers);
PACKAGE_SCOPE:
	mutable FCriticalSection PlatformOffersLock;
	void ReplacePlatformOffers(TMap<FUniqueOfferId, FOnlineStoreOfferRef> InOffer);
	void EmplacePlatformOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferRef>& InOffer);
#pragma endregion

public:
	/**
	 * Delegate called when a controller-user get a estimated price.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnGetEstimatedPriceComplete, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsEstimatedPrices>& /*EstimatedPrices*/, const FOnlineErrorAccelByte& /*OnlineError*/);
	/**
	 *  Get estimated prices of item.
	 *
	 * @param UserId The user's user ID.
	 * @param ItemIds The item's IDs to check, commas separated item ids.
	 * @param Region Code, ISO 3166-1 alpha-2 country tag, e.g., "US", "CN".
	 * @returns  
	 */
	void GetEstimatedPrice(const FUniqueNetId& UserId, const TArray<FString>& ItemIds, const FString& Region);

	/**
	 * Delegate called when a controller-user get items by criteria.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnGetItemsByCriteriaComplete, bool /*bWasSuccessful*/, const FAccelByteModelsItemPagingSlicedResult& /*Result*/, const FOnlineErrorAccelByte& /*OnlineError*/);
	/**
	 * Get an array of items with specific criteria/filter from online store.
	 *
	 * @param UserId The user's user ID.
	 * @param ItemCriteria should be contain some parameters for query.
	 * @param Offset Page number.
	 * @param Limit Page size.
	 * @param SortBy Make sure to always use more than one sort if the first sort is not an unique value for example, if you wish to sort by displayOrder,
	 * @param StoreId The Store Id, default value is published store id
	 * @param AutoCalcEstimatedPrice This will Auto Calculate Estimated Price. Default is false, if autoCalcEstimatedPrice is true and item is flexible bundle, will auto calc price.	 
	 */ 
	void GetItemsByCriteria(const FUniqueNetId& UserId,
		FAccelByteModelsItemCriteria const& ItemCriteria,
		int32 const& Offset = 0,
		int32 const& Limit = 20,
		TArray<EAccelByteItemListSortBy> SortBy = {},
		FString const& StoreId = TEXT(""),
		bool AutoCalcEstimatedPrice = false);

	/**
	 * @bried Get cached store displays.
	 *
	 * @param OutDisplays Output of cached store displays.
	 */
	virtual void GetDisplays(TArray<TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>>& OutDisplays);

	/**
	 * @brief Get cached store section from specified display id.
	 *
	 * @param UserId The UniqueNetId of current user.
	 * @param DisplayId The display id for cached sections.
	 * @param OutSections Output of cached store sections.
	 */
	virtual void GetSectionsForDisplay(const FUniqueNetId& UserId, const FString& DisplayId, TArray<TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>>& OutSections);

	/**
	 * @brief Get cached offers from specified section id.
	 *
	 * @param UserId The UniqueNetId of current user.
	 * @param SectionId The section id for cached offers.
	 * @param OutOffers Output of cached offers.
	 */
	virtual void GetOffersForSection(const FUniqueNetId& UserId, const FString& SectionId, TArray<FOnlineStoreOfferAccelByteRef>& OutOffers);

	/**
	 * @brief Get cached other platform store item mappings.
	 *
	 * @param OutMappings Output of cached platform store mappings.
	 */
	virtual void GetItemMappings(TArray<TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>>& OutMappings);

	/**
	 * @brief Query the store active sections based on store id, view id and region.
	 *
	 * @param UserId The UniqueNetId of current user.
	 * @param StoreId The store id, for default store use empty string.
	 * @param ViewId The view id, for default store use empty string.
	 * @param Region The region, for default store use empty string.
	 * @param Delegate This delegate will be called after query completed.
	 */
	virtual void QueryActiveSections(const FUniqueNetId& UserId, const FString& StoreId, const FString& ViewId, const FString& Region, const FOnQueryActiveSectionsComplete& Delegate);

	/**
	 * @brief Query store front based on store id, view id, region and platform.
	 *
	 * @param UserId The UniqueNetId of current user.
	 * @param StoreId The store id, for default store use empty string.
	 * @param ViewId The view id, for default store use empty string.
	 * @param Region The region, for default store use empty string.
	 * @param Platform The platform of third party store.
	 * @param Delegate This delegate will be called after query completed.
	 */
	virtual void QueryStorefront(const FUniqueNetId& UserId, const FString& StoreId, const FString& ViewId, const FString& Region, const EAccelBytePlatformMapping& Platform, const FOnQueryStorefrontComplete& Delegate);


protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;
	TMap<FUniqueCategoryId, FOnlineStoreCategory> StoreCategories; 
	TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef> StoreOffers;
	TMap<FUniqueOfferId, FOnlineStoreOfferRef> PlatformOffers;
	FUserIDToDynamicDataMap OffersDynamicData;

	struct FPlayerStorefrontData
	{
		TMap<FString, TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>> Sections{};
		TMultiMap<FString, TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>> SectionsByDisplay{};
		TMultiMap<FString, FOnlineStoreOfferAccelByteRef> OffersBySection{};
	};

	TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FPlayerStorefrontData> StorefrontData;
	TMap<FString, TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>> Displays;
	TMap<FString, TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>> ItemMappings;

private:
	int32 ServiceLabel;
};
