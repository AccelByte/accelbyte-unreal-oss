// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteListUserStatItems.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteQueryStatsUser.h"
#include "OnlineSubsystemUtils.h"
 

bool FOnlineStatisticAccelByte::ListUserStatItems(int32 LocalUserNum, const TArray<FString>& StatCodes, const TArray<FString>& Tags, const FString& AdditionalKey, bool bAlwaysRequestToService)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteListUserStatItems>(AccelByteSubsystem, *UserIdPtr.Get(),
					StatCodes, Tags, AdditionalKey, bAlwaysRequestToService);
				return true;
			}
			const FString ErrorStr = TEXT("list-user-stat-items-failed-userid-invalid");
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnListUserStatItemsCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsFetchUser>{}, ErrorStr);
			return false;
		}
	}

	const FString ErrorStr = TEXT("list-user-stat-items-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnListUserStatItemsCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsFetchUser>{}, ErrorStr);
	return false;
}

bool FOnlineStatisticAccelByte::GetListUserStatItems(int32 LocalUserNum, TArray<TSharedRef<FAccelByteModelsFetchUser>>& OutUsers)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	TArray<TSharedRef<FAccelByteModelsFetchUser>> Users = UsersMap.FindRef(IdentityInterface->GetUniquePlayerId(LocalUserNum)->AsShared());
	if (Users.Num() > 0)
	{
		for (TSharedRef<FAccelByteModelsFetchUser> User : Users)
		{
			OutUsers.Add(User);
		}
		return true;
	}
	return false;
}

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const TArray<FString>& StatNames, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));

	auto OnComplete = FOnlineStatsQueryUserStatsComplete::CreateLambda([&]
	(const FOnlineError& Error, const TSharedPtr<const FOnlineStatsUserStats>& Result)
		{
			UserStats = Result;
			Delegate.ExecuteIfBound(Error, Result);
		});
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUser>
		(AccelByteSubsystem, LocalUserId, StatsUser, StatNames, OnComplete);

};

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));
	
	auto OnComplete = FOnlineStatsQueryUserStatsComplete::CreateLambda([&]
	(const FOnlineError& Error, const TSharedPtr<const FOnlineStatsUserStats>& Result)
	{  
		UserStats = Result;
		Delegate.ExecuteIfBound(Error, Result);
	});
	TArray<FString> StatCodes{};
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUser>
		(AccelByteSubsystem, LocalUserId, StatsUser, StatCodes, OnComplete);

};

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId, const TArray<FUniqueNetIdRef>& StatUsers, const TArray<FString>& StatNames, const FOnlineStatsQueryUsersStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));
	const TArray<TSharedRef<const FOnlineStatsUserStats>> UsersStats;
	AccelByteSubsystem->ExecuteNextTick([UsersStats, Delegate]()
		{
			Delegate.ExecuteIfBound(FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))), UsersStats);
		});
}

TSharedPtr<const FOnlineStatsUserStats> FOnlineStatisticAccelByte::GetStats(const FUniqueNetIdRef StatsUserId) const
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::GetStats not implemented")); 
	return TSharedPtr<const FOnlineStatsUserStats>{};
}

void FOnlineStatisticAccelByte::UpdateStats(const FUniqueNetIdRef LocalUserId, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats, const FOnlineStatsUpdateStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::UpdateStats"));
	AccelByteSubsystem->ExecuteNextTick([Delegate]()
		{
			Delegate.ExecuteIfBound(FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))));
		});
}

void FOnlineStatisticAccelByte::ResetStats(const FUniqueNetIdRef StatsUserId)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::ResetStats not implemented")); 
}

