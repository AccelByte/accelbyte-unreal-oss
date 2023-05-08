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
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteQueryStatsUsers.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteUpdateStats.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteResetUserStats.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteCreateStatsUser.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteUpdateStatsUsers.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteDeleteStatsUsers.h"
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

void FOnlineStatisticAccelByte::CreateStats(const int32 LocalUserNum, const FUniqueNetIdRef StatsUser, const TArray<FString>& StatNames, const FOnlineStatsCreateStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::CreateStats"));
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateStatsUser>
		(AccelByteSubsystem, LocalUserNum, StatsUser, StatNames, Delegate);
}

void FOnlineStatisticAccelByte::EmplaceStats(const TSharedPtr<const FOnlineStatsUserStats>& InUserStats)
{
	FScopeLock ScopeLock(&StatsLock);

	int32 OldStatsIndex = UsersStats.Find(InUserStats.ToSharedRef());
	if(OldStatsIndex == INDEX_NONE)
	{
		UsersStats.Emplace(InUserStats.ToSharedRef());
	}
	else
	{
		TMap<FString, FVariantData> NewStats;
		NewStats.Append(UsersStats[OldStatsIndex]->Stats);
		NewStats.Append(InUserStats->Stats);
		const TSharedRef<const FOnlineStatsUserStats> NewUserStats = MakeShareable(new FOnlineStatsUserStats(InUserStats->Account, NewStats));
		UsersStats.Emplace(NewUserStats);
	}
}

void FOnlineStatisticAccelByte::EmplaceStats(const TArray<TSharedPtr<const FOnlineStatsUserStats>>& InUsersStats)
{
	for (const auto& InUserStats : InUsersStats)
	{
		EmplaceStats(InUserStats);
	}
}

void FOnlineStatisticAccelByte::RemoveStats(const FUniqueNetIdRef StatsUserId, const FString& StatsCode)
{
	FScopeLock ScopeLock(&StatsLock);

	TMap<FString, FVariantData> NewStat;
	int32 UserStatIndex = INDEX_NONE;

	for (const auto& UserStat : UsersStats)
	{
		if (UserStat->Account == StatsUserId)
		{
			UserStatIndex = UsersStats.Find(UserStat);
			NewStat.Append(UserStat->Stats);
			break;
		}
	}

	if (UserStatIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("User Id is not found %s"), *StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(StatsUserId)->GetAccelByteId());
		return;
	}

	if (NewStat.Remove(StatsCode) > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Removed value with key %s"), *StatsCode);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Key %s not found in map"), *StatsCode);
		return;
	}

	const TSharedRef<const FOnlineStatsUserStats> NewUserStats = MakeShareable(new FOnlineStatsUserStats(StatsUserId, NewStat));
	UsersStats[UserStatIndex] = NewUserStats;
}

void FOnlineStatisticAccelByte::ResetStats(const int32 LocalUserNum, const FUniqueNetIdRef StatsUserId)
{
#if !UE_BUILD_SHIPPING
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::ResetStats"));

	TSharedPtr<const FOnlineStatsUserStats> Value = GetStats(StatsUserId);

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteResetUserStats>(AccelByteSubsystem, LocalUserNum, StatsUserId, Value);
#endif // !UE_BUILD_SHIPPING
}

void FOnlineStatisticAccelByte::UpdateStats(const int32 LocalUserNum, const TArray<FOnlineStatsUserUpdatedStats>& BulkUpdateMultipleUserStatItems, const FOnUpdateMultipleUserStatItemsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::UpdateStats"));

	if (IsRunningDedicatedServer())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateStatsUsers>
			(AccelByteSubsystem, LocalUserNum, BulkUpdateMultipleUserStatItems, Delegate);
	}
}

void FOnlineStatisticAccelByte::DeleteStats(const int32 LocalUserNum, const FUniqueNetIdRef StatsUser, const FString& StatNames, const FString& AdditionalKey)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::DeleteStats"));

	if (IsRunningDedicatedServer())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteStatsUsers>
			(AccelByteSubsystem, LocalUserNum, StatsUser, StatNames, AdditionalKey);
	}
}

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const TArray<FString>& StatNames, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	QueryStats(0, LocalUserId.Get(), StatsUser, StatNames, Delegate);
}

