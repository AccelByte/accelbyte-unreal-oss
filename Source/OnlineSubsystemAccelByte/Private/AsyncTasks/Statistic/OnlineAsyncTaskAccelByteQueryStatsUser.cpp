// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryStatsUser.h"

FOnlineAsyncTaskAccelByteQueryStatsUser::FOnlineAsyncTaskAccelByteQueryStatsUser(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetId> InLocalUserId,
	const TSharedRef<const FUniqueNetId> InStatsUser, const TArray<FString>& InStatNames, const FOnlineStatsQueryUserStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalUserId(InLocalUserId)
	, StatsUser(InStatsUser) 
	, StatNames(InStatNames)
	, Delegate(InDelegate) 
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteQueryStatsUser")); 
	
	UserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	FUniqueNetIdAccelByteUserRef NetId = FUniqueNetIdAccelByteUser::CastChecked(StatsUser);
	AccelByteUserId = NetId->GetAccelByteId();
	Count = 0;
	StatCodes = {};

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize(); 

	THandler<FAccelByteModelsUserStatItemPagingSlicedResult> OnGetUserStatItemsSuccess =
		TDelegateUtils<THandler<FAccelByteModelsUserStatItemPagingSlicedResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStatsUser::HandleGetUserStatItems);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStatsUser::HandleAsyncTaskError);
	ApiClient->Statistic.GetUserStatItems(AccelByteUserId, StatNames, {}, OnGetUserStatItemsSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteQueryStatsUser::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates(); 
	Delegate.ExecuteIfBound(OnlineError, OnlineUserStatsPair);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::HandleGetUserStatItems(const FAccelByteModelsUserStatItemPagingSlicedResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GetUserStatItems Success")); 

	for (auto value : Result.Data)
	{
		StatCodes.Add(value.StatCode);
	}	
	
	OnBulkFetchStatItemsValueSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsStatItemValueResponse>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStatsUser::HandleBulkFetchStatItemsValue);
	ApiClient->Statistic.BulkFetchStatItemsValue(StatCodes[Count], { AccelByteUserId }, OnBulkFetchStatItemsValueSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::HandleBulkFetchStatItemsValue(const TArray<FAccelByteModelsStatItemValueResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("BulkFetchStatItemsValue Success"));

	for (FAccelByteModelsStatItemValueResponse User : Result)
	{
		FString Key = User.StatCode;
		FVariantData Value = User.Value;
		Stats.Add(Key, Value);
	}
	Count++;

	if (Count < StatCodes.Num())
	{ 
		ApiClient->Statistic.BulkFetchStatItemsValue(StatCodes[Count], { AccelByteUserId }, OnBulkFetchStatItemsValueSuccess, OnError);
	}
	else
	{
		OnlineUserStatsPair = MakeShareable(new FOnlineStatsUserStats(StatsUser, Stats));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
 
	OnlineError.SetFromErrorCode(Code);
	OnlineError.SetFromErrorMessage(FText::FromString(ErrMsg));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}