// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteResetUserStats.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteResetUserStats::FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetIdRef InStatsUserId
	, const TSharedPtr<const FOnlineStatsUserStats> InUserStats
	, const FOnlineStatsUpdateStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, StatsUserId{FUniqueNetIdAccelByteUser::CastChecked(InStatsUserId)}
	, UserStats{InUserStats}
	, Delegate{InDelegate}
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteResetUserStats"));

	if (!IsRunningDedicatedServer() && StatsUserId->IsValid())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(StatsUserId);
	}
	else
	{
		LocalUserNum = 0;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

FOnlineAsyncTaskAccelByteResetUserStats::FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte* const InABInterface
	, const int32 InLocalUserNum
	, const FUniqueNetIdRef InStatsUserId
	, const TSharedPtr<const FOnlineStatsUserStats> InUserStats
	, const FOnlineStatsUpdateStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, StatsUserId{ FUniqueNetIdAccelByteUser::CastChecked(InStatsUserId) }
	, UserStats{ InUserStats }
	, Delegate{ InDelegate }
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	LocalUserNum = InLocalUserNum;
	if (!IsRunningDedicatedServer())
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (IdentityInterface.IsValid())
		{
			UserId = FUniqueNetIdAccelByteUser::CastChecked(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef());
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-reset-stats-user-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset user stats, identity interface is invalid!"));
		return;
	}

	ELoginStatus::Type LoginStatus;
	if (!IsRunningDedicatedServer())
	{
		if (UserId.Get()->IsValid())
		{
			LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
		}
		else
		{
			ErrorMessage = TEXT("request-failed-reset-stats-user-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset user stats, userId is invalid!"));
			return;
		}
	}
	else
	{
		LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	}

	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-reset-stats-user-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset user stats, user not logged in!"));
		return;
	}

	if (!UserStats.IsValid())
	{
		ErrorMessage = TEXT("request-failed-reset-stats-user-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset user stats, user stats are invalid!"));

		return;
	}

#if !UE_BUILD_SHIPPING
	OnBulkResetMultipleUserStatItemsValueSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteResetUserStats::OnResetUserStatItemsSuccess);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteResetUserStats::OnResetUserStatItemsFailed);

	FString AdditionalKey = TEXT("");	
	TArray<FAccelByteModelsResetUserStatItemValue> UserStatItemValues;
	for (auto UserStat : UserStats->Stats)
	{
		FAccelByteModelsResetUserStatItemValue UserStatItemValue{};
		UserStatItemValue.UserId = StatsUserId->GetAccelByteId();
		UserStatItemValue.StatCode = UserStat.Key;

		UserStatItemValues.Add(UserStatItemValue);
	}


	if (IsRunningDedicatedServer())
	{
		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		ServerApiClient->ServerStatistic.BulkResetMultipleUserStatItemsValue(UserStatItemValues, OnBulkResetMultipleUserStatItemsValueSuccess, OnError);
	}
	else
	{
		ApiClient->Statistic.BulkResetMultipleUserStatItemsValue(UserStatItemValues, OnBulkResetMultipleUserStatItemsValueSuccess, OnError);
	}
#else
	OnResetUserStatItemsFailed(static_cast<int32>(AccelByte::ErrorCodes::InvalidRequest), TEXT("Unable to do ResetStats in shipping build."));
#endif

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));
	Super::Finalize();

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(Subsystem->GetStatsInterface());
	if (StatisticInterface.IsValid())
	{
		if (OnlineUserStatsPair.IsValid())
		{
			StatisticInterface->EmplaceStats(OnlineUserStatsPair);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates();
	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(Subsystem->GetStatsInterface());
	StatisticInterface->TriggerOnUserStatItemsResetCompletedDelegates(LocalUserNum, bWasSuccessful, UserStatsResetResponse, ErrorMessage);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::OnResetUserStatItemsSuccess(const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("BulkResetUserStatItemsValue Success"));

	UserStatsResetResponse = Result;

	for (const auto& Stat : Result)
	{
		float NewValue = float(Stat.Details.JsonObject.Get()->GetIntegerField("currentValue"));

		FString Key = Stat.StatCode;
		FVariantData Value = NewValue;
		Stats.Add(Key, Value);
	}

	OnlineUserStatsPair = MakeShared<FOnlineStatsUserStats>(StatsUserId, Stats);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
 
void FOnlineAsyncTaskAccelByteResetUserStats::OnResetUserStatItemsFailed(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
#undef ONLINE_ERROR_NAMESPACE