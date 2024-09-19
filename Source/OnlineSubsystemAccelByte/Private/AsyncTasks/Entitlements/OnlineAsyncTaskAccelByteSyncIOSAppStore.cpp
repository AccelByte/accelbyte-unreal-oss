// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncIOSAppStore.h"

#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteEntitlementApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSyncIOSAppStore::FOnlineAsyncTaskAccelByteSyncIOSAppStore(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const TSharedRef<FPurchaseReceipt>& Receipt, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
    
    //Request validity checking
    {
        Error = "";
        if (Receipt->ReceiptOffers.Num() != 1) //Should not be zero or less & should not be more than one
        {
            Error += "ReceiptOffers amount is invalid.";
        }
        else if (Receipt->ReceiptOffers[0].OfferId.IsEmpty())
        {
            Error += "The receipt OfferId (productIdentifier) is empty.";
        }
        else if (Receipt->ReceiptOffers[0].LineItems.Num() != 1) //Should not be zero or less & should not be more than one
        {
            Error += "ReceiptOffers[0].LineItems amount is invalid.";
        }
        else if (Receipt->ReceiptOffers[0].LineItems[0].ValidationInfo.IsEmpty())
        {
            Error += "The item ValidationInfo is empty.";
        }
        else if (Receipt->ReceiptOffers[0].LineItems[0].UniqueId.IsEmpty())
        {
            Error += "The item UniqueId is empty.";
        }
        
        if (!Error.IsEmpty())
        {
            Error += TEXT("\nInvalid IOS App Store Purchase Sync: malformed receipt.");
            bIsFailedToInitializeDueToInvalidReceipt = true;
            return;
        }
    }
    
    bIsFailedToInitializeDueToInvalidReceipt = false;
    
    ReceiptCopy.ExcludeOldTransactions = false;
    ReceiptCopy.ReceiptData = Receipt->ReceiptOffers[0].LineItems[0].ValidationInfo;
    ReceiptCopy.TransactionId = Receipt->ReceiptOffers[0].LineItems[0].UniqueId;
    ReceiptCopy.ProductId = Receipt->ReceiptOffers[0].OfferId;
}

void FOnlineAsyncTaskAccelByteSyncIOSAppStore::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    FVoidHandler OnSyncPlatformPurchaseSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncIOSAppStore::OnSyncPlatformPurchaseSuccess);
	FErrorHandler OnSyncPlatformPurchaseErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncIOSAppStore::OnSyncPlatformPurchaseError);
	API_CLIENT_CHECK_GUARD(Error);

	ApiClient->Entitlement.SyncMobilePlatformPurchaseApple(ReceiptCopy, OnSyncPlatformPurchaseSuccessDelegate, OnSyncPlatformPurchaseErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncIOSAppStore::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, Error);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncIOSAppStore::OnSyncPlatformPurchaseSuccess()
{
	UE_LOG_AB(Log, TEXT("Successfully synced IOS App Store purchase"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteSyncIOSAppStore::OnSyncPlatformPurchaseError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to sync IOS App Store purchase! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	Error = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
