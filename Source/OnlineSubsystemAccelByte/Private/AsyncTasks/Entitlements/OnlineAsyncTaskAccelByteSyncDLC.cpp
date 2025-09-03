// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncDLC.h"
#include <OnlineSubsystemAccelByte.h>
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineStoreInterfaceV2AccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSyncDLC::FOnlineAsyncTaskAccelByteSyncDLC(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Error(TEXT(""))
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteSyncDLC::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FVoidHandler OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncDLC::OnSyncDLCSuccess);
	const FErrorHandler OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncDLC::OnSyncDLCFailed);

	const FName NativeSubsystemName = SubsystemPin->GetNativePlatformName();

	// Rely to the native subsystem name as a way to differentiate which platform is it
	EAccelByteLoginType LoginType = FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(NativeSubsystemName);

	API_FULL_CHECK_GUARD(Entitlement, Error);

	switch (LoginType)
	{
		case EAccelByteLoginType::Steam:
		{
			Entitlement->SyncSteamDLC(OnSuccessDelegate, OnErrorDelegate);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}

		case EAccelByteLoginType::Xbox:
		{
			FAccelByteModelsXBoxDLCSync XboxDLCSync;

			IOnlineSubsystem* PlatformSubsystem = SubsystemPin->GetNativePlatformSubsystem();
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
			FUniqueNetIdPtr PlatformUniqueId = UserId->GetPlatformUniqueId();
			if (!PlatformUniqueId.IsValid())
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
				return;
			}

			TSharedPtr<FUserOnlineAccount> UserAccount = PlatformIdentityInt->GetUserAccount(PlatformUniqueId.ToSharedRef().Get());
			XboxDLCSync.XstsToken = UserAccount->GetAccessToken();

			Entitlement->SyncXBoxDLC(XboxDLCSync, OnSuccessDelegate, OnErrorDelegate);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}

		case EAccelByteLoginType::PS4:
		case EAccelByteLoginType::PS4CrossGen:
		{
			FAccelByteModelsPlayStationDLCSync PSSyncModel;

			int32 ServiceLabel = 0;
			const FOnlineStoreV2AccelBytePtr StoreInt = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
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

			Entitlement->SyncPSNDLC(PSSyncModel, OnSuccessDelegate, OnErrorDelegate);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}

		case EAccelByteLoginType::PS5:
		{
			FAccelByteModelsPlayStationDLCSync PSSyncModel;

			int32 ServiceLabel = 0;
			const FOnlineStoreV2AccelBytePtr StoreInt = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
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

			Entitlement->SyncPSNDLC(PSSyncModel, OnSuccessDelegate, OnErrorDelegate);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}

		case EAccelByteLoginType::EOS:
		{
			FString EpicGamesJwtToken = TEXT("");
			IOnlineSubsystem* PlatformSubsystem = SubsystemPin->GetNativePlatformSubsystem();
			if (PlatformSubsystem == nullptr)
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
				return;
			}

			IOnlineIdentityPtr IdentityInterface = PlatformSubsystem->GetIdentityInterface();
			if (!IdentityInterface.IsValid())
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
				return;
			}

			// Get user's platform user id 
			FUniqueNetIdPtr UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (!UserIdPtr.IsValid())
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
				return;
			}

			TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(UserIdPtr.ToSharedRef().Get());
			EpicGamesJwtToken = UserAccount->GetAccessToken();

			Entitlement->SyncEpicGameDurableItems(EpicGamesJwtToken, OnSuccessDelegate, OnErrorDelegate);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}
		
		default:
		{
			Error = TEXT("Sync DLC failed due to inability to identify the current native platform running in the game.");
			UE_LOG_AB(Warning, TEXT("%s"), *Error);
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			return;
		}
	}
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
