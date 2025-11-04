// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncPlatformPurchase.h"

#include "OnlineStoreInterfaceV2AccelByte.h"

#include "Api/AccelByteEntitlementApi.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

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
	EAccelBytePlatformSync PlatformEnum = GetNativePlatformSyncType();

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

EAccelBytePlatformSync FOnlineAsyncTaskAccelByteSyncPlatformPurchase::GetNativePlatformSyncType()
{
	TRY_PIN_SUBSYSTEM(EAccelBytePlatformSync::OTHER)

	FName PlatformName = SubsystemPin->GetNativePlatformName();

	// Rely to the native subsystem name as a way to differentiate which platform is it
	EAccelByteLoginType LoginType = FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(PlatformName);

	switch (LoginType)
	{
		case EAccelByteLoginType::Steam:
			return EAccelBytePlatformSync::STEAM;

		case EAccelByteLoginType::EOS:
			return EAccelBytePlatformSync::EPIC_GAMES;

		case EAccelByteLoginType::Xbox:
			return EAccelBytePlatformSync::XBOX_LIVE;
		
		case EAccelByteLoginType::PS4:
		case EAccelByteLoginType::PS4CrossGen:
		case EAccelByteLoginType::PS5:
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

		default:
			return EAccelBytePlatformSync::OTHER;
	}
}
