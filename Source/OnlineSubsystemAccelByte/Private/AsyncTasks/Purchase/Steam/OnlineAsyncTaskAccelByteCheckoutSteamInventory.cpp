// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCheckoutSteamInventory.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteCheckoutSteamInventory"

#if defined(STEAM_SDK_VER) && !UE_SERVER
FOnlineAsyncTaskAccelByteCheckoutSteamInventory::FSteamInventoryReadyCallback::FSteamInventoryReadyCallback(const FInventoryReadyDelegate& InDelegate)
	: Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteCheckoutSteamInventory::FSteamInventoryReadyCallback::OnInventoryResultReady(SteamInventoryResultReady_t* Response)
{
	if (Response->m_result != k_EResultOK)
	{
		SteamInventory()->DestroyResult(Response->m_handle);
		return;
	}

	AsyncTask(ENamedThreads::GameThread, [ReadyDelegate = Delegate]() {
		ReadyDelegate.ExecuteIfBound();
	});
	
	SteamInventory()->DestroyResult(Response->m_handle);
}
#endif // defined(STEAM_SDK_VER) && !UE_SERVER

FOnlineAsyncTaskAccelByteCheckoutSteamInventory::FOnlineAsyncTaskAccelByteCheckoutSteamInventory(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, const FPurchaseCheckoutRequest& InCheckoutRequest
	, const FOnPurchaseCheckoutComplete& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, CheckoutRequest(InCheckoutRequest)
	, CompletionDelegate(InCompletionDelegate)
{
	// #NOTE We want to make this task have a longer timeout to allow for a user to complete their purchase. In a worst
	// case scenario, the user would need to add their billing information and address before checkout can complete. With
	// this in mind, this task has a timeout of four minutes. Unfortunately, Steam does not provide concrete callbacks
	// for when a purchase has been cancelled. So if the player decides not to purchase their items, the task will use
	// the full timeout before dropping off.
	TaskTimeoutInSeconds = 240.0;

	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteCheckoutSteamInventory::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s"), *UserId->ToDebugString());

	if (CheckoutRequest.PurchaseOffers.Num() == 0)
	{
		const FText ErrorText = FText::FromString(TEXT("platform-checkout-no-offers"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::InvalidParams, FString(), ErrorText);

		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Failed to start Steam inventory purchase: No offers provided in checkout request."));
		return;
	}

#if defined(STEAM_SDK_VER) && !UE_SERVER
	const FInventoryReadyDelegate OnInventoryReadyDelegate =
		TDelegateUtils<FInventoryReadyDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCheckoutSteamInventory::OnSteamInventoryReady);
	SteamInventoryReadyCallback = MakeUnique<FSteamInventoryReadyCallback>(OnInventoryReadyDelegate);

	// Go through each purchase offer in the checkout request and store ID and quantity in array
	TArray<int32> Ids{};
	TArray<uint32> Quantities{};

	for (const FPurchaseCheckoutRequest::FPurchaseOfferEntry& PurchaseOffer : CheckoutRequest.PurchaseOffers)
	{
		Ids.Emplace(FCString::Atoi(*PurchaseOffer.OfferId));
		Quantities.Emplace(PurchaseOffer.Quantity);
	}

	// Make Steam API call to start purchasing the given inventory items
	SteamAPICall_t StartPurchaseCall = SteamInventory()->StartPurchase(Ids.GetData()
		, Quantities.GetData()
		, Ids.Num());
	if (StartPurchaseCall == k_uAPICallInvalid)
	{
		const FText ErrorText = FText::FromString(TEXT("platform-checkout-steam-call-failed"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);

		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to start Steam inventory purchase: Steam API call invalid"));
		return;
	}

	StartPurchaseInventoryResult.Set(StartPurchaseCall
		, this
		, &FOnlineAsyncTaskAccelByteCheckoutSteamInventory::OnStartPurchaseResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
#else
	const FText ErrorText = FText::FromString(TEXT("platform-checkout-steam-unavailable"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::NotConfigured, FString(), ErrorText);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Steamworks is not available for this task."));
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
#endif // defined(STEAM_SDK_VER) && !UE_SERVER
}

void FOnlineAsyncTaskAccelByteCheckoutSteamInventory::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s; bWasSuccessful: %s"), *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful));
	
	// Start by creating purchase receipt from this purchase
	Receipt.TransactionId = FString::Printf(TEXT("%lld"), OrderId);
	Receipt.TransactionState = (bWasSuccessful) ? EPurchaseTransactionState::Purchased : EPurchaseTransactionState::Failed;
	
	// #NOTE: Not adding line items as our purchase sync API only requires the order ID

	TRY_PIN_SUBSYSTEM();
	
	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(SubsystemPin->GetPurchaseInterface());
	if (bWasSuccessful && PurchaseInterface.IsValid())
	{
		PurchaseInterface->AddPlatformReceipt(UserId.ToSharedRef(), Receipt);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckoutSteamInventory::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s; bWasSuccessful: %s"), *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful));

	const bool bExecuted = CompletionDelegate.ExecuteIfBound(OnlineError, MakeShared<FPurchaseReceipt>(Receipt));
	if (!bExecuted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Can't trigger delegate as it is unbound."));
		return;
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#if defined(STEAM_SDK_VER) && !UE_SERVER
void FOnlineAsyncTaskAccelByteCheckoutSteamInventory::OnStartPurchaseResult(SteamInventoryStartPurchaseResult_t* ResultCallback, bool bIoFailure)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; OrderId: %lld"), *UserId->ToDebugString(), ResultCallback->m_ulOrderID);

	if (ResultCallback->m_result != k_EResultOK)
	{
		const FText ErrorText = FText::FromString(TEXT("platform-checkout-steam-start-purchase-failed"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);

		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Result: %d"), static_cast<int32>(ResultCallback->m_result));
		return;
	}

	OrderId = ResultCallback->m_ulOrderID;

	// #NOTE Purchase is not complete at this point, we need to wait for SteamInventoryResultReady for purchase complete

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckoutSteamInventory::OnSteamInventoryReady()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; OrderId: %lld"), *UserId->ToDebugString(), OrderId);

	// Purchase is complete at this point, so complete the task
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
#endif // defined(STEAM_SDK_VER) && !UE_SERVER

#undef ONLINE_ERROR_NAMESPACE
