// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncPlatformPurchase.h"

#include "OnlineStoreInterfaceV2AccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteEntitlementApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSyncPlatformPurchase::FOnlineAsyncTaskAccelByteSyncPlatformPurchase(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, FAccelByteModelsEntitlementSyncBase InEntitlementSyncBase, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, EntitlementSyncBase(InEntitlementSyncBase)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
	Error = TEXT("");
}

void FOnlineAsyncTaskAccelByteSyncPlatformPurchase::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	EAccelBytePlatformSync PlatformEnum = GetNavitePlatformSyncType();

	FVoidHandler OnSyncPlatformPurchaseSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformPurchase::OnSyncPlatformPurchaseSuccess);
	FErrorHandler OnSyncPlatformPurchaseErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformPurchase::OnSyncPlatformPurchaseError);
	API_FULL_CHECK_GUARD(Entitlement, Error);
	Entitlement->SyncPlatformPurchase(EntitlementSyncBase, PlatformEnum, OnSyncPlatformPurchaseSuccessDelegate, OnSyncPlatformPurchaseErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformPurchase::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, Error);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformPurchase::OnSyncPlatformPurchaseSuccess()
{
	UE_LOG_AB(Log, TEXT("Successfully synced platform purchase"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteSyncPlatformPurchase::OnSyncPlatformPurchaseError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to sync platform purchase! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	Error = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

EAccelBytePlatformSync FOnlineAsyncTaskAccelByteSyncPlatformPurchase::GetNavitePlatformSyncType()
{
	TRY_PIN_SUBSYSTEM(EAccelBytePlatformSync::OTHER)

	FName PlatformName = SubsystemPin->GetNativePlatformName();
#ifdef STEAM_SUBSYSTEM
	if (PlatformName == STEAM_SUBSYSTEM)
	{
		return EAccelBytePlatformSync::STEAM;
	}
#endif
#ifdef EOS_SUBSYSTEM
	if (PlatformName == EOS_SUBSYSTEM)
	{
		return EAccelBytePlatformSync::EPIC_GAMES;
	}
#endif
#ifdef GDK_SUBSYSTEM
	if (PlatformName == GDK_SUBSYSTEM)
	{
		return EAccelBytePlatformSync::XBOX_LIVE;
	}
#endif
#ifdef PS4_SUBSYSTEM
	if (PlatformName == PS4_SUBSYSTEM)
	{
		if (EntitlementSyncBase.ServiceLabel == 0)
		{
			const FOnlineStoreV2AccelBytePtr StoreInt = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
			if (StoreInt.IsValid())
			{
				EntitlementSyncBase.ServiceLabel = StoreInt->GetServiceLabel();
			}
		}
		return EAccelBytePlatformSync::PLAYSTATION;
	}
#endif
#ifdef PS5_SUBSYSTEM
	if (PlatformName == PS5_SUBSYSTEM)
	{
		if (EntitlementSyncBase.ServiceLabel == 0)
		{
			const FOnlineStoreV2AccelBytePtr StoreInt = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
			if (StoreInt.IsValid())
			{
				EntitlementSyncBase.ServiceLabel = StoreInt->GetServiceLabel();
			}
		}
		return EAccelBytePlatformSync::PLAYSTATION;
	}
#endif
	return EAccelBytePlatformSync::OTHER;
}
