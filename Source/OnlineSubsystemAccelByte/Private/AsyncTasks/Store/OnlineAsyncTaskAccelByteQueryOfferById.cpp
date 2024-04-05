// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryOfferById.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryOfferById::FOnlineAsyncTaskAccelByteQueryOfferById(FOnlineSubsystemAccelByte* const InABSubsystem,
	const FUniqueNetId& InUserId,
	const TArray<FUniqueOfferId>& InOfferIds,
	const FOnQueryOnlineStoreOffersComplete& InDelegate,
	const FString& InStoreId,
	bool InAutoCalcEstimatedPrice)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, OfferIds(InOfferIds)
	, Language(InABSubsystem->GetLanguage())
	, Delegate(InDelegate)
	, StoreId(InStoreId)
	, AutoCalcEstimatedPrice(InAutoCalcEstimatedPrice)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryOfferById::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	FOnlineAsyncTaskAccelByte::Initialize();
	
	OnSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsItemInfo>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferById::HandleGetItemByIds);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferById::HandleAsyncTaskError);
	ApiClient->Item.BulkGetLocaleItems(OfferIds, TEXT(""), Language, OnSuccess, OnError, StoreId, AutoCalcEstimatedPrice);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	FOnlineAsyncTaskAccelByte::Finalize();
	
	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	StoreV2Interface->EmplaceOffers(OfferMap);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	TArray<FString> QueriedOfferIds;
	OfferMap.GenerateKeyArray(QueriedOfferIds);
	Delegate.ExecuteIfBound(bWasSuccessful, QueriedOfferIds, ErrorMsg);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::HandleGetItemByIds(const TArray<FAccelByteModelsItemInfo>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Get Success"));
	for (const auto& Item : Result)
	{
		FOnlineStoreOfferAccelByteRef Offer = MakeShared<FOnlineStoreOfferAccelByte>();
		Offer->SetItem(Item);
		OfferMap.Add(Offer->OfferId, Offer);
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferById::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
	
	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}