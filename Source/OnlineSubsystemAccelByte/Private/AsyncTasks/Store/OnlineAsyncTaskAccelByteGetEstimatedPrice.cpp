// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetEstimatedPrice.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineStoreV2SystemAccelByte"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetEstimatedPrice::FOnlineAsyncTaskAccelByteGetEstimatedPrice(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const TArray<FString>& InItemIds, const FString& InRegion)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, ItemIds(InItemIds)
	, Region(InRegion)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));
	
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	EstimatedPrices = TArray<FAccelByteModelsEstimatedPrices>{}; 
	ErrorCode = 0;
    ErrorMessage = TEXT("");
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteGetEstimatedPrice::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	const THandler<TArray<FAccelByteModelsEstimatedPrices>>& OnSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsEstimatedPrices>>>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGetEstimatedPrice::HandleSuccess);
	const FErrorHandler& OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGetEstimatedPrice::HandleError);
	API_CLIENT_CHECK_GUARD(ErrorMessage);
	ApiClient->Item.GetEstimatedPrice(ItemIds, Region, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetEstimatedPrice::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	
	Super::Finalize();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetEstimatedPrice::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	
	Super::TriggerDelegates();
	const FOnlineStoreV2AccelBytePtr StoreInterface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
	if (StoreInterface.IsValid())
	{
		const FOnlineErrorAccelByte OnlineError = bWasSuccessful ? ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success) :
			ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorMessage));
		StoreInterface->TriggerOnGetEstimatedPriceCompleteDelegates(bWasSuccessful, EstimatedPrices, OnlineError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetEstimatedPrice::HandleSuccess(const TArray<FAccelByteModelsEstimatedPrices>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	EstimatedPrices = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetEstimatedPrice::HandleError(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	ErrorCode = Code;
	ErrorMessage = Message; 
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE