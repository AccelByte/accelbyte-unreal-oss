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
#include "OnlineSubsystemUtils.h"

FOnlineStoreV2AccelByte::FOnlineStoreV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem) 
	: AccelByteSubsystem(InSubsystem)
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
	StoreOffers = InOffer;
}

void FOnlineStoreV2AccelByte::EmplaceOffers(const TMap<FUniqueOfferId, FOnlineStoreOfferRef>& InOffer)
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
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
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
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryCategories>(AccelByteSubsystem, UserId, Delegate);
}

void FOnlineStoreV2AccelByte::QueryChildCategories(const FUniqueNetId& UserId, const FString& CategoryPath, const FOnQueryOnlineStoreCategoriesComplete& Delegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryChildCategories>(AccelByteSubsystem, UserId, CategoryPath, Delegate);
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
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferByFilter>(AccelByteSubsystem, UserId, Filter, Delegate);
}

void FOnlineStoreV2AccelByte::QueryOffersById(const FUniqueNetId& UserId, const TArray<FUniqueOfferId>& OfferIds,
	const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferById>(AccelByteSubsystem, UserId, OfferIds, Delegate);
}

void FOnlineStoreV2AccelByte::QueryOfferBySku(const FUniqueNetId& UserId, const FString& Sku, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferBySku>(AccelByteSubsystem, UserId, Sku, Delegate);
}

void FOnlineStoreV2AccelByte::QueryOfferDynamicData(const FUniqueNetId& UserId, const FUniqueOfferId& OfferId, const FOnQueryOnlineStoreOffersComplete& Delegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryOfferDynamicData>(AccelByteSubsystem, UserId, OfferId, Delegate);
}

void FOnlineStoreV2AccelByte::GetOffers(TArray<FOnlineStoreOfferRef>& OutOffers) const
{
	FScopeLock ScopeLock(&OffersLock);
	StoreOffers.GenerateValueArray(OutOffers);
}

TSharedPtr<FOnlineStoreOffer> FOnlineStoreV2AccelByte::GetOffer(const FUniqueOfferId& OfferId) const
{
	FScopeLock ScopeLock(&OffersLock);
	const TSharedRef<FOnlineStoreOffer>* Result = StoreOffers.Find(OfferId);
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
