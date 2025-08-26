// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts.h"

#include "Core/Platform/AccelBytePlatformHandler.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts"

FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, const FOnQueryReceiptsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	
	FAccelBytePlatformHandler PlatformHandler;
	const AccelBytePlatformWrapperWPtr PlatformWrapperWPtr = PlatformHandler.GetPlatformWrapper(EAccelBytePlatformType::Oculus);
	const AccelBytePlatformWrapperPtr PlatformWrapper = PlatformWrapperWPtr.Pin();
		
	if (!PlatformWrapper.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Can't fetch platform friends as platform wrapper is invalid."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const auto OnSuccess = TDelegateUtils<TDelegate<void(const TArray<FPlatformPurchasePtr>&)>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::OnGetPurchasesSuccess);
	const auto OnError = TDelegateUtils<TDelegate<void(const FPlatformHandlerError&)>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::OnGetPurchasesError);
	PlatformWrapper->GetPurchasedProducts(OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Finalize();
	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(SubsystemPin->GetPurchaseInterface());

	if (bWasSuccessful && PurchaseInterface.IsValid())
	{
		for (const auto& Receipt : Purchases)
		{
			PurchaseInterface->AddPlatformReceipt(UserId.ToSharedRef(), Receipt);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	bool bExecuted = Delegate.ExecuteIfBound(OnlineError);

	if (!bExecuted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Can't trigger delegate as it is unbound."));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::OnGetPurchasesSuccess(const TArray<FPlatformPurchasePtr>& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	for (const auto& Purchase : Response)
	{
		FPurchaseReceipt Receipt{};
		FPurchaseReceipt::FReceiptOfferEntry ReceiptOfferEntry;
		ReceiptOfferEntry.OfferId = Purchase->Sku;
		FPurchaseReceipt::FLineItemInfo ItemInfo;
		Purchase->Metadata.JsonObjectToString(ItemInfo.ValidationInfo);
		ReceiptOfferEntry.LineItems.Add(ItemInfo);

		Receipt.ReceiptOffers.Add(ReceiptOfferEntry);
		Receipt.TransactionId = Purchase->TransactionId;

		//if it successfully bought, the state supposed to be purchased
		Receipt.TransactionState = EPurchaseTransactionState::Purchased;
		Purchases.Add(Receipt);
	}

	OnlineError.bSucceeded = true;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts::OnGetPurchasesError(const FPlatformHandlerError& Response)
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
