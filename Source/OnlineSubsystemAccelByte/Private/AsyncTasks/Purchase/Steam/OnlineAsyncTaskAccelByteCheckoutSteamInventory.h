// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlinePurchaseInterfaceAccelByte.h"

#if defined(STEAM_SDK_VER) && !UE_SERVER
#include "steam/steam_api.h"
#endif // defined(STEAM_SDK_VER) && !UE_SERVER

using FInventoryReadyDelegate = TDelegate<void()>;

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteCheckoutSteamInventory
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteCheckoutSteamInventory, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCheckoutSteamInventory(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InUserId
		, const FPurchaseCheckoutRequest& InCheckoutRequest
		, const FOnPurchaseCheckoutComplete& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteCheckoutSteamInventory");
	}

private:
	/**
	 * Checkout request provided by the caller to this checkout call.
	 */
	FPurchaseCheckoutRequest CheckoutRequest{};

	/**
	 * Delegate triggered when the purchase has been completed.
	 */
	FOnPurchaseCheckoutComplete CompletionDelegate{};

	/**
	 * Error structure returned at the end of this task.
	 */
	FOnlineError OnlineError{};

	/**
	 * Order ID returned by Steam once StartPurchase is complete.
	 */
	uint64 OrderId{};

	/**
	 * Receipt created once purchase has completed.
	 */
	FPurchaseReceipt Receipt{};

#if defined(STEAM_SDK_VER) && !UE_SERVER
	/**
	 * @brief Class used to handle inventory result ready callback from Steam
	 */
	class FSteamInventoryReadyCallback
	{
	public:
		FSteamInventoryReadyCallback(const FInventoryReadyDelegate& InDelegate);
		
	private:
		/**
		 * Delegate triggered when the inventory ready callback has been processed.
		 */
		FInventoryReadyDelegate Delegate{};

		STEAM_CALLBACK(FSteamInventoryReadyCallback, OnInventoryResultReady, SteamInventoryResultReady_t);
	};

	/**
	 * Unique pointer to the class that handles Steam inventory ready callbacks.
	 */
	TUniquePtr<FSteamInventoryReadyCallback> SteamInventoryReadyCallback{};

	/**
	 * Steam callback handle for when a purchase has been started.
	 */
	CCallResult<FOnlineAsyncTaskAccelByteCheckoutSteamInventory, SteamInventoryStartPurchaseResult_t> StartPurchaseInventoryResult{};
	
	/**
	 * Method for handling the callback from Steam upon starting a purchase.
	 */
	void OnStartPurchaseResult(SteamInventoryStartPurchaseResult_t* ResultCallback, bool bIoFailure);

	/**
	 * Method for handling Steam inventory result being ready, signaling that purchase has completed.
	 */
	void OnSteamInventoryReady();
#endif // defined(STEAM_SDK_VER) && !UE_SERVER

};
