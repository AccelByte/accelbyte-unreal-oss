// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryOfferBySku.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryOfferBySku::FOnlineAsyncTaskAccelByteQueryOfferBySku(FOnlineSubsystemAccelByte* const InABSubsystem,
	const FUniqueNetId& InUserId,
	const FString& InSku,
	const FOnQueryOnlineStoreOffersComplete& InDelegate,
	bool InAutoCalcEstimatedPrice) 
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, Sku(InSku)
	, Delegate(InDelegate)
	, Offer(MakeShared<FOnlineStoreOfferAccelByte>())
	, Language(InABSubsystem->GetLanguage())
	, AutoCalcEstimatedPrice(InAutoCalcEstimatedPrice)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	OnSuccess = TDelegateUtils<THandler<FAccelByteModelsItemInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleGetItemBySku);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleAsyncTaskError);
	API_FULL_CHECK_GUARD(Item, ErrorMsg);
	Item->GetItemBySku(Sku, Language, TEXT(""), OnSuccess, OnError, AutoCalcEstimatedPrice);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	Super::Finalize();
	
	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
	StoreV2Interface->EmplaceOffers(TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef>{TPair<FUniqueOfferId, FOnlineStoreOfferAccelByteRef>{Offer->OfferId, Offer}});
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates();

	TArray<FString> QueriedOfferIds{Offer->OfferId};
	Delegate.ExecuteIfBound(bWasSuccessful, QueriedOfferIds, ErrorMsg);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleGetItemBySku(const FAccelByteModelsItemInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Get Success"));
	Offer->SetItem(Result);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferBySku::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
	
	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}