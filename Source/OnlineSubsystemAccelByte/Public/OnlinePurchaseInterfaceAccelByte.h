// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemAccelByteTypes.h"

using FReceiptMap = TMap<FString, FPurchaseReceipt>;
using FUserIDToReceiptMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FReceiptMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FReceiptMap>>;

class ONLINESUBSYSTEMACCELBYTE_API FOnlinePurchaseAccelByte : public IOnlinePurchase
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a purchase interface instance */
	FOnlinePurchaseAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	void AddReceipt(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, FPurchaseReceipt Receipt);
	
public:
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlinePurchaseAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlinePurchaseAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	virtual bool IsAllowedToPurchase(const FUniqueNetId& UserId) override;
	virtual void Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate) override;
	virtual void FinalizePurchase(const FUniqueNetId& UserId, const FString& ReceiptId) override;
	virtual void RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate) override;
	virtual void QueryReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate) override;
	virtual void GetReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const override;
	virtual void FinalizeReceiptValidationInfo(const FUniqueNetId& UserId, FString& InReceiptValidationInfo, const FOnFinalizeReceiptValidationInfoComplete& Delegate) override;

protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	FUserIDToReceiptMap PurchaseReceipts;
	/** Critical sections for thread safe operation of ReceiptMap */
	mutable FCriticalSection ReceiptMapLock;
};
