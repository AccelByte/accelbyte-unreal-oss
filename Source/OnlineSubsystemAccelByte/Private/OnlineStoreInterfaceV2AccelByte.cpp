// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryCategories.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryChildCategories.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryOfferByFilter.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryOfferById.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryOfferBySku.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryOfferDynamicData.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteGetEstimatedPrice.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteGetItemByCriteria.h" 
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryActiveSections.h"
#include "AsyncTasks/Store/OnlineAsyncTaskAccelByteQueryStorefront.h"
#include "AsyncTasks/Store/MetaQuest/OnlineAsyncTaskAccelByteGetMetaQuestProductsBySku.h"
#include "OnlineSubsystemUtils.h"

FOnlineStoreV2AccelByte::FOnlineStoreV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem) 
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
	, ServiceLabel(1)
{}

void FOnlineStoreV2AccelByte::ReplaceCategories(TArray<FOnlineStoreCategory> InCategories)
{
	FScopeLock ScopeLock(&CategoriesLock);
	StoreCategories.Reset();
	for (const auto& Category : InCategories)
	{
		StoreCategories.Add(Category.Id, Category);
	}
}

void FOnlineStoreV2AccelByte::EmplaceCategories(TArray<FOnlineStoreCategory> InCategories)
{
	FScopeLock ScopeLock(&CategoriesLock);
	for (const auto& Category : InCategories)
	{
		StoreCategories.Emplace(Category.Id, Category);
	}
}

void FOnlineStoreV2AccelByte::ReplaceOffers(TMap<FUniqueOfferId, FOnlineStoreOfferRef> InOffer)
{
	FScopeLock ScopeLock(&OffersLock);
	StoreOffers = {};
	for (const auto& Offer : InOffer)
	{
		FOnlineStoreOfferAccelByte StoreOfferAccelByte(Offer.Value.Get());
		StoreOffers.Add(Offer.Key, MakeShared<FOnlineStoreOfferAccelByte>(StoreOfferAccelByte));
	} 
}

void FOnlineStoreV2AccelByte::ReplaceOffers(TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef> InOffer)
{
	FScopeLock ScopeLock(&OffersLock);
	StoreOffers = InOffer;
} 

void FOnlineStoreV2AccelByte::EmplaceOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferRef>& InOffer)
{
	FScopeLock ScopeLock(&OffersLock);

	TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef> StoreOffersTemp = {};
	for (const auto& Offer : InOffer)
	{
		FOnlineStoreOfferAccelByte StoreOfferAccelByte(Offer.Value.Get());
		StoreOffersTemp.Add(Offer.Key, MakeShared<FOnlineStoreOfferAccelByte>(StoreOfferAccelByte));
	} 
	
	for (const auto& Offer : StoreOffersTemp)
	{
		StoreOffers.Emplace(Offer.Key, Offer.Value);
	}
}

void FOnlineStoreV2AccelByte::EmplaceOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef>& InOffer)
{
	FScopeLock ScopeLock(&OffersLock);
	for (const auto& Offer : InOffer)
	{
		StoreOffers.Emplace(Offer.Key, Offer.Value);
	}
}

void FOnlineStoreV2AccelByte::ResetOffers()
{
	FScopeLock ScopeLock(&OffersLock);
	StoreOffers.Reset();
}

void FOnlineStoreV2AccelByte::EmplaceOfferDynamicData(const FUniqueNetId& InUserId, TSharedRef<FAccelByteModelsItemDynamicData> InDynamicData)
{
	FScopeLock ScopeLock(&DynamicDataLock);
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	FOfferToDynamicDataMap& FoundDynamicDataMap = OffersDynamicData.FindOrAdd(SharedUserId);
	FoundDynamicDataMap.Emplace(InDynamicData->ItemId, InDynamicData);
}

