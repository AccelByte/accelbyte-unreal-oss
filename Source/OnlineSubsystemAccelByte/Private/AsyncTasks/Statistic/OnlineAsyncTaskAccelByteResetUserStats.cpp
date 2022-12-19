// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteResetUserStats.h"

FOnlineAsyncTaskAccelByteResetUserStats::FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetIdRef InStatsUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, StatsUserId(InStatsUserId) 
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteResetUserStats"));

	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(StatsUserId); 
	
	for (auto const& UpdatedUserStat : UpdatedUserStats)
	{
		FAccelByteModelsUpdateUserStatItemWithStatCode UpdateUserStatItemWithStatCode;
		TSharedPtr<const FUniqueNetIdAccelByteUser> Account = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UpdatedUserStat.Account);
		for (auto const& Stat : UpdatedUserStat.Stats)
		{
			FString Key = Stat.Key;
			FOnlineStatUpdate const& Value = Stat.Value;			
			UpdateUserStatItemWithStatCode.StatCode = Key;
			UpdateUserStatItemWithStatCode.Value = FCString::Atof(*Value.ToString()); 
		}
		BulkUpdateUserStatItems.Add(UpdateUserStatItemWithStatCode);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();

	OnBulkResetMultipleUserStatItemsValueSuccess = THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>>::CreateRaw(this, &FOnlineAsyncTaskAccelByteResetUserStats::HandleBulkUpdateUserStatItemsValue);
	OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteResetUserStats::HandleAsyncTaskError);

	FString AdditionalKey = TEXT("");	
	FAccelByteModelsResetUserStatItemValue UserStatItemValue{};
	UserStatItemValue.UserId = UserId->ToString();
	TArray<FAccelByteModelsResetUserStatItemValue> UserStatItemValues = { UserStatItemValue };
	ApiClient->Statistic.BulkResetMultipleUserStatItemsValue(UserStatItemValues, OnBulkResetMultipleUserStatItemsValueSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates();
	Delegate.ExecuteIfBound(OnlineError);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::HandleBulkUpdateUserStatItemsValue(const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("BulkUpdateUserStatItemsValue Success")); 
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
 
void FOnlineAsyncTaskAccelByteResetUserStats::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	OnlineError.SetFromErrorCode(Code);
	OnlineError.SetFromErrorMessage(FText::FromString(ErrMsg));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}