void FOnlineStatisticAccelByte::QueryStats(const int32 LocalUserNum, const FUniqueNetIdRef StatsUser, const TArray<FString>& StatNames, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		QueryStats(LocalUserNum, *UserIdPtr.Get(), StatsUser, StatNames, Delegate);
	}
	else
	{
		QueryStats(LocalUserNum, *FUniqueNetIdPtr().Get(), StatsUser, StatNames, Delegate);
	}
}

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId, const FUniqueNetIdRef StatsUser, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	QueryStats(0, LocalUserId.Get(), StatsUser, {}, Delegate);
};

void FOnlineStatisticAccelByte::QueryStats(const int32 LocalUserNum, const FUniqueNetIdRef StatsUser, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		QueryStats(LocalUserNum, *UserIdPtr.Get(), StatsUser, {}, Delegate);
	}
	else
	{
		QueryStats(LocalUserNum, *FUniqueNetIdPtr().Get(), StatsUser, {}, Delegate);
	}
}

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId, const TArray<FUniqueNetIdRef>& StatUsers, const TArray<FString>& StatNames, const FOnlineStatsQueryUsersStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUsers>(AccelByteSubsystem, LocalUserId, StatUsers, StatNames, Delegate);
}

void FOnlineStatisticAccelByte::QueryStats(const int32 LocalUserNum, const TArray<FUniqueNetIdRef>& StatUsers, const TArray<FString>& StatNames, const FOnlineStatsQueryUsersStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUsers>(AccelByteSubsystem, LocalUserNum, StatUsers, StatNames, Delegate);
}

void FOnlineStatisticAccelByte::QueryStats(const int32 LocalUserNum, const FUniqueNetId& LocalUserId, const FUniqueNetIdRef StatsUser, const TArray<FString>& StatNames, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUser>(AccelByteSubsystem, LocalUserNum, LocalUserId, StatsUser, StatNames, Delegate);
}

TSharedPtr<const FOnlineStatsUserStats> FOnlineStatisticAccelByte::GetStats(const FUniqueNetIdRef StatsUserId) const
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::GetStats")); 
	
	FScopeLock ScopeLock(&StatsLock);
	TSharedPtr<const FOnlineStatsUserStats> Value = nullptr;
	if (!StatsUserId->IsValid())
	{
		return Value;
	}

	for (auto const& UserStat : UsersStats)
	{
		if (UserStat.Get().Account == StatsUserId)
		{
			Value = UserStat;
			break;
		}
	}

	return Value;
}

void FOnlineStatisticAccelByte::UpdateStats(const FUniqueNetIdRef LocalUserId, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats, const FOnlineStatsUpdateStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::UpdateStats"));

	if (!IsRunningDedicatedServer())
	{
		auto OnComplete = FOnlineStatsUpdateStatsComplete::CreateLambda([&](const FOnlineError& Error)
			{
				Delegate.ExecuteIfBound(Error);
			});
		TArray<FString> StatCodes{};
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateStats>
			(AccelByteSubsystem, LocalUserId, UpdatedUserStats, OnComplete);
	}
}

TSharedPtr<const FOnlineStatsUserStats> FOnlineStatisticAccelByte::GetAllListUserStatItemFromCache(const FUniqueNetIdRef StatsUserId) const
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::GetAllListUserStatItemFromCache"));
	if (UsersStats.Num() == 0)
	{
		return nullptr;
	}
	TSharedPtr<const FOnlineStatsUserStats> Value = GetStats(StatsUserId);

	return Value;
}

#if !UE_BUILD_SHIPPING
void FOnlineStatisticAccelByte::ResetStats(const FUniqueNetIdRef StatsUserId)
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());

	if (IsRunningDedicatedServer())
	{
		ResetStats(0, StatsUserId);
	}
	else
	{
		int32 UserNumber;
		IdentityInterface->GetLocalUserNum(StatsUserId.Get(),UserNumber);
		ResetStats(UserNumber, StatsUserId);
	}
}
#endif

