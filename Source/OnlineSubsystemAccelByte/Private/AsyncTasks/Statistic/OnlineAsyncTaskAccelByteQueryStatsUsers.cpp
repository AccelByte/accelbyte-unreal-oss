// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryStatsUsers.h"
#include "OnlineError.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteQueryStatsUsers::FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetId> InLocalUserId,
	const TArray<FUniqueNetIdRef>& InStatsUsers, const TArray<FString>& InStatNames, const FOnlineStatsQueryUsersStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, StatsUsers(InStatsUsers)
	, StatNames(InStatNames)
	, Delegate(InDelegate)
	, CountUsers(InStatsUsers.Num())
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteQueryStatsUsers"));

	if (!IsRunningDedicatedServer() && InLocalUserId->IsValid())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}

	if (CountUsers < 0)
	{
		CountUsers = 0;
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

FOnlineAsyncTaskAccelByteQueryStatsUsers::FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum, 
	const TArray<FUniqueNetIdRef>& InStatsUsers, const TArray<FString>& InStatNames, const FOnlineStatsQueryUsersStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalUserNum(InLocalUserNum)
	, StatsUsers(InStatsUsers)
	, StatNames(InStatNames)
	, Delegate(InDelegate)
	, CountUsers(InStatsUsers.Num())
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (IdentityInterface.IsValid())
		{
			UserId = FUniqueNetIdAccelByteUser::CastChecked(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef());
		}
	}

	if (CountUsers < 0)
	{
		CountUsers = 0;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Initialize(); 

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-query-stats-user-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user stats, identity interface is invalid!"));
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
			ErrorMessage = TEXT("request-failed-query-stats-user-error");
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user stats, userId is invalid!"));
			return;
		}
	}
	else
	{
		LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	}
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-query-stats-user-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user stats, user not logged in!"));
		return;
	}

	for (const auto& StatsUser : StatsUsers)
	{
		const FUniqueNetIdAccelByteUserRef ABStatsUser = FUniqueNetIdAccelByteUser::CastChecked(StatsUser);

		OnGetUserStatItemsSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsUserStatItemPagingSlicedResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUserStatItemsSuccess);
		OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUsersStatsItemsError);

		if (IsRunningDedicatedServer())
		{
			FMultiRegistry::GetServerApiClient()->ServerStatistic.GetUserStatItems(ABStatsUser->GetAccelByteId(), StatNames, {}, OnGetUserStatItemsSuccessHandler, OnError);
		}
		else
		{
			ApiClient->Statistic.GetUserStatItems(ABStatsUser->GetAccelByteId(), StatNames, {}, OnGetUserStatItemsSuccessHandler, OnError);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteQueryStatsUsers::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
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

void FOnlineAsyncTaskAccelByteQueryStatsUsers::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::TriggerDelegates();
	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)), OnlineUsersStatsPairs);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::Tick()
{
	Super::Tick();

	if (CountUsers == 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUserStatItemsSuccess(const FAccelByteModelsUserStatItemPagingSlicedResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("")); 

	if (Result.Data.Num() > 0)
	{
		TMap<FString, FVariantData> StatCodes;
		for (const auto& Stats : Result.Data)
		{
			FString Key = Stats.StatCode;
			FVariantData Value = Stats.Value;
			StatCodes.Add(Key, Value);
		}

		for (const auto& StatsUser : StatsUsers)
		{
			const FUniqueNetIdAccelByteUserRef ABStatsUser = FUniqueNetIdAccelByteUser::CastChecked(StatsUser);
			if (ABStatsUser->GetAccelByteId() == Result.Data[0].userId)
			{
				OnlineUsersStatsPairs.Add(MakeShareable(new FOnlineStatsUserStats(StatsUser, StatCodes)));
			}
		}
	}

	if (CountUsers > 0)
	{
		CountUsers--;
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUsersStatsItemsError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
	//continue until all users queried
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE