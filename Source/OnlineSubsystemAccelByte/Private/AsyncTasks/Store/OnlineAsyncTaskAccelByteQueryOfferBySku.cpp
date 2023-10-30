// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryOfferBySku.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryOfferBySku::FOnlineAsyncTaskAccelByteQueryOfferBySku(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InSku,
	const FOnQueryOnlineStoreOffersComplete& InDelegate) 
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, Sku(InSku)
	, Delegate(InDelegate)
	, Offer(MakeShared<FOnlineStoreOfferAccelByte>())
	, Language(InABSubsystem->GetLanguage())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	OnSuccess = TDelegateUtils<THandler<FAccelByteModelsItemInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleGetItemBySku);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleAsyncTaskError);
	ApiClient->Item.GetItemBySku(Sku, Language, TEXT(""), OnSuccess, OnError);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	Super::Finalize();
	
	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	StoreV2Interface->EmplaceOffers(TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef>{TPair<FUniqueOfferId, FOnlineStoreOfferAccelByteRef>{Offer->OfferId, Offer}});
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates();

	TArray<FString> QueriedOfferIds{Offer->OfferId};
	Delegate.ExecuteIfBound(bWasSuccessful, QueriedOfferIds, ErrorMsg);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleGetItemBySku(const FAccelByteModelsItemInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Get Success"));
	Offer->OfferId = Result.ItemId;
	Offer->NumericPrice = Result.RegionData[0].DiscountedPrice;
	Offer->RegularPrice = Result.RegionData[0].Price;
	Offer->CurrencyCode = Result.RegionData[0].CurrencyCode;
	Offer->Title = FText::FromString(Result.Title);
	Offer->RegionData = Result.RegionData;
	Offer->Language = Result.Language;
	Offer->Sku = Result.Sku;
	Offer->Flexible = Result.Flexible;
	Offer->Sellable = Result.Sellable;
	Offer->Stackable = Result.Stackable;
	Offer->Purchasable = Result.Purchasable;
	Offer->Listable = Result.Listable;
	Offer->SectionExclusive = Result.SectionExclusive;
	Offer->SaleConfig = Result.SaleConfig;
	Offer->LootBoxConfig = Result.LootBoxConfig;
	Offer->OptionBoxConfig = Result.OptionBoxConfig;
	if (Result.Images.Num() > 0)
	{
		Offer->DynamicFields.Add(TEXT("IconUrl"), Result.Images[0].ImageUrl);
	}
	Offer->DynamicFields.Add(TEXT("Region"), Result.Region);
	Offer->DynamicFields.Add(TEXT("IsConsumable"), Result.EntitlementType == EAccelByteEntitlementType::CONSUMABLE ? TEXT("true") : TEXT("false"));
	Offer->DynamicFields.Add(TEXT("Category"), Result.CategoryPath);
	Offer->DynamicFields.Add(TEXT("Name"), Result.Name);
	Offer->DynamicFields.Add(TEXT("ItemType"), FAccelByteUtilities::GetUEnumValueAsString(Result.ItemType));
	Offer->DynamicFields.Add(TEXT("Sku"), Result.Sku);
	if (Result.ItemType == EAccelByteItemType::COINS)
	{
		Offer->DynamicFields.Add(TEXT("TargetCurrencyCode"), Result.TargetCurrencyCode);
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
	
	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}