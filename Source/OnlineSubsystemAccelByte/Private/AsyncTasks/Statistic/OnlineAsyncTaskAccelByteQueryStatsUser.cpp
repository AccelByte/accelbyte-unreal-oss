// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryStatsUser.h"
#include "OnlineError.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteQueryStatsUser::FOnlineAsyncTaskAccelByteQueryStatsUser(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum, const FUniqueNetId& InLocalUserId,
	const TSharedRef<const FUniqueNetId> InStatsUser, const TArray<FString>& InStatNames, const FOnlineStatsQueryUserStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalUserNum(InLocalUserNum)
	, StatsUser(FUniqueNetIdAccelByteUser::CastChecked(InStatsUser))
	, StatNames(InStatNames)
	, Delegate(InDelegate)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteQueryStatsUser")); 
	
	if (!IsRunningDedicatedServer() && InLocalUserId.IsValid())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::Initialize()
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

	if (!StatsUser->IsValid())
	{
		ErrorMessage = TEXT("request-failed-query-stats-user-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user stats, user is not valid!"));
		return;
	}

	THandler<FAccelByteModelsUserStatItemPagingSlicedResult> OnGetUserStatItemsSuccessHandler =
		TDelegateUtils<THandler<FAccelByteModelsUserStatItemPagingSlicedResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStatsUser::OnGetUserStatItemsSuccess);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStatsUser::OnGetUserStatsItemsError);

	const FString AccelByteUserId = StatsUser->GetAccelByteId();

	if (IsRunningDedicatedServer())
	{
		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		ServerApiClient->ServerStatistic.GetUserStatItems(AccelByteUserId, StatNames, {}, OnGetUserStatItemsSuccessHandler, OnError);
	}
	else
	{
		ApiClient->Statistic.GetUserStatItems(AccelByteUserId, StatNames, {}, OnGetUserStatItemsSuccessHandler, OnError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
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

void FOnlineAsyncTaskAccelByteQueryStatsUser::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::TriggerDelegates(); 
	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)), OnlineUserStatsPair);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::OnGetUserStatItemsSuccess(const FAccelByteModelsUserStatItemPagingSlicedResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("")); 

	for (const auto& Stat : Result.Data)
	{
		FString Key = Stat.StatCode;
		FVariantData Value = Stat.Value;
		Stats.Add(Key, Value);
	}
	
	OnlineUserStatsPair = MakeShared<FOnlineStatsUserStats>(StatsUser, Stats);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUser::OnGetUserStatsItemsError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
 
	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE