// Copyright (c) 2022 - 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryStatsUsers.h"
#include "OnlineError.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"
#define BATCH_QUERY_LIMIT 100

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
	TRY_PIN_SUBSYSTEM();
	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	bool bIsGameClient = (IsDS.IsSet() && !IsDS.GetValue()); //Is GameClient

	if (bIsGameClient && InLocalUserId->IsValid())
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

FOnlineAsyncTaskAccelByteQueryStatsUsers::FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte* const InABInterface
	, FUniqueNetIdRef const InLocalUserId
	, TArray<FUniqueNetIdRef> const& InStatsUsers
	, TArray<FString> const& InStatNames
	, TArray<FString> const& InTags
	, EAccelByteStatisticSortBy InSortBy
	, FOnlineStatsQueryUsersStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, StatsUsers(InStatsUsers)
	, StatNames(InStatNames)
	, Tags(InTags)
	, SortBy(InSortBy)
	, Delegate(InDelegate)
	, CountUsers(InStatsUsers.Num())
{
	TRY_PIN_SUBSYSTEM();
	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	bool bIsGameClient = (IsDS.IsSet() && !IsDS.GetValue()); //Is GameClient

	if (bIsGameClient && InLocalUserId->IsValid())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}

	if (CountUsers < 0)
	{
		CountUsers = 0;
	}
}

FOnlineAsyncTaskAccelByteQueryStatsUsers::FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, TArray<FUniqueNetIdRef> const& InStatsUsers
	, TArray<FString> const& InStatNames
	, TArray<FString> const& InTags
	, EAccelByteStatisticSortBy InSortBy
	, FOnlineStatsQueryUsersStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, StatsUsers(InStatsUsers)
	, StatNames(InStatNames)
	, Tags(InTags)
	, SortBy(InSortBy)
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
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-query-stats-user-error-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user stats, identity interface is invalid!"));
		return;
	}

	ELoginStatus::Type LoginStatus;

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	bool bIsGameClient = (IsDS.IsSet() && !IsDS.GetValue()); //Is GameClient

	if (bIsGameClient)
	{
		if (UserId.IsValid())
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

		if (!bIsGameClient)
		{
			SERVER_API_CLIENT_CHECK_GUARD();
			ServerApiClient->ServerStatistic.GetUserStatItems(AccelByteUserId
				, StatNames
				, Tags
				, OnGetUserStatItemsSuccessHandler
				, OnError
				, BATCH_QUERY_LIMIT
				, 0
				, SortBy);
		}
		else
		{
			API_FULL_CHECK_GUARD(Statistic, ErrorMessage);
			if (UserId->GetAccelByteId() == AccelByteUserId)
			{
				Statistic->GetMyStatItems(StatNames
					, Tags
					, OnGetUserStatItemsSuccessHandler
					, OnError);
			}
			else
			{
				Statistic->GetUserStatItems(AccelByteUserId
					, StatNames
					, Tags
					, OnGetUserStatItemsSuccessHandler
					, OnError
					, BATCH_QUERY_LIMIT
					, 0
					, SortBy);
			}
		}

		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	}
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Finalize();

	for (const TPair<FString, TArray<FAccelByteModelsUserStatItemInfo>>& UserStatPair : UserStatsRaw)
	{
		TMap<FString, FVariantData> StatCodes;
		for (const auto& Stats : UserStatPair.Value)
		{
			FString Key = Stats.StatCode;
			FVariantData Value = Stats.Value;
			StatCodes.Add(Key, Value);
		}

		for (const auto& StatsUser : StatsUsers)
		{
			const FUniqueNetIdAccelByteUserRef ABStatsUser = FUniqueNetIdAccelByteUser::CastChecked(StatsUser);
			if (ABStatsUser->GetAccelByteId() == UserStatPair.Key)
			{
				OnlineUsersStatsPairs.Add(MakeShared<FOnlineStatsUserStats>(StatsUser, StatCodes));
				break;
			}
		}

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (bWasSuccessful && PredefinedEventInterface.IsValid())
		{
			TSharedPtr<FAccelByteModelsUserStatItemGetItemsByCodesPayload> UserStatItemGetItemsByCodesPayload = MakeShared<FAccelByteModelsUserStatItemGetItemsByCodesPayload>();
			for (const auto& StatItem : UserStatPair.Value)
			{
				UserStatItemGetItemsByCodesPayload->UserId = StatItem.userId;
				UserStatItemGetItemsByCodesPayload->StatCodes.Add(StatItem.StatCode);
			}

			PredefinedEventInterface->SendEvent(LocalUserNum, UserStatItemGetItemsByCodesPayload.ToSharedRef());
		}
	}

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
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

void FOnlineAsyncTaskAccelByteQueryStatsUsers::QueryNextPage(const FAccelByteModelsUserStatItemPagingSlicedResult& NextPage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	int32 Offset = -1;
	int32 Count = -1;
	FString UrlOut;
	FString Params;
	NextPage.Paging.Next.Split(TEXT("?"), &UrlOut, &Params);
	if (!Params.IsEmpty())
	{
		TArray<FString> ParamsArray;
		Params.ParseIntoArray(ParamsArray, TEXT("&"));
		for (const FString& Param : ParamsArray)
		{
			FString Key;
			FString Value;
			Param.Split(TEXT("="), &Key, &Value);
			if (Key.Equals(TEXT("offset"), ESearchCase::IgnoreCase) && Value.IsNumeric())
			{
				Offset = FCString::Atoi(*Value);
			}
			else if (Key.Equals(TEXT("limit"), ESearchCase::IgnoreCase) && Value.IsNumeric())
			{
				Count = FCString::Atoi(*Value);
			}
		}

		if (Offset == -1 || Count == -1)
		{
			CountUsers--;
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failing to get Next Page! No offset or limit parameter in the URL!"));
			return;
		}
	}

	Count = FMath::Min(Count, BATCH_QUERY_LIMIT);

	OnGetUserStatItemsSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsUserStatItemPagingSlicedResult>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUserStatItemsSuccess);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUsersStatsItemsError);

	TRY_PIN_SUBSYSTEM();
	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	bool bIsGameClient = (IsDS.IsSet() && !IsDS.GetValue()); //Is GameClient

	if (!bIsGameClient)
	{
		SERVER_API_CLIENT_CHECK_GUARD(ErrorMessage);

		ServerApiClient->ServerStatistic.GetUserStatItems(NextPage.Data[0].userId
			, StatNames
			, Tags
			, OnGetUserStatItemsSuccessHandler
			, OnError
			, Count
			, Offset
			, SortBy);
	}
	else
	{
		API_FULL_CHECK_GUARD(Statistic, ErrorMessage);

		Statistic->GetUserStatItems(NextPage.Data[0].userId
			, StatNames
			, Tags
			, OnGetUserStatItemsSuccessHandler
			, OnError
			, Count
			, Offset
			, SortBy);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::OnGetUserStatItemsSuccess(FAccelByteModelsUserStatItemPagingSlicedResult const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (Result.Data.Num() > 0)
	{
		UserStatsRaw.FindOrAdd(Result.Data[0].userId).Append(Result.Data);
	}

	if (!Result.Paging.Next.IsEmpty())
	{
		QueryNextPage(Result);
	}
	else if (Result.Paging.Next.IsEmpty() && CountUsers > 0)
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
	// missing decrement the user count?
	if (CountUsers > 0)
	{
		CountUsers--;
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef BATCH_QUERY_LIMIT
#undef ONLINE_ERROR_NAMESPACE