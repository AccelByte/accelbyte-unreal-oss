// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteQueryCategories.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteQueryOfferByFilter.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteQueryOfferById.h"
#include "OnlineSubsystemUtils.h"

FOnlineStoreV2AccelByte::FOnlineStoreV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem) 
	: AccelByteSubsystem(InSubsystem)
	, ServiceLabel(1)
{}

void FOnlineStoreV2AccelByte::ReplaceCategories(TArray<FOnlineStoreCategory> InCategories)
{
	FScopeLock ScopeLock(&CategoriesLock);
	StoreCategories = InCategories;
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

void FOnlineStoreV2AccelByte::GetCategories(TArray<FOnlineStoreCategory>& OutCategories) const
{
	FScopeLock ScopeLock(&CategoriesLock);
	OutCategories = StoreCategories;
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
