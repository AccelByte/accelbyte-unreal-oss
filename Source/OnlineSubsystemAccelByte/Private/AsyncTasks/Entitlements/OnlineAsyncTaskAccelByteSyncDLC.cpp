// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncDLC.h"
#include <OnlineSubsystemAccelByte.h>
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineStoreInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteSyncDLC::FOnlineAsyncTaskAccelByteSyncDLC(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Error(TEXT(""))
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteSyncDLC::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FVoidHandler OnSuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteSyncDLC::OnSyncDLCSuccess);
	const FErrorHandler OnErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteSyncDLC::OnSyncDLCFailed);

	const FName NativeSubsystemName = Subsystem->GetNativePlatformName();

	// Use the respective API sync depending on the platform user is on
#ifdef STEAM_SUBSYSTEM
	if (NativeSubsystemName == STEAM_SUBSYSTEM)
	{
		ApiClient->Entitlement.SyncSteamDLC(OnSuccessDelegate, OnErrorDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}
#endif
#ifdef GDK_SUBSYSTEM
	if (NativeSubsystemName == GDK_SUBSYSTEM)
	{
		FAccelByteModelsXBoxDLCSync XboxDLCSync;

		IOnlineSubsystem* PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
		if (PlatformSubsystem == nullptr)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}

		IOnlineIdentityPtr PlatformIdentityInt = PlatformSubsystem->GetIdentityInterface();
		if (!PlatformIdentityInt.IsValid())
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}

		// Get user's platform user id 
		TSharedPtr<const FUniqueNetId> PlatformUniqueId = UserId->GetPlatformUniqueId();
		if (!PlatformUniqueId.IsValid())
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}

		TSharedPtr<FUserOnlineAccount> UserAccount = PlatformIdentityInt->GetUserAccount(PlatformUniqueId.ToSharedRef().Get());
		XboxDLCSync.XstsToken = UserAccount->GetAccessToken();

		ApiClient->Entitlement.SyncXBoxDLC(XboxDLCSync, OnSuccessDelegate, OnErrorDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}
#endif
#ifdef PS4_SUBSYSTEM
	if (NativeSubsystemName == PS4_SUBSYSTEM)
	{
		FAccelByteModelsPlayStationDLCSync PSSyncModel;

		int32 ServiceLabel = 0;
		const FOnlineStoreV2AccelBytePtr StoreInt = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
		if (StoreInt.IsValid())
		{
			ServiceLabel = StoreInt->GetServiceLabel();
		}

		if (ServiceLabel == 0)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}

		PSSyncModel.ServiceLabel = ServiceLabel;

		ApiClient->Entitlement.SyncPSNDLC(PSSyncModel, OnSuccessDelegate, OnErrorDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}
#endif
#ifdef PS5_SUBSYSTEM
	if (NativeSubsystemName == PS5_SUBSYSTEM)
	{
		FAccelByteModelsPlayStationDLCSync PSSyncModel;

		int32 ServiceLabel = 0;
		const FOnlineStoreV2AccelBytePtr StoreInt = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
		if (StoreInt.IsValid());
		{
			ServiceLabel = StoreInt->GetServiceLabel();
		}

		if (ServiceLabel == 0)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}

		PSSyncModel.ServiceLabel = ServiceLabel;

		ApiClient->Entitlement.SyncPSNDLC(PSSyncModel, OnSuccessDelegate, OnErrorDelegate);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}
#endif
}

void FOnlineAsyncTaskAccelByteSyncDLC::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, Error);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncDLC::OnSyncDLCSuccess()
{
	UE_LOG_AB(Log, TEXT("Successfully synced DLC on platform"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteSyncDLC::OnSyncDLCFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Sync DLC on platform failed"));
	Error = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
