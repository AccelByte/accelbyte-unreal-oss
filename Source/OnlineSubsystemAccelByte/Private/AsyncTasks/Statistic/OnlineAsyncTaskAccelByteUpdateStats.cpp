// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateStats.h"

FOnlineAsyncTaskAccelByteUpdateStats::FOnlineAsyncTaskAccelByteUpdateStats(FOnlineSubsystemAccelByte *const InABInterface
	, FUniqueNetIdRef const InLocalUserId
	, TArray<FOnlineStatsUserUpdatedStats> const& InUpdatedUserStats
	, FOnlineStatsUpdateStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, UpdatedUserStats(InUpdatedUserStats) 
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId); 
	
	for (auto const& UpdatedUserStat : UpdatedUserStats)
	{
		FAccelByteModelsUpdateUserStatItemWithStatCode UpdateUserStatItemWithStatCode;
		TSharedPtr<const FUniqueNetIdAccelByteUser> Account = FUniqueNetIdAccelByteUser::CastChecked(UpdatedUserStat.Account);
		for (auto const& Stat : UpdatedUserStat.Stats)
		{
			FString Key = Stat.Key;
			FOnlineStatUpdate const& Value = Stat.Value;			
			UpdateUserStatItemWithStatCode.StatCode = Key;
			UpdateUserStatItemWithStatCode.Value = FCString::Atof(*Value.ToString()); 
		}
		BulkUpdateUserStatItems.Add(UpdateUserStatItemWithStatCode);
	}
}

void FOnlineAsyncTaskAccelByteUpdateStats::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));

	OnBulkUpdateUserStatItemsValueSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteUpdateStats::HandleBulkUpdateUserStatItemsValue);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteUpdateStats::HandleAsyncTaskError);

	FString AdditionalKey = TEXT("");	
	ApiClient->Statistic.BulkUpdateUserStatItemsValue(AdditionalKey
		, BulkUpdateUserStatItems
		, OnBulkUpdateUserStatItemsValueSuccess
		, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStats::TriggerDelegates()
{
	Super::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Delegate.ExecuteIfBound(OnlineError);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStats::HandleBulkUpdateUserStatItemsValue(const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("BulkUpdateUserStatItemsValue Success")); 
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
 
void FOnlineAsyncTaskAccelByteUpdateStats::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	OnlineError.SetFromErrorCode(Code);
	OnlineError.SetFromErrorMessage(FText::FromString(ErrMsg));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}