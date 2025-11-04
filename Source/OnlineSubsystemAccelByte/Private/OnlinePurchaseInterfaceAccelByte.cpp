// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlinePurchaseInterfaceAccelByte.h"
#include "AsyncTasks/Purchase/OnlineAsyncTaskAccelByteCheckout.h"
#include "AsyncTasks/Purchase/OnlineAsyncTaskAccelByteRedeemCode.h"
#include "AsyncTasks/Purchase/OnlineAsyncTaskAccelByteQueryUserOrders.h"
#include "AsyncTasks/Purchase/OnlineAsyncTaskAccelByteCreateNewOrder.h"
#include "AsyncTasks/Purchase/OnlineAsyncTaskAccelBytePreviewOrder.h"
#include "AsyncTasks/Purchase/MetaQuest/OnlineAsyncTaskAccelByteCheckoutMetaQuestProduct.h"
#include "AsyncTasks/Purchase/MetaQuest/OnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts.h"
#include "AsyncTasks/Purchase/Steam/OnlineAsyncTaskAccelByteCheckoutSteamInventory.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteLog.h"

FOnlinePurchaseAccelByte::FOnlinePurchaseAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{}

void FOnlinePurchaseAccelByte::AddReceipt(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, FPurchaseReceipt Receipt)
{
	FScopeLock ScopeLock(&ReceiptMapLock);
	FReceiptMap& ReceiptMap = PurchaseReceipts.FindOrAdd(UserId);
	ReceiptMap.Emplace(Receipt.TransactionId, Receipt);
}

bool FOnlinePurchaseAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlinePurchaseAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(Subsystem->GetPurchaseInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlinePurchaseAccelByte::GetFromWorld(const UWorld* World, FOnlinePurchaseAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlinePurchaseAccelByte::IsAllowedToPurchase(const FUniqueNetId& UserId)
{
	return true;
}

void FOnlinePurchaseAccelByte::Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate)
{
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCheckout>(AccelByteSubsystemPtr.Get(), UserId, CheckoutRequest, Delegate);
}

void FOnlinePurchaseAccelByte::FinalizePurchase(const FUniqueNetId& UserId, const FString& ReceiptId)
{
}

void FOnlinePurchaseAccelByte::RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRedeemCode>(AccelByteSubsystemPtr.Get(), UserId, RedeemCodeRequest, Delegate);
}

void FOnlinePurchaseAccelByte::QueryReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate)
{
}

void FOnlinePurchaseAccelByte::GetReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	FScopeLock ScopeLock(&ReceiptMapLock);
	const FReceiptMap* ReceiptMap = PurchaseReceipts.Find(SharedUserId);
	if(ReceiptMap)
	{
		ReceiptMap->GenerateValueArray(OutReceipts);
	}
}

void FOnlinePurchaseAccelByte::FinalizeReceiptValidationInfo(const FUniqueNetId& UserId,
	FString& InReceiptValidationInfo, const FOnFinalizeReceiptValidationInfoComplete& Delegate)
{
}

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
void FOnlinePurchaseAccelByte::Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseReceiptlessCheckoutComplete& Delegate)
{
	// @TODO: Implement checkout without receipt
	UE_LOG_AB(Warning, TEXT("FOnlinePurchaseAccelByte::Checkout without receipt is not currently supported."));
}
#endif

void FOnlinePurchaseAccelByte::PlatformCheckout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), UserId.IsValid() ? *UserId.ToDebugString() : TEXT(""));
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	if (AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Oculus"))
		|| AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Meta")))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCheckoutMetaQuestProduct>(AccelByteSubsystemPtr.Get()
			, UserId
			, CheckoutRequest
			, Delegate);
	}
	else if (AccelByteSubsystemPtr->GetNativePlatformNameString().Equals(TEXT("Steam"), ESearchCase::IgnoreCase))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCheckoutSteamInventory>(AccelByteSubsystemPtr.Get()
			, UserId
			, CheckoutRequest
			, Delegate);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
}

void FOnlinePurchaseAccelByte::QueryPlatformReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), UserId.IsValid() ? *UserId.ToDebugString() : TEXT(""));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	if (AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Oculus"))
		|| AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Meta")))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetMetaQuestPurchasedProducts>(AccelByteSubsystemPtr.Get(), UserId, Delegate);
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
}

void FOnlinePurchaseAccelByte::GetPlatformReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	FScopeLock ScopeLock(&PlatformReceiptMapLock);
	const FReceiptMap* ReceiptMap = PlatformReceipts.Find(SharedUserId);
	if (ReceiptMap)
	{
		ReceiptMap->GenerateValueArray(OutReceipts);
	}
}

void FOnlinePurchaseAccelByte::AddPlatformReceipt(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, FPurchaseReceipt Receipt)
{
	FScopeLock ScopeLock(&PlatformReceiptMapLock);
	FReceiptMap& ReceiptMap = PlatformReceipts.FindOrAdd(UserId);
	ReceiptMap.Emplace(Receipt.TransactionId, Receipt);
}

void FOnlinePurchaseAccelByte::QueryUserOrders(const FUniqueNetId& UserId, const FAccelByteModelsUserOrdersRequest& UserOrderRequest)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserOrders>(AccelByteSubsystemPtr.Get(), UserId, UserOrderRequest);
}

void FOnlinePurchaseAccelByte::CreateNewOrder(const FUniqueNetId& UserId, const FAccelByteModelsOrderCreate& OrderCreate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateNewOrder>(AccelByteSubsystemPtr.Get(), UserId, OrderCreate);
}

void FOnlinePurchaseAccelByte::PreviewOrder(const FUniqueNetId& UserId,	const FAccelByteModelsUserPreviewOrderRequest& PreviewOrderRequest)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelBytePreviewOrder>(AccelByteSubsystemPtr.Get(), UserId, PreviewOrderRequest);
}