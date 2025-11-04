// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncIOSAppStore.h"


#include "Api/AccelByteEntitlementApi.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSyncIOSAppStore::FOnlineAsyncTaskAccelByteSyncIOSAppStore(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const TSharedRef<FPurchaseReceipt>& Receipt, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
    LocalUserNum = InLocalUserNum;
    bIsFailedToInitializeDueToInvalidReceipt = true;
    //Request validity checking
    if (Receipt->TransactionId.IsEmpty())
    {
        Error = TEXT("\nInvalid IOS App Store Purchase Sync: require transaction ID.");
        return;
    }
    
    bIsFailedToInitializeDueToInvalidReceipt = false;
    SyncRequest.TransactionId = Receipt->TransactionId;
}

void FOnlineAsyncTaskAccelByteSyncIOSAppStore::Initialize()
{
	Super::Initialize();
    if(bIsFailedToInitializeDueToInvalidReceipt)
    {
        Error = "InvalidReceipt";
        CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
        return;
    }

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    FVoidHandler OnSyncPlatformPurchaseSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncIOSAppStore::OnSyncPlatformPurchaseSuccess);
	FErrorHandler OnSyncPlatformPurchaseErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncIOSAppStore::OnSyncPlatformPurchaseError);
	API_FULL_CHECK_GUARD(Entitlement, Error);

	Entitlement->SyncMobilePlatformPurchaseApple(SyncRequest, OnSyncPlatformPurchaseSuccessDelegate, OnSyncPlatformPurchaseErrorDelegate);

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
