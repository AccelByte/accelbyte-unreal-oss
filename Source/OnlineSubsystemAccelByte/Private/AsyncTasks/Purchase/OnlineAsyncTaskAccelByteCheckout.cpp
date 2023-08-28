#include "OnlineAsyncTaskAccelByteCheckout.h"

#include "OnlinePurchaseInterfaceAccelByte.h"
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineStoreSystemAccelByte"

FOnlineAsyncTaskAccelByteCheckout::FOnlineAsyncTaskAccelByteCheckout(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	const FUniqueNetId& InUserId,
	const FPurchaseCheckoutRequest& InCheckoutRequest,
	const FOnPurchaseCheckoutComplete& InDelegate) 
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, CheckoutRequest(InCheckoutRequest)
	, Delegate(InDelegate)
	, ErrorCode(TEXT(""))
	, ErrorMessage(FText::FromString(TEXT("")))
	, Language(InABSubsystem->GetLanguage())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);	
}

void FOnlineAsyncTaskAccelByteCheckout::Initialize()
{
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
	
	TSharedPtr<FOnlineStoreOffer> Offer = Subsystem->GetStoreV2Interface()->GetOffer(CheckoutRequest.PurchaseOffers[0].OfferId);

	FAccelByteModelsOrderCreate OrderRequest;
	OrderRequest.Language = Language;
	OrderRequest.ItemId = CheckoutRequest.PurchaseOffers[0].OfferId;
	OrderRequest.Quantity = CheckoutRequest.PurchaseOffers[0].Quantity;
	OrderRequest.Price = Offer->RegularPrice;
	OrderRequest.DiscountedPrice = Offer->NumericPrice;
	OrderRequest.CurrencyCode = Offer->CurrencyCode;
	if(FString* Region = Offer->DynamicFields.Find(TEXT("Region")))
	{
		OrderRequest.Region = *Region;
	}
	
	THandler<FAccelByteModelsOrderInfo> OnSuccess = TDelegateUtils<THandler<FAccelByteModelsOrderInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCheckout::HandleCheckoutComplete);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCheckout::HandleAsyncTaskError);
	ApiClient->Order.CreateNewOrder(OrderRequest, OnSuccess, OnError);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckout::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Finalize();
	
	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(Subsystem->GetPurchaseInterface());
	PurchaseInterface->AddReceipt(UserId.ToSharedRef(), Receipt);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckout::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::TriggerDelegates();

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, ErrorMessage), MakeShared<FPurchaseReceipt>(Receipt));
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
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteCheckout::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = FText::FromString(ErrMsg);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE