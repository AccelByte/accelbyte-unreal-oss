// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryOfferById.h"

FOnlineAsyncTaskAccelByteQueryOfferById::FOnlineAsyncTaskAccelByteQueryOfferById(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const TArray<FUniqueOfferId>& InOfferIds,
	const FOnQueryOnlineStoreOffersComplete& InDelegate) 
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, OfferIds(InOfferIds)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
	Delegate = InDelegate;
	OfferMap = TMap<FUniqueOfferId, FOnlineStoreOfferRef>{};
	
	Language = Subsystem->GetLanguage();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	FOnlineAsyncTaskAccelByte::Initialize();
	
	OnSuccess = THandler<TArray<FAccelByteModelsItemInfo>>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferById::HandleGetItemByIds);
	OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferById::HandleAsyncTaskError);
	ApiClient->Item.BulkGetLocaleItems(OfferIds, TEXT(""), Language, OnSuccess, OnError);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	FOnlineAsyncTaskAccelByte::Finalize();
	
	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	StoreV2Interface->EmplaceOffers(OfferMap);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	TArray<FString> QueriedOfferIds;
	OfferMap.GenerateKeyArray(QueriedOfferIds);
	Delegate.ExecuteIfBound(bWasSuccessful, QueriedOfferIds, ErrorMsg);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::HandleGetItemByIds(const TArray<FAccelByteModelsItemInfo>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Get Success"));
	for (const auto& Item : Result)
	{
		FOnlineStoreOfferRef Offer = MakeShared<FOnlineStoreOffer>();
		Offer->OfferId = Item.ItemId;
		Offer->NumericPrice = Item.RegionData[0].DiscountedPrice;
		Offer->RegularPrice = Item.RegionData[0].Price;
		Offer->CurrencyCode = Item.RegionData[0].CurrencyCode;
		Offer->Title = FText::FromString(Item.Title);
		if (Item.Images.Num() > 0)
		{
			Offer->DynamicFields.Add(TEXT("IconUrl"), Item.Images[0].ImageUrl);
		}
		Offer->DynamicFields.Add(TEXT("Region"), Item.Region);
		Offer->DynamicFields.Add(TEXT("IsConsumable"), Item.EntitlementType == EAccelByteEntitlementType::CONSUMABLE ? TEXT("true") : TEXT("false"));
		Offer->DynamicFields.Add(TEXT("Category"), Item.CategoryPath);
		Offer->DynamicFields.Add(TEXT("Name"), Item.Name);
		Offer->DynamicFields.Add(TEXT("ItemType"), FAccelByteUtilities::GetUEnumValueAsString(Item.ItemType));
		Offer->DynamicFields.Add(TEXT("Sku"), Item.Sku);
		if (Item.ItemType == EAccelByteItemType::COINS)
		{
			Offer->DynamicFields.Add(TEXT("TargetCurrencyCode"), Item.TargetCurrencyCode);
		}
		OfferMap.Add(Offer->OfferId, Offer);
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
	
	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}