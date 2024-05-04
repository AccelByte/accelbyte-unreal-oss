// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetItemByCriteria.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineStoreV2SystemAccelByte"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetItemByCriteria::FOnlineAsyncTaskAccelByteGetItemByCriteria(
FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, FAccelByteModelsItemCriteria const& InItemCriteria, int32 const& InOffset, int32 const& InLimit,
	TArray<EAccelByteItemListSortBy> InSortBy, FString const& InStoreId, bool InAutoCalcEstimatedPrice)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, ItemCriteria(InItemCriteria)
	, Offset(InOffset)
	, Limit(InLimit)
	, SortBy(InSortBy)
	, StoreId(InStoreId)
	, AutoCalcEstimatedPrice(InAutoCalcEstimatedPrice)

{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));
	
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	ItemPagingSliced = FAccelByteModelsItemPagingSlicedResult{}; 
	ErrorCode = 0;
    ErrorMessage = TEXT("");
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteGetItemByCriteria::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	const THandler<FAccelByteModelsItemPagingSlicedResult>& OnSuccess = TDelegateUtils<THandler<FAccelByteModelsItemPagingSlicedResult>>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGetItemByCriteria::HandleSuccess);
	const FErrorHandler& OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGetItemByCriteria::HandleError);
	API_CLIENT_CHECK_GUARD(ErrorMessage);
	ApiClient->Item.GetItemsByCriteria(ItemCriteria, Offset, Limit, OnSuccess, OnError, SortBy, StoreId, AutoCalcEstimatedPrice);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetItemByCriteria::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	
	Super::Finalize();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetItemByCriteria::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	
	Super::TriggerDelegates();
	const FOnlineStoreV2AccelBytePtr StoreInterface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	if (StoreInterface.IsValid())
	{
		const FOnlineErrorAccelByte OnlineError = bWasSuccessful ? ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success) :
			ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorMessage));
		StoreInterface->TriggerOnGetItemsByCriteriaCompleteDelegates(bWasSuccessful, ItemPagingSliced, OnlineError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetItemByCriteria::HandleSuccess(const FAccelByteModelsItemPagingSlicedResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	ItemPagingSliced = Result; 
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetItemByCriteria::HandleError(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	ErrorCode = Code;
	ErrorMessage = Message; 
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE