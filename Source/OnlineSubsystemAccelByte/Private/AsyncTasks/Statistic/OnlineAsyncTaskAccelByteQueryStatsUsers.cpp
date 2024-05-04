// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryStatsUsers.h"
#include "OnlineError.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteQueryStatsUsers::FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte *const InABInterface
	, FUniqueNetIdRef const InLocalUserId
	, TArray<FUniqueNetIdRef> const& InStatsUsers
	, TArray<FString> const& InStatNames
	, FOnlineStatsQueryUsersStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, StatsUsers(InStatsUsers)
	, StatNames(InStatNames)
	, Delegate(InDelegate)
	, CountUsers(InStatsUsers.Num())
{
	if (!IsRunningDedicatedServer() && InLocalUserId->IsValid())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}

	if (CountUsers < 0)
	{
		CountUsers = 0;
	}
}

FOnlineAsyncTaskAccelByteQueryStatsUsers::FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte *const InABInterface
	, int32 InLocalUserNum 
	, TArray<FUniqueNetIdRef> const& InStatsUsers
	, TArray<FString> const& InStatNames
	, FOnlineStatsQueryUsersStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, LocalUserNum(InLocalUserNum)
	, StatsUsers(InStatsUsers)
	, StatNames(InStatNames)
	, Delegate(InDelegate)
	, CountUsers(InStatsUsers.Num())
{
	if (CountUsers < 0)
	{
		CountUsers = 0;
	}
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-query-stats-user-error-identity-invalid");
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
			ErrorMessage = TEXT("request-failed-query-stats-user-error-user-invalid");
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
		ErrorMessage = TEXT("request-failed-query-stats-user-error-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user stats, not logged in!"));
		return;
	}

	for (const auto& StatsUser : StatsUsers)
	{
		const FUniqueNetIdAccelByteUserRef ABStatsUser = FUniqueNetIdAccelByteUser::CastChecked(StatsUser);

		if (!ABStatsUser->IsValid())
		{
			CountUsers--;
			continue;
		}

		FString AccelByteUserId = ABStatsUser->GetAccelByteId();

		OnGetUserStatItemsSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsUserStatItemPagingSlicedResult>>::CreateThreadSafeSelfPtr(this
			, &FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUserStatItemsSuccess);
		OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
			, &FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUsersStatsItemsError);

		if (IsRunningDedicatedServer())
		{
			FMultiRegistry::GetServerApiClient()->ServerStatistic.GetUserStatItems(AccelByteUserId
				, StatNames
				, {}
				, OnGetUserStatItemsSuccessHandler
				, OnError);
		}
		else
		{
			API_CLIENT_CHECK_GUARD(ErrorMessage);
			ApiClient->Statistic.GetUserStatItems(AccelByteUserId
				, StatNames
				, {}
				, OnGetUserStatItemsSuccessHandler
				, OnError);
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

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		TSharedPtr<FAccelByteModelsUserStatItemGetItemsByCodesPayload> UserStatItemGetItemsByCodesPayload = MakeShared<FAccelByteModelsUserStatItemGetItemsByCodesPayload>();
		for (const auto& StatItem : QueryUserStatItemResponse.Data)
		{
			UserStatItemGetItemsByCodesPayload->UserId = StatItem.userId;
			UserStatItemGetItemsByCodesPayload->StatCodes.Add(StatItem.StatCode);
		}

		PredefinedEventInterface->SendEvent(LocalUserNum, UserStatItemGetItemsByCodesPayload.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::TriggerDelegates();
	EOnlineErrorResult Result = bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure;

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode
		, FText::FromString(ErrorMessage))
		, OnlineUsersStatsPairs);
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

void FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUserStatItemsSuccess(FAccelByteModelsUserStatItemPagingSlicedResult const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("")); 

	QueryUserStatItemResponse = Result;

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
				OnlineUsersStatsPairs.Add(MakeShared<FOnlineStatsUserStats>(StatsUser, StatCodes));
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

void FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUsersStatsItemsError(int32 Code
	, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s")
		, Code
		, *ErrMsg);
	//continue until all users queried
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE