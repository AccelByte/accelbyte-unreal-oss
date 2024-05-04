// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetWalletInfo.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineWalletInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetWalletInfo::FOnlineAsyncTaskAccelByteGetWalletInfo(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InCurrencyCode, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, CurrencyCode(InCurrencyCode)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGetWalletInfo::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Getting wallet info, UserId: %s"), *UserId->ToDebugString());

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		FAccelByteModelsWalletInfo WalletInfo;
		if (!WalletInterface->GetWalletInfoFromCache(LocalUserNum, CurrencyCode, WalletInfo) || bAlwaysRequestToService)
		{	
			// Create delegates for successfully as well as unsuccessfully requesting to get wallet info
			OnGetWalletInfoSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsWalletInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetWalletInfo::OnGetWalletInfoSuccess);
			OnGetWalletInfoErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetWalletInfo::OnGetWalletInfoError);

			// Send off a request to get wallet info, as well as connect our delegates for doing so
			API_CLIENT_CHECK_GUARD(ErrorStr);
			ApiClient->Wallet.GetWalletInfoByCurrencyCode(CurrencyCode, OnGetWalletInfoSuccessDelegate, OnGetWalletInfoErrorDelegate);
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

void FOnlineAsyncTaskAccelByteGetWalletInfo::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			WalletInterface->TriggerOnGetWalletInfoCompletedDelegates(LocalUserNum, bWasSuccessful, CachedWalletInfo, TEXT(""));
		}
		else
		{
			WalletInterface->TriggerOnGetWalletInfoCompletedDelegates(LocalUserNum, false, FAccelByteModelsWalletInfo{}, ErrorStr);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetWalletInfo::OnGetWalletInfoSuccess(const FAccelByteModelsWalletInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineWalletAccelBytePtr WalletInterface = StaticCastSharedPtr<FOnlineWalletAccelByte>(Subsystem->GetWalletInterface());
	if (WalletInterface.IsValid())
	{
		WalletInterface->AddWalletInfoToList(LocalUserNum, CurrencyCode, MakeShared<FAccelByteModelsWalletInfo>(Result));
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

void FOnlineAsyncTaskAccelByteGetWalletInfo::OnGetWalletInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-get-wallet-info-error");
	UE_LOG_AB(Warning, TEXT("Failed to get wallet info! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}