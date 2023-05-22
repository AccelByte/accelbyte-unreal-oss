// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateStats.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

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
			FOnlineStatUpdate::EOnlineStatModificationType UpdateStrategy = Stat.Value.GetModificationType();
			
			UpdateUserStatItemWithStatCode.StatCode = Key;
			UpdateUserStatItemWithStatCode.Value = FCString::Atof(*Value.ToString());
			UpdateUserStatItemWithStatCode.UpdateStrategy = FOnlineStatisticAccelByte::ConvertUpdateStrategy(UpdateStrategy);
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

void FOnlineAsyncTaskAccelByteUpdateStats::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));
	Super::Finalize();
	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(Subsystem->GetStatsInterface());
	if (StatisticInterface.IsValid())
	{
		for (const auto& UserStatsPair : OnlineUsersStatsPairs)
		{
			StatisticInterface->EmplaceStats(UserStatsPair);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStats::TriggerDelegates()
{
	Super::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)));
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStats::HandleBulkUpdateUserStatItemsValue(const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleBulkUpdateUserStatItemsValue Success"));
	BulkUpdateStatItemsResponse = Result;
	if (Result.Num() > 0)
	{
		for (int i = 0; i < Result.Num(); i++)
		{
			if (!Result[i].Success)
			{
				continue;
			}
			float NewValue = 0.0f;
			TSharedPtr<FJsonValue> NewValueJson = Result[i].Details.JsonObject.Get()->TryGetField("currentValue");
			if (NewValueJson.IsValid())
			{
				if (!NewValueJson->IsNull())
				{
					NewValue = NewValueJson->AsNumber();
				}
			}
			FString Key = Result[i].StatCode;
			FVariantData Value = NewValue;
			Stats.Add(Key, Value);

			OnlineUsersStatsPairs.Add(MakeShared<FOnlineStatsUserStats>(UserId.ToSharedRef(), Stats));
		}
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
 
void FOnlineAsyncTaskAccelByteUpdateStats::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE