// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCheckoutMetaQuestProduct.h"

#include "Core/Platform/AccelBytePlatformHandler.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct"

FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, const FPurchaseCheckoutRequest& InPurchaseCheckoutRequest
	, const FOnPurchaseCheckoutComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, CheckoutRequest(InPurchaseCheckoutRequest)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	if (CheckoutRequest.PurchaseOffers.Num() > 1)
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Purchasing multiple item is not supported! Total: %d. It will only checkout the first offer."), CheckoutRequest.PurchaseOffers.Num());
	}
	else if (CheckoutRequest.PurchaseOffers.Num() == 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Purchase Offer is empty! "));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	FAccelBytePlatformHandler PlatformHandler;
	const AccelBytePlatformWrapperWPtr PlatformWrapperWPtr = PlatformHandler.GetPlatformWrapper(EAccelBytePlatformType::Oculus);
	const AccelBytePlatformWrapperPtr PlatformWrapper = PlatformWrapperWPtr.Pin();
		
	if (!PlatformWrapper.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Can't fetch platform friends as platform wrapper is invalid."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const auto OnSuccess = TDelegateUtils<TDelegate<void(const FPlatformPurchasePtr&)>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::OnCheckoutProductSuccess);
	const auto OnError = TDelegateUtils<TDelegate<void(const FPlatformHandlerError&)>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::OnCheckoutProductError);
	PlatformWrapper->CheckoutProduct(CheckoutRequest.PurchaseOffers[0].OfferId, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Finalize();
	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(SubsystemPin->GetPurchaseInterface());

	if (bWasSuccessful && PurchaseInterface.IsValid())
	{
		PurchaseInterface->AddPlatformReceipt(UserId.ToSharedRef(), Receipt);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d, bWasSuccessfull: %s"), *UserId->ToDebugString(), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful));

	bool bExecuted = Delegate.ExecuteIfBound(OnlineError, MakeShared<FPurchaseReceipt>(Receipt));

	if (!bExecuted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Can't trigger delegate as it is unbound."));
		return;
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::OnCheckoutProductSuccess(const FPlatformPurchasePtr& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	FPurchaseReceipt::FReceiptOfferEntry ReceiptOfferEntry;
	ReceiptOfferEntry.OfferId = Response->Sku;
	FPurchaseReceipt::FLineItemInfo ItemInfo;
	Response->Metadata.JsonObjectToString(ItemInfo.ValidationInfo);
	ReceiptOfferEntry.LineItems.Add(ItemInfo);

	Receipt.ReceiptOffers.Add(ReceiptOfferEntry);
	Receipt.TransactionId = Response->TransactionId;

	//if it successfully bought, the state supposed to be purchased
	Receipt.TransactionState = EPurchaseTransactionState::Purchased;

	OnlineError.bSucceeded = true;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct::OnCheckoutProductError(const FPlatformHandlerError& Response)
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
