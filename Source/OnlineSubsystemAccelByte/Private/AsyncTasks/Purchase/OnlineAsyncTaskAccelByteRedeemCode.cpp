// Copyright (c) 2022 - 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRedeemCode.h"

#include "OnlinePurchaseInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

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

	THandler<FAccelByteModelsFulfillmentResult> OnSuccess = TDelegateUtils<THandler<FAccelByteModelsFulfillmentResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRedeemCode::HandleRedeemCodeComplete);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRedeemCode::HandleAsyncTaskError);
	FString ONLINE_ERROR_NAMESPACE = "FOnlineAsyncTaskAccelByteRedeemCode";
	API_FULL_CHECK_GUARD(Fulfillment, Error);
	Fulfillment->RedeemCode(RedeemCodeRequest.Code, TEXT(""), Language, OnSuccess, OnError);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRedeemCode::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Finalize();
	if (bWasSuccessful)
	{
		const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(SubsystemPin->GetPurchaseInterface());
		PurchaseInterface->AddReceipt(UserId.ToSharedRef(), Receipt);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsCampaignCodeRedeemedPayload CampaignCodeRedeemedPayload{};
			CampaignCodeRedeemedPayload.Code = RedeemCodeRequest.Code;
			CampaignCodeRedeemedPayload.UserId = FulfillmentResult.UserId;
			for (const auto& EntitlementSummary : FulfillmentResult.EntitlementSummaries)
			{
				CampaignCodeRedeemedPayload.EntitlementSummaries.Add(FAccelByteModelsEntitlementSummaryEventPayload::CreateEntitlementSummaryEventPayload(EntitlementSummary));
			}
			for (const auto& CreditSummary : FulfillmentResult.CreditSummaries)
			{
				CampaignCodeRedeemedPayload.CreditSummaries.Add(FAccelByteModelsCreditSummaryEventPayload::CreateCreditSummaryEventPayload(CreditSummary));
			}
			for (const auto& SubscriptionSummary : FulfillmentResult.SubscriptionSummaries)
			{
				CampaignCodeRedeemedPayload.SubscriptionSummaries.Add(FAccelByteModelsSubscriptionSummaryEventPayload::CreateSubscriptionSummaryEventPayload(SubscriptionSummary));
			}

			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsCampaignCodeRedeemedPayload>(CampaignCodeRedeemedPayload));
		}
	}

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
	TRY_PIN_SUBSYSTEM();

	for (const auto& Entitlement : Result.EntitlementSummaries)
	{
		TSharedPtr<FOnlineStoreOffer> Offer = SubsystemPin->GetStoreV2Interface()->GetOffer(Entitlement.ItemId);
		FPurchaseReceipt::FReceiptOfferEntry ReceiptOfferEntry;
		ReceiptOfferEntry.Quantity = Entitlement.Stackable ? Entitlement.StackedQuantity : 1;
		ReceiptOfferEntry.Namespace = Entitlement.Namespace;
		ReceiptOfferEntry.OfferId = Entitlement.ItemId;
		FPurchaseReceipt::FLineItemInfo ItemInfo;
		ItemInfo.ItemName = Entitlement.Name;
		ItemInfo.UniqueId = Entitlement.Id;
		ReceiptOfferEntry.LineItems.Add(ItemInfo);

		Receipt.ReceiptOffers.Add(ReceiptOfferEntry);
	}

	for (const auto& Credit : Result.CreditSummaries)
	{
		UE_LOG_AB(Log, TEXT("Credit Redeemed to Wallet! WalletId: %s | Amount: %s %d"), *Credit.WalletId, *Credit.CurrencyCode, Credit.Amount);
	}

	//Receipt.TransactionId = Result.OrderNo;
	Receipt.TransactionState = EPurchaseTransactionState::Purchased;
	
	FulfillmentResult = Result;

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