// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncPlatformGooglePlay.h"


#include "Api/AccelByteEntitlementApi.h"
#include "Runtime/Core/Public/Misc/Base64.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSyncGooglePlay::FOnlineAsyncTaskAccelByteSyncGooglePlay(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, FAccelByteModelsPlatformSyncMobileGoogle InRequest, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Request(InRequest)
	, Delegate(InDelegate)
{
	if (!SetLocalUserNum(InLocalUserNum))
	{
		Error = TEXT("LocalUserNum is Invalid!");
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}
	Error = TEXT("");
}

FOnlineAsyncTaskAccelByteSyncGooglePlay::FOnlineAsyncTaskAccelByteSyncGooglePlay(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const TSharedRef<FPurchaseReceipt>& Receipt, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
	Error = TEXT("");
	if (!SetLocalUserNum(InLocalUserNum))
	{
		bIsFailedToInitialize = true;
		Error = TEXT("LocalUserNum is Invalid!");
		return;
	}

	//Request validity checking
	{
		if (Receipt->ReceiptOffers.Num() != 1) //Should not be zero or less & should not be more than one
		{
			Error = TEXT("ReceiptOffers amount is invalid.");
		}
		else if (Receipt->ReceiptOffers[0].LineItems.Num() != 1) //Should not be zero or less & should not be more than one
		{
			Error = TEXT("ReceiptOffers[0].LineItems amount is invalid.");
		}
		else if (!ParsePurchaseReceiptToPlatformSyncMobileGoogle(Receipt))
		{
			Error = TEXT("Failed to parse PurchaseReceipt.");
		}

		if (!Error.IsEmpty())
		{
			bIsFailedToInitialize = true;
			return;
		}
	}

}

void FOnlineAsyncTaskAccelByteSyncGooglePlay::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bIsFailedToInitialize)
	{
		UE_LOG_AB(Warning, TEXT("Invalid GooglePlay Purchase Sync: Failed to initialize.\nDetail: %s"), *Error);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (!Request.SubscriptionPurchase)
	{
		THandler<FAccelByteModelsItemInfo> OnGetItemSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsItemInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncGooglePlay::OnGetItemBySkuSuccess);
		FErrorHandler OnGetItemBySkuErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncGooglePlay::OnRequestError);
		API_FULL_CHECK_GUARD(Item, Error);
		Item->GetItemBySku(Request.ProductId, TEXT(""), TEXT(""), OnGetItemSuccessDelegate, OnGetItemBySkuErrorDelegate);
	}
	else
	{
		SyncPlatformPurchase();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncGooglePlay::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, Error);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncGooglePlay::OnGetItemBySkuSuccess(const FAccelByteModelsItemInfo& Response)
{
	UE_LOG_AB(Log, TEXT("Successfully Get Item"));
	SetLastUpdateTimeToCurrentTime();
	if (Response.EntitlementType == EAccelByteEntitlementType::DURABLE)
	{
		Request.AutoAck = true;
	}
	SyncPlatformPurchase();
}

void FOnlineAsyncTaskAccelByteSyncGooglePlay::OnSyncPlatformPurchaseSuccess(const FAccelByteModelsPlatformSyncMobileGoogleResponse& Response)
{
	UE_LOG_AB(Log, TEXT("Successfully synced platform purchase"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteSyncGooglePlay::OnRequestError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to sync platform purchase! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	Error = FString::Printf(TEXT("%d: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteSyncGooglePlay::SyncPlatformPurchase()
{
	THandler<FAccelByteModelsPlatformSyncMobileGoogleResponse> OnSyncPlatformPurchaseSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsPlatformSyncMobileGoogleResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncGooglePlay::OnSyncPlatformPurchaseSuccess);
	FErrorHandler OnSyncPlatformPurchaseErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncGooglePlay::OnRequestError);

	API_FULL_CHECK_GUARD(Entitlement, Error);
	Entitlement->SyncMobilePlatformPurchaseGooglePlay(Request, OnSyncPlatformPurchaseSuccessDelegate, OnSyncPlatformPurchaseErrorDelegate);
}

bool FOnlineAsyncTaskAccelByteSyncGooglePlay::ParsePurchaseReceiptToPlatformSyncMobileGoogle(const TSharedRef<FPurchaseReceipt>& Receipt)
{
	if (Receipt->ReceiptOffers.Num() > 0)
	{
		TSharedPtr<FJsonObject> JsonObject;
		if (Receipt->ReceiptOffers[0].LineItems.Num() > 0)
		{
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Receipt->ReceiptOffers[0].LineItems[0].ValidationInfo);
			if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
			{
				if (JsonObject.IsValid())
				{
					FString ReceiptData;
					if (JsonObject->TryGetStringField(TEXT("receiptData"), ReceiptData))
					{
						FString DecodedReceiptData;
						FBase64::Decode(ReceiptData, DecodedReceiptData);
						if (FAccelByteJsonConverter::JsonObjectStringToUStruct(DecodedReceiptData, &Request))
						{
							bool bIsSubscription = false;
							if (JsonObject->TryGetBoolField(TEXT("isSubscription"), bIsSubscription))
							{
								Request.SubscriptionPurchase = bIsSubscription;
							}
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}