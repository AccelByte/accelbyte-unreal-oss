// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteStatsUsers.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteDeleteStatsUsers::FOnlineAsyncTaskAccelByteDeleteStatsUsers(FOnlineSubsystemAccelByte *const InABInterface
	, int32 InLocalUserNum
	, FUniqueNetIdRef const& InStatsUser
	, FString const& InStatCode
	, FString const& InAdditionalKey)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, StatCode(InStatCode)
	, AdditionalKey(InAdditionalKey)
{
	StatsUser = FUniqueNetIdAccelByteUser::CastChecked(InStatsUser);
}

void FOnlineAsyncTaskAccelByteDeleteStatsUsers::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	
	if (!IsRunningDedicatedServer())
	{
		ErrorMessage = TEXT("request-failed-delete-user-stats-error-not-ds");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user stats, not running as Dedicated Server!"));
		return;
	}
	
	if (!StatsUser.IsValid() || !StatsUser->IsValid())
	{
		ErrorMessage = TEXT("request-failed-delete-user-stats-error-user-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user stats, selected user is invalid!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-delete-user-stats-error-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user stats, Identity Interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-delete-user-stats-error-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete user stats, not logged in!"));
		return;
	}

	OnDeleteUserStatItemsValueSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteDeleteStatsUsers::OnDeleteUserStatsSuccess);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteDeleteStatsUsers::OnDeleteUserStatsFailed);
	
	FString AccelByteUserId = StatsUser->GetAccelByteId();
	SERVER_API_CLIENT_CHECK_GUARD();
	ServerApiClient->ServerStatistic.DeleteUserStatItems(AccelByteUserId
		, StatCode
		, AdditionalKey
		, OnDeleteUserStatItemsValueSuccess
		, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteStatsUsers::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));
	Super::Finalize();

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && StatisticInterface.IsValid())
	{
		FAccelByteModelsUserStatItemDeletedPayload UserStatItemDeletedPayload{};
		UserStatItemDeletedPayload.UserId = StatsUser->GetAccelByteId();
		for (const auto& UserStatsPair : OnlineUsersStatsPairs)
		{
			for (const auto& UserStats : UserStatsPair->Stats)
			{
				StatisticInterface->RemoveStats(UserStatsPair->Account
					, UserStats.Key);
				UserStatItemDeletedPayload.StatCodes.Add(UserStats.Key);
			}
		}
		if (PredefinedEventInterface.IsValid())
		{
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsUserStatItemDeletedPayload>(UserStatItemDeletedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteStatsUsers::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s")
		, LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
	if (!ensure(StatisticInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for delete user statistic as our statistic interface is invalid!"));
		return;
	}

	EOnlineErrorResult Result = bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure;

	StatisticInterface->TriggerOnUserStatItemsDeleteCompletedDelegates(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteStatsUsers::OnDeleteUserStatsSuccess()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Delete User Stats Success"));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
	TSharedPtr<const FOnlineStatsUserStats> CurrentUserStats = StatisticInterface->GetStats(StatsUser.ToSharedRef());

	for (const auto& UserStat : CurrentUserStats->Stats)
	{
		if (UserStat.Key == StatCode)
		{
			FString Key = UserStat.Key;
			FVariantData Value = UserStat.Value;
			Stats.Add(Key, Value);
		}
	}

	OnlineUsersStatsPairs.Add(MakeShared<FOnlineStatsUserStats>(StatsUser.ToSharedRef(), Stats));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteStatsUsers::OnDeleteUserStatsFailed(int32 Code
	, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s")
		, Code
		, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE