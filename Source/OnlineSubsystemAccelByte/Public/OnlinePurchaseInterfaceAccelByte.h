// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineErrorAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelBytePackage.h"

using FReceiptMap = TMap<FString, FPurchaseReceipt>;
using FUserIDToReceiptMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FReceiptMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FReceiptMap>>;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnQueryUserOrdersComplete, bool /* bWasSuccessful */, const FAccelByteModelsPagedOrderInfo& /* Result */, const FOnlineErrorAccelByte& /* Error */);
typedef FOnQueryUserOrdersComplete::FDelegate FOnQueryUserOrdersCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnCreateNewOrderComplete, bool /* bWasSuccessful */, const FAccelByteModelsOrderInfo& /* Result */, const FOnlineErrorAccelByte& /* Error */);
typedef FOnCreateNewOrderComplete::FDelegate FOnCreateNewOrderCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPreviewOrderComplete, bool /* bWasSuccessful */, const FAccelByteModelsUserPreviewOrderResponse& /* Result */, const FOnlineErrorAccelByte& /* Error */);
typedef FOnPreviewOrderComplete::FDelegate FOnPreviewOrderCompleteDelegate;

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

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	virtual void Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseReceiptlessCheckoutComplete& Delegate) override;
#endif

#pragma region PlatformIAP
	void PlatformCheckout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate);
	void QueryPlatformReceipts(const FUniqueNetId& UserId, bool bRestoreReceipts, const FOnQueryReceiptsComplete& Delegate);
	void GetPlatformReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts) const;
PACKAGE_SCOPE:
	void AddPlatformReceipt(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, FPurchaseReceipt Receipt);
#pragma endregion

public:
	/**
	 * Delegate called when a controller-user query user orders.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnQueryUserOrdersComplete, bool /*bWasSuccessful*/, const FAccelByteModelsPagedOrderInfo& /*PagedOrderInfo*/, const FOnlineErrorAccelByte& /*OnlineError*/);
	/**
	 *  Get all of user's orders that have been created with paging.  
	 *
	 * @param UserId The user's user ID.
	 * @param UserOrderRequest contains some parameters for query. 
	 */ 
	void QueryUserOrders(const FUniqueNetId& UserId, const FAccelByteModelsUserOrdersRequest& UserOrderRequest);

	/**
	 * Delegate called when a controller-user create new order.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnCreateNewOrderComplete, bool /*bWasSuccessful*/, const FAccelByteModelsOrderInfo& /*OrderInfo*/, const FOnlineErrorAccelByte & /*OnlineError*/);
	/**
	 *  Create order to purchase something from the store.   
	 *
	 * @param UserId The user's user ID.
	 * @param OrderCreate contains some parameters for create new order.
	 */
	void CreateNewOrder(const FUniqueNetId& UserId, const FAccelByteModelsOrderCreate& OrderCreate);

	/**
	 * Delegate called when a controller-user preview an order complete.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnPreviewOrderComplete, bool /*bWasSuccessful*/, const FAccelByteModelsUserPreviewOrderResponse& /*OrderInfo*/, const FOnlineErrorAccelByte & /*OnlineError*/);
	/**
	 *  Preview an order using discount code(s).
	 *
	 * @param UserId The user's user ID.
	 * @param PreviewOrderRequest contains some parameters for previewing the order.
	*/
	void PreviewOrder(const FUniqueNetId& UserId, const FAccelByteModelsUserPreviewOrderRequest& PreviewOrderRequest);

protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

	FUserIDToReceiptMap PurchaseReceipts;
	/** Critical sections for thread safe operation of ReceiptMap */
	mutable FCriticalSection ReceiptMapLock;

	FUserIDToReceiptMap PlatformReceipts;
	/** Critical sections for thread safe operation of ReceiptMap */
	mutable FCriticalSection PlatformReceiptMapLock;
};
