// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteResetUserStats.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteResetUserStats::FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte *const InABInterface
	, FUniqueNetIdRef const InStatsUserId
	, TSharedPtr<const FOnlineStatsUserStats> const InUserStats
	, FOnlineStatsUpdateStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, StatsUserId{ FUniqueNetIdAccelByteUser::CastChecked(InStatsUserId) }
	, UserStats{ InUserStats }
	, Delegate{ InDelegate }
{
	if (!IsRunningDedicatedServer() && InStatsUserId->IsValid())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InStatsUserId);
	}
	else
	{
		LocalUserNum = 0;
	}
}

FOnlineAsyncTaskAccelByteResetUserStats::FOnlineAsyncTaskAccelByteResetUserStats(FOnlineSubsystemAccelByte *const InABInterface
	, int32 InLocalUserNum
	, FUniqueNetIdRef const InStatsUserId
	, TSharedPtr<const FOnlineStatsUserStats> const InUserStats
	, FOnlineStatsUpdateStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, StatsUserId{ FUniqueNetIdAccelByteUser::CastChecked(InStatsUserId) }
	, UserStats{ InUserStats }
	, Delegate{ InDelegate }
{
}

void FOnlineAsyncTaskAccelByteResetUserStats::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-reset-stats-user-error-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset user stats, Identity Interface is invalid!"));
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
			ErrorMessage = TEXT("request-failed-reset-stats-user-error-user-invalid");
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
		ErrorMessage = TEXT("request-failed-reset-stats-user-error-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset user stats, not logged in!"));
		return;
	}

	if (!UserStats.IsValid())
	{
		ErrorMessage = TEXT("request-failed-reset-stats-user-error-user-stats-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset user stats, user stats are invalid!"));
		return;
	}

#if !UE_BUILD_SHIPPING
	OnBulkResetMultipleUserStatItemsValueSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteResetUserStats::OnResetUserStatItemsSuccess);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteResetUserStats::OnResetUserStatItemsFailed);

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
		ServerApiClient->ServerStatistic.BulkResetMultipleUserStatItemsValue(UserStatItemValues
			, OnBulkResetMultipleUserStatItemsValueSuccess
			, OnError);
	}
	else
	{
		ApiClient->Statistic.BulkResetMultipleUserStatItemsValue(UserStatItemValues
			, OnBulkResetMultipleUserStatItemsValueSuccess
			, OnError);
	}
#else
	OnResetUserStatItemsFailed(static_cast<int32>(AccelByte::ErrorCodes::InvalidRequest)
		, TEXT("Unable to do ResetStats in shipping build."));
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

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		TSharedPtr<FAccelByteModelsUserStatItemResetPayload> UserStatItemResetPayload = MakeShared<FAccelByteModelsUserStatItemResetPayload>();
		UserStatItemResetPayload->UserId = StatsUserId->GetAccelByteId();
		for (const auto& StatItem : UserStatsResetResponse)
		{
			if (StatItem.Success)
			{
				UserStatItemResetPayload->StatCodes.Add(StatItem.StatCode);
			}
		}

		PredefinedEventInterface->SendEvent(LocalUserNum, UserStatItemResetPayload.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates();
	EOnlineErrorResult Result = bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure;

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(Subsystem->GetStatsInterface());
	StatisticInterface->TriggerOnUserStatItemsResetCompletedDelegates(LocalUserNum
		, bWasSuccessful
		, UserStatsResetResponse
		, ErrorMessage);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteResetUserStats::OnResetUserStatItemsSuccess(const TArray<FAccelByteModelsUpdateUserStatItemsResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("BulkResetUserStatItemsValue Success"));

	UserStatsResetResponse = Result;

	for (const auto& Stat : UserStatsResetResponse)
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
