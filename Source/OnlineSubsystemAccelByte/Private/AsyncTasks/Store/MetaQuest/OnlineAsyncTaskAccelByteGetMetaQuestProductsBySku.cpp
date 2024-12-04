// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetMetaQuestProductsBySku.h"

#include "Core/Platform/AccelBytePlatformHandler.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku"

FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, const TArray<FString>& InSkus
	, const FOnQueryOnlineStoreOffersComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Skus(InSkus)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	FAccelBytePlatformHandler PlatformHandler;
	const AccelBytePlatformWrapperWPtr PlatformWrapperWPtr = PlatformHandler.GetPlatformWrapper(EAccelBytePlatformType::Oculus);
	const AccelBytePlatformWrapperPtr PlatformWrapper = PlatformWrapperWPtr.Pin();
		
	if (!PlatformWrapper.IsValid())
	{
		OnlineError.bSucceeded = false;
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Can't fetch platform friends as platform wrapper is invalid."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const auto OnSuccess = TDelegateUtils<TDelegate<void(const TArray<FPlatformProductPtr>&)>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::OnGetProductsBySkuSuccess);
	const auto OnError = TDelegateUtils<TDelegate<void(const FPlatformHandlerError&)>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::OnGetProductsBySkuError);
	PlatformWrapper->GetProductsBySku(Skus, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	Super::Finalize();
	
	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
	StoreV2Interface->EmplacePlatformOffers(Products);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d, bWasSuccessfull: %s"), *UserId->ToDebugString(), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful));
	TArray<FString> QueriedOfferIds{};
	Products.GetKeys(QueriedOfferIds);
	bool bExecuted = Delegate.ExecuteIfBound(bWasSuccessful, QueriedOfferIds, OnlineError.ErrorMessage.ToString());

	if (!bExecuted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Can't trigger delegate as it is unbound."));
		return;
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::OnGetProductsBySkuSuccess(const TArray<FPlatformProductPtr>& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	for (const auto& PlatformProduct : Response)
	{
		if (PlatformProduct.IsValid())
		{
			FOnlineStoreOfferRef OfferRef = MakeShared<FOnlineStoreOffer>();
			OfferRef->OfferId = PlatformProduct->Sku;
			const int64 Price = PlatformProduct->Price.IsNumeric() ? FCString::Atoi(*PlatformProduct->Price) : 0;
			OfferRef->NumericPrice = Price;
			OfferRef->RegularPrice = Price;
			OfferRef->Title = FText::FromString(PlatformProduct->Name);
			OfferRef->Description = FText::FromString(PlatformProduct->Description);
			OfferRef->DynamicFields.Add(TEXT("Sku"), PlatformProduct->Sku);
			Products.Emplace(OfferRef->OfferId, OfferRef);
		}
	}

	OnlineError.bSucceeded = true;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestProductsBySku::OnGetProductsBySkuError(const FPlatformHandlerError& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	OnlineError.bSucceeded = false;
	OnlineError.ErrorCode = Response.ErrorCode;
	OnlineError.ErrorMessage = FText::FromString(Response.ErrorMessage);
	OnlineError.ErrorRaw = Response.ErrorMessage;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
