// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"

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

FOnlineStatisticAccelByte::FOnlineStatisticAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{}

bool FOnlineStatisticAccelByte::ListUserStatItems(int32 LocalUserNum
	, const TArray<FString>& StatCodes
	, const TArray<FString>& Tags
	, const FString& AdditionalKey
	, bool bAlwaysRequestToService)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteListUserStatItems>(AccelByteSubsystemPtr.Get()
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		return false;
	}

	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		return false;
	}

	TArray<TSharedRef<FAccelByteModelsFetchUser>> Users = UsersMap.FindRef(LocalUserId.ToSharedRef());
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateStatsUser>(AccelByteSubsystemPtr.Get()
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
		UE_LOG(LogAccelByteOSS, Warning, TEXT("User Id is not found %s"), *StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(StatsUserId)->GetAccelByteId());
		return;
	}

	if (NewStat.Remove(StatsCode) > 0)
	{
		UE_LOG(LogAccelByteOSS, Warning, TEXT("Removed value with key %s"), *StatsCode);
	}
	else
	{
		UE_LOG(LogAccelByteOSS, Warning, TEXT("Key %s not found in map"), *StatsCode);
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
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	TSharedPtr<const FOnlineStatsUserStats> Value = GetStats(StatsUserId);

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteResetUserStats>(AccelByteSubsystemPtr.Get()
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
		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if (!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
			return;
		}
		
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateStatsUsers>(AccelByteSubsystemPtr.Get()
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
		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if (!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
			return;
		}
		
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateStats>
			(AccelByteSubsystemPtr.Get(), LocalUserId, UpdatedUserStats, Delegate);
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
		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if (!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
			return;
		}
		
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteStatsUsers>(AccelByteSubsystemPtr.Get()
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

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUsers>(AccelByteSubsystemPtr.Get()
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

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (LocalUserId.IsValid() && LocalUserId->IsValid())
		{
			QueryStats(LocalUserId.ToSharedRef(), StatsUsers, StatsNames, Delegate);
		}
		else
		{
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUsers>(AccelByteSubsystemPtr.Get()
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
		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if (!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
			return;
		}
		
		auto OnComplete = FOnlineStatsUpdateStatsComplete::CreateLambda([&](const FOnlineError& Error)
			{
				Delegate.ExecuteIfBound(Error);
			});
		TArray<FString> StatCodes{};
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateStats>
			(AccelByteSubsystemPtr.Get(), LocalUserId, UpdatedUserStats, OnComplete);
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());

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

void FOnlineStatisticAccelByte::QueryStats(const int32 LocalUserNum, const FUniqueNetIdRef StatsUser
	, const TArray<FString>& StatsNames, const TArray<FString>& Tags, const FOnlineStatsQueryUserStatsComplete& Delegate
	, EAccelByteStatisticSortBy SortBy)
{
	UE_LOG_ONLINE_STATS(Display, TEXT("FOnlineStatisticAccelByte::QueryStats"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	TArray<FUniqueNetIdRef> StatsUserList{ StatsUser };

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

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryStatsUsers>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, StatsUserList
		, StatsNames
		, Tags
		, SortBy
		, QueryUsersStatsDelegate);
}

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