bool FOnlineStoreV2AccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineStoreV2AccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineStoreV2AccelByte::GetFromWorld(const UWorld* World, FOnlineStoreV2AccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

int32 FOnlineStoreV2AccelByte::GetServiceLabel()
{
	return ServiceLabel;
}

void FOnlineStoreV2AccelByte::SetServiceLabel(int32 InServiceLabel)
{
	ServiceLabel = InServiceLabel;
}

void FOnlineStoreV2AccelByte::QueryCategories(const FUniqueNetId& UserId, const FOnQueryOnlineStoreCategoriesComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryCategories>(AccelByteSubsystemPtr.Get(), UserId, Delegate);
}

void FOnlineStoreV2AccelByte::QueryChildCategories(const FUniqueNetId& UserId, const FString& CategoryPath, const FOnQueryOnlineStoreCategoriesComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryChildCategories>(AccelByteSubsystemPtr.Get(), UserId, CategoryPath, Delegate);
}

void FOnlineStoreV2AccelByte::GetCategories(TArray<FOnlineStoreCategory>& OutCategories) const
{
	FScopeLock ScopeLock(&CategoriesLock);
	StoreCategories.GenerateValueArray(OutCategories);
}

void FOnlineStoreV2AccelByte::GetCategory(const FString& CategoryPath, FOnlineStoreCategory& OutCategory) const
{
	FScopeLock ScopeLock(&CategoriesLock);
	OutCategory = *StoreCategories.Find(CategoryPath);
}

void FOnlineStoreV2AccelByte::QueryOffersByFilter(const FUniqueNetId& UserId, const FOnlineStoreFilter& Filter, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferByFilter>(AccelByteSubsystemPtr.Get(), UserId, Filter, Delegate, false);
}

void FOnlineStoreV2AccelByte::QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds,
	const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferById>(AccelByteSubsystemPtr.Get(), UserId, OfferIds, Delegate, FString(), false);
}

void FOnlineStoreV2AccelByte::QueryOffersByFilter(const FUniqueNetId& UserId, const FOnlineStoreFilter& Filter, bool AutoCalcEstimatedPrice, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferByFilter>(AccelByteSubsystemPtr.Get(), UserId, Filter, Delegate, AutoCalcEstimatedPrice);
}

void FOnlineStoreV2AccelByte::QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds, const FString& StoreId, bool AutoCalcEstimatedPrice, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferById>(AccelByteSubsystemPtr.Get(), UserId, OfferIds, Delegate, StoreId, AutoCalcEstimatedPrice);
}

void FOnlineStoreV2AccelByte::QueryOfferBySku(const FUniqueNetId& UserId, const FString& Sku, const FOnQueryOnlineStoreOffersComplete& Delegate, bool AutoCalcEstimatedPrice)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferBySku>(AccelByteSubsystemPtr.Get(), UserId, Sku, Delegate, AutoCalcEstimatedPrice);
}

void FOnlineStoreV2AccelByte::QueryOfferDynamicData(const FUniqueNetId& UserId, const FUniqueOfferId& OfferId, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferDynamicData>(AccelByteSubsystemPtr.Get(), UserId, OfferId, Delegate);
}

void FOnlineStoreV2AccelByte::GetOffers(TArray<FOnlineStoreOfferAccelByteRef>& OutOffers) const
{
	FScopeLock ScopeLock(&OffersLock);
	StoreOffers.GenerateValueArray(OutOffers);
}

void FOnlineStoreV2AccelByte::GetOffers(TArray<FOnlineStoreOfferRef>& OutOffers) const
{
	FScopeLock ScopeLock(&OffersLock);
	TArray<FOnlineStoreOfferAccelByteRef> OnlineStoreOfferAccelByte;
	StoreOffers.GenerateValueArray(OnlineStoreOfferAccelByte);

	TArray<FOnlineStoreOfferRef> OutOffersTemp;
	for(auto Offer : OnlineStoreOfferAccelByte)
	{
		OutOffersTemp.Add(Offer);
	}
	OutOffers = OutOffersTemp;
}

TSharedPtr<FOnlineStoreOffer> FOnlineStoreV2AccelByte::GetOffer(const FUniqueOfferId& OfferId) const
{
	FScopeLock ScopeLock(&OffersLock);
	const TSharedRef<FOnlineStoreOfferAccelByte>* Result = StoreOffers.Find(OfferId);
	if(Result)
	{
		return *Result;
	}
	return nullptr;
}

