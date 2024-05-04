// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryOfferDynamicData.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryOfferDynamicData::FOnlineAsyncTaskAccelByteQueryOfferDynamicData(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FUniqueOfferId& InOfferId,
	const FOnQueryOnlineStoreOffersComplete& InDelegate) 
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, OfferId(InOfferId)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryOfferDynamicData::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	OnSuccess = TDelegateUtils<THandler<FAccelByteModelsItemDynamicData>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferDynamicData::HandleGetItemDynamicData);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryOfferDynamicData::HandleAsyncTaskError);
	API_CLIENT_CHECK_GUARD(ErrorMsg);
	ApiClient->Item.GetItemDynamicData(OfferId, OnSuccess, OnError);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferDynamicData::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	Super::Finalize();
	
	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	StoreV2Interface->EmplaceOfferDynamicData(*UserId.Get(), MakeShared<FAccelByteModelsItemDynamicData>(DynamicData));
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferDynamicData::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates();

	TArray<FString> QueriedOfferIds{DynamicData.ItemId};
	Delegate.ExecuteIfBound(bWasSuccessful, QueriedOfferIds, ErrorMsg);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferDynamicData::HandleGetItemDynamicData(const FAccelByteModelsItemDynamicData& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Get Success"));

	DynamicData = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryOfferDynamicData::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
	
	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}