// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlinePurchaseInterfaceAccelByte.h"
#include "AsyncTasks/Purchase/OnlineAsyncTaskAccelByteCheckout.h"
#include "AsyncTasks/Purchase/OnlineAsyncTaskAccelByteRedeemCode.h"
#include "OnlineSubsystemUtils.h"

FOnlinePurchaseAccelByte::FOnlinePurchaseAccelByte(FOnlineSubsystemAccelByte* InSubsystem) : AccelByteSubsystem(InSubsystem)
{
}

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
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
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
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCheckout>(AccelByteSubsystem, UserId, CheckoutRequest, Delegate);
}

void FOnlinePurchaseAccelByte::FinalizePurchase(const FUniqueNetId& UserId, const FString& ReceiptId)
{
}

void FOnlinePurchaseAccelByte::RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRedeemCode>(AccelByteSubsystem, UserId, RedeemCodeRequest, Delegate);
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
