// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteListUserStatItems.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteQueryStatsUsers.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteUpdateStats.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteResetUserStats.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteCreateStatsUser.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteUpdateStatsUsers.h"
#include "AsyncTasks/Statistic/OnlineAsyncTaskAccelByteDeleteStatsUsers.h"
#include "OnlineSubsystemUtils.h"

bool FOnlineStatisticAccelByte::ListUserStatItems(int32 LocalUserNum
	, const TArray<FString>& StatCodes
	, const TArray<FString>& Tags
	, const FString& AdditionalKey
	, bool bAlwaysRequestToService)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteListUserStatItems>(AccelByteSubsystem
		, LocalUserNum
		, StatCodes
		, Tags
		, AdditionalKey
		, bAlwaysRequestToService);
	return true;
}

bool FOnlineStatisticAccelByte::GetListUserStatItems(int32 LocalUserNum
	, TArray<TSharedRef<FAccelByteModelsFetchUser>>& OutUsers)
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

void FOnlineStatisticAccelByte::CreateStats(const int32 LocalUserNum
	, const FUniqueNetIdRef StatsUser
	, const TArray<FString>& StatNames
	, const FOnlineStatsCreateStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::CreateStats"));
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateStatsUser>(AccelByteSubsystem
		, LocalUserNum
		, StatsUser
		, StatNames
		, Delegate);
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

void FOnlineStatisticAccelByte::RemoveStats(const FUniqueNetIdRef StatsUserId
	, const FString& StatsCode)
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

void FOnlineStatisticAccelByte::ResetStats(const int32 LocalUserNum
	, const FUniqueNetIdRef StatsUserId)
{
#if !UE_BUILD_SHIPPING
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::ResetStats"));

	TSharedPtr<const FOnlineStatsUserStats> Value = GetStats(StatsUserId);

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteResetUserStats>(AccelByteSubsystem
		, LocalUserNum
		, StatsUserId
		, Value);
#endif // !UE_BUILD_SHIPPING
}

void FOnlineStatisticAccelByte::UpdateStats(const int32 LocalUserNum
	, const TArray<FOnlineStatsUserUpdatedStats>& BulkUpdateMultipleUserStatItems
	, const FOnUpdateMultipleUserStatItemsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::UpdateStats"));

	if (IsRunningDedicatedServer())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateStatsUsers>(AccelByteSubsystem
			, LocalUserNum
			, BulkUpdateMultipleUserStatItems
			, Delegate);
	}
}

void FOnlineStatisticAccelByte::UpdateStats(const FUniqueNetIdRef LocalUserId
	, const TArray<FOnlineStatsUserUpdatedStats>& UpdatedUserStats
	, const FOnUpdateMultipleUserStatItemsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::UpdateStats"));

	if (!IsRunningDedicatedServer())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateStats>
			(AccelByteSubsystem, LocalUserId, UpdatedUserStats, Delegate);
	}
}

void FOnlineStatisticAccelByte::DeleteStats(const int32 LocalUserNum
	, const FUniqueNetIdRef StatsUser
	, const FString& StatNames
	, const FString& AdditionalKey)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::DeleteStats"));

	if (IsRunningDedicatedServer())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteStatsUsers>(AccelByteSubsystem
			, LocalUserNum
			, StatsUser
			, StatNames
			, AdditionalKey);
	}
}

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId
	, const FUniqueNetIdRef StatsUser
	, const TArray<FString>& StatsNames
	, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	TArray<FUniqueNetIdRef> StatsUserList {StatsUser};
	
	auto QueryUsersStatsDelegate = FOnlineStatsQueryUsersStatsComplete::CreateLambda(
		[Delegate](const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
		{
			if (ResultState.bSucceeded && UsersStatsResult.Num() > 0)
			{
				Delegate.ExecuteIfBound(ResultState, UsersStatsResult[0]);
			}
			else
			{
				Delegate.ExecuteIfBound(ResultState, nullptr);
			}
		});
	
	QueryStats(LocalUserId
		, StatsUserList
		, StatsNames
		, QueryUsersStatsDelegate);
}

void FOnlineStatisticAccelByte::QueryStats(int32 LocalUserNum
	, const FUniqueNetIdRef StatsUser
	, const TArray<FString>& StatsNames
	, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	TArray<FUniqueNetIdRef> StatsUserList {StatsUser};
	
	auto QueryUsersStatsDelegate = FOnlineStatsQueryUsersStatsComplete::CreateLambda(
		[Delegate](const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
		{
			if (ResultState.bSucceeded && UsersStatsResult.Num() > 0)
			{
				Delegate.ExecuteIfBound(ResultState, UsersStatsResult[0]);
			}
			else
			{
				Delegate.ExecuteIfBound(ResultState, nullptr);
			}
		});
	
	QueryStats(LocalUserNum
		, StatsUserList
		, StatsNames
		, QueryUsersStatsDelegate);
}

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId
	, const FUniqueNetIdRef StatsUser
	, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	QueryStats(LocalUserId, StatsUser, {}, Delegate);
};

void FOnlineStatisticAccelByte::QueryStats(int32 LocalUserNum
	, const FUniqueNetIdRef StatsUser
	, const FOnlineStatsQueryUserStatsComplete& Delegate)
{
	QueryStats(LocalUserNum
		, StatsUser
		, {}
		, Delegate);
}

void FOnlineStatisticAccelByte::QueryStats(const FUniqueNetIdRef LocalUserId
	, const TArray<FUniqueNetIdRef>& StatsUsers
	, const TArray<FString>& StatsNames
	, const FOnlineStatsQueryUsersStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));
	
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUsers>(AccelByteSubsystem
		, LocalUserId
		, StatsUsers
		, StatsNames
		, Delegate);
}

void FOnlineStatisticAccelByte::QueryStats(int32 LocalUserNum
	, const TArray<FUniqueNetIdRef>& StatsUsers
	, const TArray<FString>& StatsNames
	, const FOnlineStatsQueryUsersStatsComplete& Delegate)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));
	
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		FUniqueNetIdPtr UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (UserIdPtr.IsValid() && UserIdPtr->IsValid())
		{
			QueryStats(UserIdPtr.ToSharedRef(), StatsUsers, StatsNames, Delegate);
		}
		else
		{
			AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUsers>(AccelByteSubsystem
				, LocalUserNum
				, StatsUsers
				, StatsNames
				, Delegate);
		}
	}
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

EAccelByteStatisticUpdateStrategy FOnlineStatisticAccelByte::ConvertUpdateStrategy(FOnlineStatUpdate::EOnlineStatModificationType Strategy)
{
	switch (Strategy)
	{
	case FOnlineStatUpdate::EOnlineStatModificationType::Largest:
		return EAccelByteStatisticUpdateStrategy::MAX;
	case FOnlineStatUpdate::EOnlineStatModificationType::Smallest:
		return EAccelByteStatisticUpdateStrategy::MIN;
	case FOnlineStatUpdate::EOnlineStatModificationType::Set:
		return EAccelByteStatisticUpdateStrategy::OVERRIDE;
	case FOnlineStatUpdate::EOnlineStatModificationType::Sum:
		return EAccelByteStatisticUpdateStrategy::INCREMENT;
	default:
		return EAccelByteStatisticUpdateStrategy::OVERRIDE;
	}
}