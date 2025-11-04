// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetWalletInfoV2.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineWalletV2InterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetWalletInfoV2::FOnlineAsyncTaskAccelByteGetWalletInfoV2(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InCurrencyCode, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, CurrencyCode(InCurrencyCode)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGetWalletInfoV2::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting wallet info, UserId: %s"), *UserId->ToDebugString());

	const FOnlineWalletV2AccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletV2AccelByte>(SubsystemPin->GetWalletV2Interface());
	if (WalletInterface.IsValid())
	{
		FAccelByteModelsWalletInfoResponse WalletInfo;
		if (!WalletInterface->GetWalletInfoFromCacheV2(LocalUserNum, CurrencyCode, WalletInfo) || bAlwaysRequestToService)
		{	
			// Create delegates for successfully as well as unsuccessfully requesting to get wallet info
			OnGetWalletInfoSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsWalletInfoResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetWalletInfoV2::OnGetWalletInfoSuccess);
			OnGetWalletInfoErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetWalletInfoV2::OnGetWalletInfoError);

			// Send off a request to get wallet info, as well as connect our delegates for doing so
			API_FULL_CHECK_GUARD(Wallet, ErrorStr);
			Wallet->GetWalletInfoByCurrencyCodeV2(CurrencyCode, OnGetWalletInfoSuccessDelegate, OnGetWalletInfoErrorDelegate);
		}
		else
		{
			CachedWalletInfo = WalletInfo;
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
	}
	else
	{
		ErrorStr = TEXT("request-failed-wallet-interface-invalid");
		UE_LOG_AB(Warning, TEXT("Failed to get wallet info! wallet interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetWalletInfoV2::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineWalletV2AccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletV2AccelByte>(SubsystemPin->GetWalletV2Interface());
	if (WalletInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			WalletInterface->TriggerOnGetWalletInfoV2CompletedDelegates(LocalUserNum, bWasSuccessful, CachedWalletInfo, TEXT(""));
		}
		else
		{
			WalletInterface->TriggerOnGetWalletInfoV2CompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfoResponse{}, ErrorStr);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetWalletInfoV2::OnGetWalletInfoSuccess(const FAccelByteModelsWalletInfoResponse& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineWalletV2AccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletV2AccelByte>(SubsystemPin->GetWalletV2Interface());
	if (WalletInterface.IsValid())
	{
		WalletInterface->AddWalletInfoToListV2(LocalUserNum, CurrencyCode, MakeShared<FAccelByteModelsWalletInfoResponse>(Result));
		CachedWalletInfo = Result;
	}
	else

	{
		ErrorStr = TEXT("request-failed-wallet-interface-invalid");
		UE_LOG_AB(Warning, TEXT("Failed to get wallet info! wallet interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to get wallet info for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteGetWalletInfoV2::OnGetWalletInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-get-wallet-info-error");
	UE_LOG_AB(Warning, TEXT("Failed to get wallet info! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}