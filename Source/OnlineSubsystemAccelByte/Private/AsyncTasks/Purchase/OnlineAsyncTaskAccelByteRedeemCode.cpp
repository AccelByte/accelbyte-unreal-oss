#include "OnlineAsyncTaskAccelByteRedeemCode.h"

#include "OnlinePurchaseInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteRedeemCode::FOnlineAsyncTaskAccelByteRedeemCode(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FRedeemCodeRequest& InRedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, RedeemCodeRequest(InRedeemCodeRequest)
	, Delegate(InDelegate)
	, Language(InABSubsystem->GetLanguage())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteRedeemCode::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Initialize();
	
	if (RedeemCodeRequest.Code.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Redeemable Code is empty!"));
		Error.ErrorMessage = FText::FromString(TEXT("Redeemable Code is empty!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	THandler<FAccelByteModelsFulfillmentResult> OnSuccess = THandler<FAccelByteModelsFulfillmentResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteRedeemCode::HandleRedeemCodeComplete);
	FErrorHandler OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteRedeemCode::HandleAsyncTaskError);
	ApiClient->Fulfillment.RedeemCode(RedeemCodeRequest.Code, TEXT(""), Language, OnSuccess, OnError);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRedeemCode::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Finalize();

	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(Subsystem->GetPurchaseInterface());
	PurchaseInterface->AddReceipt(UserId.ToSharedRef(), Receipt);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRedeemCode::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	Error.bSucceeded = bWasSuccessful;

	Delegate.ExecuteIfBound(Error, MakeShared<FPurchaseReceipt>(Receipt));
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRedeemCode::HandleRedeemCodeComplete(const FAccelByteModelsFulfillmentResult& Result)
{
	for (const auto& Entitlement : Result.EntitlementSummaries)
	{
		TSharedPtr<FOnlineStoreOffer> Offer = Subsystem->GetStoreV2Interface()->GetOffer(Entitlement.ItemId);
		FPurchaseReceipt::FReceiptOfferEntry ReceiptOfferEntry;
		ReceiptOfferEntry.Quantity = Entitlement.Stackable ? Entitlement.StackedQuantity : 1;
		ReceiptOfferEntry.Namespace = Entitlement.Namespace;
		ReceiptOfferEntry.OfferId = Entitlement.ItemId;
		FPurchaseReceipt::FLineItemInfo ItemInfo;
		ItemInfo.ItemName = Offer.IsValid() ? *Offer->DynamicFields.Find("Name") : TEXT("");
		ItemInfo.UniqueId = Entitlement.Id;
		ReceiptOfferEntry.LineItems.Add(ItemInfo);

		Receipt.ReceiptOffers.Add(ReceiptOfferEntry);
	}

	for (const auto& Credit : Result.CreditSummaries)
	{
		UE_LOG_AB(Log, TEXT("Credit Redeemed to Wallet! WalletId: %s | Amount: %s"), *Credit.WalletId, Credit.Amount);
	}

	//Receipt.TransactionId = Result.OrderNo;
	Receipt.TransactionState = EPurchaseTransactionState::Purchased;
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteRedeemCode::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	Error.bSucceeded = false;
	Error.ErrorCode = FString::FromInt(Code);
	Error.ErrorMessage = FText::FromString(ErrMsg);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}