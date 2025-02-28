// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCheckout.h"
#include "OnlinePurchaseInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlinePurchaseSystemAccelByte"

FOnlineAsyncTaskAccelByteCheckout::FOnlineAsyncTaskAccelByteCheckout(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	const FUniqueNetId& InUserId,
	const FPurchaseCheckoutRequest& InCheckoutRequest,
	const FOnPurchaseCheckoutComplete& InDelegate) 
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, CheckoutRequest(InCheckoutRequest)
	, Delegate(InDelegate)
	, ErrorCode(TEXT(""))
	, ErrorMessage(TEXT(""))
	, Language(InABSubsystem->GetLanguage())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);	
}

void FOnlineAsyncTaskAccelByteCheckout::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Initialize();

	if(CheckoutRequest.PurchaseOffers.Num() > 1)
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Purchasing multiple item is not supported! Total: %d. It will only checkout the first offer."), CheckoutRequest.PurchaseOffers.Num());
	}
	else if (CheckoutRequest.PurchaseOffers.Num() == 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Purchase Offer is empty! "));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	const TSharedPtr<FOnlineStoreOffer> Offer = SubsystemPin->GetStoreV2Interface()->GetOffer(CheckoutRequest.PurchaseOffers[0].OfferId);
	if (!Offer.IsValid())
	{
		ErrorMessage = "Could not find Offer by ID! ";
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Could not find Offer by ID! "));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	FAccelByteModelsOrderCreate OrderRequest;
	OrderRequest.Language = Language;
	OrderRequest.ItemId = CheckoutRequest.PurchaseOffers[0].OfferId;
	OrderRequest.Quantity = CheckoutRequest.PurchaseOffers[0].Quantity;
	OrderRequest.Price = Offer->RegularPrice*OrderRequest.Quantity;
	OrderRequest.DiscountedPrice = Offer->NumericPrice*OrderRequest.Quantity;
	OrderRequest.CurrencyCode = Offer->CurrencyCode;
	if(FString* Region = Offer->DynamicFields.Find(TEXT("Region")))
	{
		OrderRequest.Region = *Region;
	}

	if (!CheckoutRequest.PurchaseOffers[0].OfferNamespace.IsEmpty())
	{
		OrderRequest.SectionId = CheckoutRequest.PurchaseOffers[0].OfferNamespace;
	}

	THandler<FAccelByteModelsOrderInfo> OnSuccess = TDelegateUtils<THandler<FAccelByteModelsOrderInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCheckout::HandleCheckoutComplete);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCheckout::HandleAsyncTaskError);
	API_FULL_CHECK_GUARD(Order, ErrorMessage);
	Order->CreateNewOrder(OrderRequest, OnSuccess, OnError);

	PaymentEventPayload.ItemId = OrderRequest.ItemId;
	PaymentEventPayload.Price = OrderRequest.Price;
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckout::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Finalize();
	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(SubsystemPin->GetPurchaseInterface());
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	
	if (bWasSuccessful && PurchaseInterface.IsValid())
	{
		PurchaseInterface->AddReceipt(UserId.ToSharedRef(), Receipt);
	}

	if (PaymentEventPayload.UserId.IsEmpty() && UserId.IsValid())
	{
		PaymentEventPayload.UserId = UserId->GetAccelByteId();
	}
	if (PredefinedEventInterface.IsValid() && !PaymentEventPayload.Status.IsEmpty())
	{
		switch (FAccelByteUtilities::GetUEnumValueFromString<EAccelByteOrderStatus>(PaymentEventPayload.Status))
		{
			case EAccelByteOrderStatus::CHARGED:
			case EAccelByteOrderStatus::FULFILLED:
			case EAccelByteOrderStatus::CHARGEBACK_REVERSED:
			case EAccelByteOrderStatus::FULFILL_FAILED:
				PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPaymentSuccededPayload>(PaymentEventPayload));
				break;
			case EAccelByteOrderStatus::CHARGEBACK:
			case EAccelByteOrderStatus::REFUNDING:
			case EAccelByteOrderStatus::REFUNDED:
			case EAccelByteOrderStatus::REFUND_FAILED:
				PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPaymentFailedPayload>(PaymentEventPayload));
			default:
				break;
		}
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckout::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::TriggerDelegates();

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)), MakeShared<FPurchaseReceipt>(Receipt));
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckout::HandleCheckoutComplete(const FAccelByteModelsOrderInfo& Result)
{
	FPurchaseReceipt::FReceiptOfferEntry ReceiptOfferEntry;
	ReceiptOfferEntry.Quantity = Result.Quantity;
	ReceiptOfferEntry.Namespace = Result.Namespace;
	ReceiptOfferEntry.OfferId = Result.ItemId;
	FPurchaseReceipt::FLineItemInfo ItemInfo;
	ItemInfo.ItemName = Result.ItemSnapshot.Name;
	// need help, is this correct?
	ItemInfo.ValidationInfo = Result.ItemSnapshot.ItemType == EAccelByteItemType::CODE ? TEXT("Redeemable") : TEXT("");
	// need to query entitlement 
	// ItemInfo.UniqueId = 
	ReceiptOfferEntry.LineItems.Add(ItemInfo);
	
	Receipt.ReceiptOffers.Add(ReceiptOfferEntry);
	Receipt.TransactionId = Result.OrderNo;
	switch (Result.Status)
	{
	case EAccelByteOrderStatus::FULFILLED:
		Receipt.TransactionState = EPurchaseTransactionState::Purchased;
		break;
	case EAccelByteOrderStatus::INIT:
		Receipt.TransactionState = EPurchaseTransactionState::Processing;
		break;
	default:
		// need help for other cases, not sure how to handle
		break;
	}
	
	PaymentEventPayload.OrderNo = Result.OrderNo;
	PaymentEventPayload.PaymentOrderNo = Result.PaymentOrderNo;
	PaymentEventPayload.ItemId = Result.ItemId;
	PaymentEventPayload.Price = Result.Price;
	PaymentEventPayload.UserId = Result.UserId;
	PaymentEventPayload.Status = FAccelByteUtilities::GetUEnumValueAsString(Result.Status);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteCheckout::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE