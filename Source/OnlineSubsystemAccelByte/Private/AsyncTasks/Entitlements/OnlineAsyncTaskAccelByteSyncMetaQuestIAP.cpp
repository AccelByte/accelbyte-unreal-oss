// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncMetaQuestIAP.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSyncMetaQuestIAP"

FOnlineAsyncTaskAccelByteSyncMetaQuestIAP::FOnlineAsyncTaskAccelByteSyncMetaQuestIAP(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, const FOnRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	OnlineError = FOnlineError();
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestIAP::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	API_FULL_CHECK_GUARD(Entitlement, OnlineError);

	const THandler<TArray<FAccelByteModelsSyncOculusConsumableEntitlementInfo>> OnSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsSyncOculusConsumableEntitlementInfo>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncMetaQuestIAP::OnSyncOculusConsumableEntitlementsSuccess);
	const FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncMetaQuestIAP::OnSyncOculusConsumableEntitlementsFailed);
	Entitlement->SyncOculusConsumableEntitlements(OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestIAP::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	TRY_PIN_SUBSYSTEM();
	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());
	if (EntitlementInterface.IsValid())
	{
		EntitlementInterface->TriggerOnSyncMetaQuestIAPCompleteDelegates(LocalUserNum, *UserId, EntitlementInfos, OnlineError);
	}

	bool bDelegateTriggered = Delegate.ExecuteIfBound(bWasSuccessful, OnlineError.ErrorMessage.ToString());

	if (!bDelegateTriggered)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate as it is unbound."));
		return;
	}

	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestIAP::OnSyncOculusConsumableEntitlementsSuccess(const TArray<FAccelByteModelsSyncOculusConsumableEntitlementInfo>& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	for(const auto& Result : Response)
	{
		FPurchaseReceipt Receipt;
		FPurchaseReceipt::FReceiptOfferEntry ReceiptOfferEntry;
		ReceiptOfferEntry.OfferId = Result.OculusItemSku;
		FPurchaseReceipt::FLineItemInfo ItemInfo;
		ItemInfo.ItemName = Result.ItemIdentity;
		ItemInfo.ValidationInfo = FAccelByteUtilities::GetUEnumValueAsString(Result.ItemIdentityType);
		ReceiptOfferEntry.LineItems.Add(ItemInfo);

		Receipt.ReceiptOffers.Add(ReceiptOfferEntry);
		Receipt.TransactionId = Result.TransactionId;
		switch (Result.IAPOrderStatus)
		{
		case EAccelByteEntitlementIAPOrderStatus::FULFILLED:
			Receipt.TransactionState = EPurchaseTransactionState::Purchased;
			break;
		case EAccelByteEntitlementIAPOrderStatus::VERIFIED:
			Receipt.TransactionState = EPurchaseTransactionState::Processing;
			break;
		case EAccelByteEntitlementIAPOrderStatus::FAILED:
			Receipt.TransactionState = EPurchaseTransactionState::Failed;
			break;
		case EAccelByteEntitlementIAPOrderStatus::REVOKED:
			Receipt.TransactionState = EPurchaseTransactionState::Canceled;
		default:
			Receipt.TransactionState = EPurchaseTransactionState::Invalid;
			break;
		}

		EntitlementInfos.Add(MakeShared<FPurchaseReceipt>(Receipt));
	}
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncMetaQuestIAP::OnSyncOculusConsumableEntitlementsFailed(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	ErrorText = FText::FromString(TEXT("sync-oculus-iap-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to sync oculus consumable entitlements, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