TSharedPtr<FOnlineStoreOfferAccelByte> FOnlineStoreV2AccelByte::GetOfferAccelByte(const FUniqueOfferId& OfferId) const
{
	FScopeLock ScopeLock(&OffersLock);
	const TSharedRef<FOnlineStoreOfferAccelByte>* Result = StoreOffers.Find(OfferId); 
	if(Result)
	{
		return *Result;
	}
	return nullptr;
}

TSharedPtr<FOnlineStoreOffer> FOnlineStoreV2AccelByte::GetOfferBySku(const FString& Sku) const
{
	FScopeLock ScopeLock(&OffersLock);
	for (const auto& Offer : StoreOffers)
	{
		if (Offer.Value->DynamicFields.Find(TEXT("Sku"))->Equals(Sku))
		{
			return Offer.Value;
		}
	}
	return nullptr;
}

TSharedPtr<FOnlineStoreOfferAccelByte> FOnlineStoreV2AccelByte::GetOfferBySkuAccelByte(const FString& Sku) const
{
	FScopeLock ScopeLock(&OffersLock);
	for (const auto& Offer : StoreOffers)
	{
		if (Offer.Value->DynamicFields.Find(TEXT("Sku"))->Equals(Sku))
		{
			return Offer.Value;
		}
	}
	return nullptr;
}

TSharedPtr<FAccelByteModelsItemDynamicData> FOnlineStoreV2AccelByte::GetOfferDynamicData(const FUniqueNetId& UserId, const FUniqueOfferId& OfferId) const
{
	FScopeLock ScopeLock(&DynamicDataLock);
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const FOfferToDynamicDataMap* FoundDynamicDataMap = OffersDynamicData.Find(SharedUserId);
	if (FoundDynamicDataMap != nullptr)
	{
		const TSharedRef<FAccelByteModelsItemDynamicData>* FoundDynamicData = FoundDynamicDataMap->Find(OfferId);
		if (FoundDynamicData != nullptr)
		{
			return *FoundDynamicData;
		}
	}
	return nullptr;
}

void FOnlineStoreV2AccelByte::GetEstimatedPrice(const FUniqueNetId& UserId, const TArray<FString>& ItemIds, const FString& Region)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetEstimatedPrice>(AccelByteSubsystemPtr.Get(), UserId, ItemIds, Region);
}

void FOnlineStoreV2AccelByte::GetItemsByCriteria(const FUniqueNetId& UserId,
	FAccelByteModelsItemCriteria const& ItemCriteria,
	int32 const& Offset,
	int32 const& Limit,
	TArray<EAccelByteItemListSortBy> SortBy,
	FString const& StoreId,
	bool AutoCalcEstimatedPrice)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetItemByCriteria>(AccelByteSubsystemPtr.Get(), UserId, ItemCriteria, Offset, Limit, SortBy, StoreId, AutoCalcEstimatedPrice);
}

void FOnlineStoreV2AccelByte::EmplaceSections(const FUniqueNetId& UserId, const TMap<FString, TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>>& InSections)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	FScopeLock ScopeLock(&SectionsLock);
	FPlayerStorefrontData& PlayerStorefrontData = StorefrontData.FindOrAdd(SharedUserId);
	PlayerStorefrontData.SectionsByDisplay.Reset();
	for (const TTuple<FString, TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>>& Section : InSections)
	{
		PlayerStorefrontData.Sections.Emplace(Section.Key, Section.Value);
		PlayerStorefrontData.SectionsByDisplay.EmplaceUnique(Section.Value->ViewId, Section.Value);
	}
}

void FOnlineStoreV2AccelByte::EmplaceOffersBySection(const FUniqueNetId& UserId, const TMultiMap<FString, FOnlineStoreOfferAccelByteRef> InOffersBySection)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	FScopeLock ScopeLock(&SectionsLock);
	FPlayerStorefrontData& PlayerStorefrontData = StorefrontData.FindOrAdd(SharedUserId);
	PlayerStorefrontData.OffersBySection.Reset();
	for (const TTuple<FString, FOnlineStoreOfferAccelByteRef>& Offers : InOffersBySection)
	{
		PlayerStorefrontData.OffersBySection.EmplaceUnique(Offers.Key, Offers.Value);
	}
}

