// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryOfferByFilter.h"

FOnlineAsyncTaskAccelByteQueryOfferByFilter::FOnlineAsyncTaskAccelByteQueryOfferByFilter(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FOnlineStoreFilter& InFilter, const FOnQueryOnlineStoreOffersComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, Filter(InFilter)
	, Delegate(InDelegate)
	, Language(InABSubsystem->GetLanguage())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryOfferByFilter::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Initialize();

	if(Filter.Keywords.Num() > 1 || Filter.IncludeCategories.Num() > 1 || Filter.ExcludeCategories.Num() > 1)
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Multiple filter value currently not supported! Keyword.Num: %d, IncludeCategories.Num: %d, ExcludeCategories.Num: %d"), Filter.Keywords.Num(), Filter.IncludeCategories.Num(), Filter.ExcludeCategories.Num());
	}

	if(Filter.Keywords.Num() == 0)
	{
		bIsSearchByCriteria = true;
		// Search all items
		THandler<FAccelByteModelsItemPagingSlicedResult> OnSuccess = THandler<FAccelByteModelsItemPagingSlicedResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleGetItemByCriteria);
		FErrorHandler OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleAsyncTaskError);
		SearchCriteriaRequest = {};
		SearchCriteriaRequest.Language = Language;
		if (Filter.IncludeCategories.Num() != 0)
		{
			SearchCriteriaRequest.CategoryPath = Filter.IncludeCategories[0].Id;
		}
		ApiClient->Item.GetItemsByCriteria(SearchCriteriaRequest, 0, 20, OnSuccess, OnError);
	}
	else
	{
		// search by keyword, and the result filtered by categories
		THandler<FAccelByteModelsItemPagingSlicedResult> OnSuccess = THandler<FAccelByteModelsItemPagingSlicedResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleSearchItem);
		FErrorHandler OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleAsyncTaskError);
		ApiClient->Item.SearchItem(Language, Filter.Keywords[0], 0, 20, TEXT(""), OnSuccess, OnError);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferByFilter::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Finalize();
	
	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	StoreV2Interface->EmplaceOffers(OfferMap);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferByFilter::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	TArray<FString> OfferIds;
	OfferMap.GenerateKeyArray(OfferIds);
	Delegate.ExecuteIfBound(bWasSuccessful, OfferIds, ErrorMsg);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleGetItemByCriteria(const FAccelByteModelsItemPagingSlicedResult& Result)
{
	FilterAndAddResults(Result);

	if(!Result.Paging.Next.IsEmpty())
	{
		int32 Offset = -1;
		int32 Limit = -1;
		GetNextOffset(Result.Paging.Next, Offset, Limit);
		if(Offset != -1 && Limit != -1)
		{
			THandler<FAccelByteModelsItemPagingSlicedResult> OnSuccess = THandler<FAccelByteModelsItemPagingSlicedResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleGetItemByCriteria);
			FErrorHandler OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleAsyncTaskError);
			ApiClient->Item.GetItemsByCriteria(SearchCriteriaRequest, Offset, Limit, OnSuccess, OnError);
			return;
		}
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleSearchItem(const FAccelByteModelsItemPagingSlicedResult& Result)
{
	FilterAndAddResults(Result);

	if(!Result.Paging.Next.IsEmpty())
	{
		int32 Offset = -1;
		int32 Limit = -1;
		GetNextOffset(Result.Paging.Next, Offset, Limit);
		if(Offset != -1 && Limit != -1)
		{
			THandler<FAccelByteModelsItemPagingSlicedResult> OnSuccess = THandler<FAccelByteModelsItemPagingSlicedResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleGetItemByCriteria);
			FErrorHandler OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleAsyncTaskError);
			ApiClient->Item.SearchItem(Language, Filter.Keywords[0], Offset, Limit, TEXT(""), OnSuccess, OnError);
			return;
		}
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}


void FOnlineAsyncTaskAccelByteQueryOfferByFilter::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferByFilter::GetNextOffset(FString const& NextUrl, int32& OutOffset, int32& OutLimit)
{
	FString UrlOut;
	FString Params;
	NextUrl.Split(TEXT("?"), &UrlOut, &Params);
	if(!Params.IsEmpty())
	{
		TArray<FString> ParamsArray;
		Params.ParseIntoArray(ParamsArray, TEXT("&"));
		for(FString Param : ParamsArray)
		{
			FString Key;
			FString Value;
			Param.Split(TEXT("="), &Key, &Value);
			if(Key.Equals(TEXT("offset")) && Value.IsNumeric())
			{
				OutOffset = FCString::Atoi(*Value);
			}
			else if(Key.Equals(TEXT("limit")) && Value.IsNumeric())
			{
				OutLimit = FCString::Atoi(*Value);
			}
		}
	}
}

void FOnlineAsyncTaskAccelByteQueryOfferByFilter::FilterAndAddResults(const FAccelByteModelsItemPagingSlicedResult& Result)
{
	for(FAccelByteModelsItemInfo const& Item : Result.Data)
	{
		if(!Item.Purchasable ||	Item.ItemType == EAccelByteItemType::APP ||
			(Filter.ExcludeCategories.Num() > 0 && Item.CategoryPath.Contains(Filter.ExcludeCategories[0].Id)))
		{
			continue;
		}
		if (!bIsSearchByCriteria && Filter.IncludeCategories.Num() > 0)
		{
			if (!Item.CategoryPath.Contains(Filter.IncludeCategories[0].Id))
			{
				continue;
			}
		}
		
		FOnlineStoreOfferRef Offer = MakeShared<FOnlineStoreOffer>();
		Offer->OfferId = Item.ItemId;
		Offer->NumericPrice = Item.RegionData[0].DiscountedPrice;
		Offer->RegularPrice = Item.RegionData[0].Price;
		Offer->CurrencyCode = Item.RegionData[0].CurrencyCode;
		Offer->Title = FText::FromString(Item.Title);
		if(Item.Images.Num() > 0)
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
}