void FOnlineStoreV2AccelByte::EmplaceDisplays(const TMap<FString, TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>>& InDisplays)
{
	FScopeLock ScopeLock(&DisplayLock);
	for (const TTuple<FString, TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>>& DisplayEntry : InDisplays)
	{
		Displays.Emplace(DisplayEntry.Key, DisplayEntry.Value);
	}
}

void FOnlineStoreV2AccelByte::EmplaceItemMappings(const TMap<FString, TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>>& InMappings)
{
	FScopeLock ScopeLock(&ItemMappingLock);
	for (const TTuple<FString,TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>>& MappingEntry : InMappings)
	{
		ItemMappings.Emplace(MappingEntry.Key, MappingEntry.Value);
	}
}

void FOnlineStoreV2AccelByte::GetDisplays(TArray<TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>>& OutDisplays)
{
	FScopeLock ScopeLock(&DisplayLock);
	Displays.GenerateValueArray(OutDisplays);
}

void FOnlineStoreV2AccelByte::GetSectionsForDisplay(const FUniqueNetId& UserId, const FString& DisplayId, TArray<TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>>& OutSections)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	FScopeLock ScopeLock(&SectionsLock);
	FPlayerStorefrontData* PlayerStorefrontData = StorefrontData.Find(SharedUserId);
	if (PlayerStorefrontData)
	{
		PlayerStorefrontData->SectionsByDisplay.MultiFind(DisplayId, OutSections);
	}
}

void FOnlineStoreV2AccelByte::GetOffersForSection(const FUniqueNetId& UserId, const FString& SectionId, TArray<FOnlineStoreOfferAccelByteRef>& OutOffers)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	FScopeLock ScopeLock(&SectionsLock);
	FPlayerStorefrontData* PlayerStorefrontData = StorefrontData.Find(SharedUserId);
	if (PlayerStorefrontData)
	{
		PlayerStorefrontData->OffersBySection.MultiFind(SectionId, OutOffers);
	}
}

void FOnlineStoreV2AccelByte::GetItemMappings(TArray<TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>>& OutMappings)
{
	FScopeLock ScopeLock(&ItemMappingLock);
	ItemMappings.GenerateValueArray(OutMappings);
}

void FOnlineStoreV2AccelByte::QueryActiveSections(const FUniqueNetId& UserId, const FString& StoreId, const FString& ViewId, const FString& Region, const FOnQueryActiveSectionsComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryActiveSections>(AccelByteSubsystemPtr.Get(), UserId, StoreId, ViewId, Region, Delegate);
}

void FOnlineStoreV2AccelByte::QueryStorefront(const FUniqueNetId& UserId, const FString& StoreId, const FString& ViewId, const FString& Region, const EAccelBytePlatformMapping& Platform, const FOnQueryStorefrontComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStorefront>(AccelByteSubsystemPtr.Get(), UserId, StoreId, ViewId, Region, Platform, Delegate);
}

void FOnlineStoreV2AccelByte::QueryPlatformOfferBySku(const FUniqueNetId& UserId, const TArray<FString>& Skus, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	if (AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Oculus"))
		|| AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Meta")))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku>(AccelByteSubsystemPtr.Get(), UserId, Skus, Delegate);
	}
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
}

void FOnlineStoreV2AccelByte::GetPlatformOffers(TArray<FOnlineStoreOfferRef>& OutOffers)
{
	FScopeLock ScopeLock(&PlatformOffersLock);
	PlatformOffers.GenerateValueArray(OutOffers);
}

void FOnlineStoreV2AccelByte::ReplacePlatformOffers(TMap<FUniqueOfferId, FOnlineStoreOfferRef> InOffer)
{
	FScopeLock ScopeLock(&PlatformOffersLock);
	PlatformOffers = InOffer;
}

void FOnlineStoreV2AccelByte::EmplacePlatformOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferRef>&InOffer)
{
	FScopeLock ScopeLock(&PlatformOffersLock);
	for (const auto& Offer : InOffer)
	{
		PlatformOffers.Emplace(Offer.Key, Offer.Value);
	}
}
