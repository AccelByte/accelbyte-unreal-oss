// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineLeaderboardInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Leaderboard/OnlineAsyncTaskAccelByteReadLeaderboards.h"

bool FOnlineLeaderboardAccelByte::GetFromSubsystem(
	const IOnlineSubsystem* Subsystem,
	TSharedPtr<FOnlineLeaderboardAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(Subsystem->GetLeaderboardsInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineLeaderboardAccelByte::GetFromWorld(
	const UWorld* World,
	TSharedPtr<FOnlineLeaderboardAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineLeaderboardAccelByte::ReadLeaderboards(
	TArray<FUniqueNetIdRef> const& Players,
	FOnlineLeaderboardReadRef& ReadObject)
{
	UE_LOG_ONLINE_LEADERBOARD(Display, TEXT("FOnlineLeaderboardAccelByte::ReadLeaderboards"));

	if (Players.Num() > 0)
	{
		if (!IsRunningDedicatedServer())
		{
			AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReadLeaderboards>(AccelByteSubsystem, AccelByteSubsystem->GetLocalUserNumCached(), Players, ReadObject);

			return true;
		}
		else
		{
			UE_LOG_ONLINE_LEADERBOARD(Warning, TEXT("ReadLeaderboards can only accessed for player"));
			return false;
		}
	}
	else
	{
		UE_LOG_ONLINE_LEADERBOARD(Warning, TEXT("Filtering with total 0 player ids is not supported"));
		return false;
	}
}

bool FOnlineLeaderboardAccelByte::ReadLeaderboardsForFriends(
	int32 LocalUserNum,
	FOnlineLeaderboardReadRef& ReadObject)
{
	UE_LOG_ONLINE_LEADERBOARD(Display, TEXT("FOnlineLeaderboardAccelByte::ReadLeaderboardsForFriends"));

	bool bReadFriendLeaderboard = false;

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	IOnlineFriendsPtr FriendsInterface = AccelByteSubsystem->GetFriendsInterface();
	IOnlineLeaderboardsPtr LeaderboardInterface = AccelByteSubsystem->GetLeaderboardsInterface();

	if (!IdentityInterface.IsValid())
	{
		UE_LOG_ONLINE_LEADERBOARD(Warning, TEXT("Fail to read leaderboards for friends as the identity interface is not valid"));
		return false;
	}

	if (IsRunningDedicatedServer())
	{
		UE_LOG_ONLINE_LEADERBOARD(Warning, TEXT("ReadLeaderboardsForFriends can only accessed for player"));
		return false;
	}

	const FUniqueNetIdPtr CurrentUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!CurrentUserId->IsValid())
	{
		UE_LOG_ONLINE_LEADERBOARD(Warning, TEXT("Fail to read leaderboards for friends as the user id is not valid"));
		return false;
	}

	if (!FriendsInterface.IsValid())
	{
		UE_LOG_ONLINE_LEADERBOARD(Warning, TEXT("Fail to read leaderboards for friends as the friends interface is not valid"));
		return false;
	}

	// Get Current User Friend List from Cache
	TArray <TSharedRef<FOnlineFriend>> FriendList;
	FriendsInterface->GetFriendsList(
		LocalUserNum, 
		TEXT(""), 
		FriendList);

	if (FriendList.Num() == 0)
	{
		LeaderboardInterface->TriggerOnLeaderboardReadCompleteDelegates(true);
		UE_LOG_ONLINE_LEADERBOARD(Warning, TEXT("Fail to read friend list as The User Has 0 Friends"));
		return true;
	}

	TArray<FUniqueNetIdRef> FriendsUserIds;
	for (const auto& Friend : FriendList)
	{
		FriendsUserIds.Add(Friend->GetUserId());
	}

	bReadFriendLeaderboard = ReadLeaderboards(FriendsUserIds, ReadObject);

	return bReadFriendLeaderboard;
}

bool FOnlineLeaderboardAccelByte::ReadLeaderboardsAroundRank(
	int32 Rank,
	uint32 Range,
	FOnlineLeaderboardReadRef& ReadObject)
{
	UE_LOG_AB(Warning, TEXT("warning. not implemented function. FOnlineLeaderboardAccelByte::ReadLeaderboardsAroundRank"));
	return false;
}

bool FOnlineLeaderboardAccelByte::ReadLeaderboardsAroundUser(
	FUniqueNetIdRef Player,
	uint32 Range,
	FOnlineLeaderboardReadRef& ReadObject)
{
	UE_LOG_AB(Warning, TEXT("warning. feature is not available. Calling ReadLeaderboards for single player specified with 0 range"));
	return true;
}

void FOnlineLeaderboardAccelByte::FreeStats(
	FOnlineLeaderboardRead& ReadObject)
{
	UE_LOG_AB(Warning, TEXT("warning. FreeStats feature is not available."));
	return;
}

bool FOnlineLeaderboardAccelByte::WriteLeaderboards(
	FName const& SessionName,
	FUniqueNetId const& Player,
	FOnlineLeaderboardWrite& WriteObject)
{
	UE_LOG_AB(Warning, TEXT("warning. WriteLeaderboards feature is not available."));
	return true;
}

bool FOnlineLeaderboardAccelByte::FlushLeaderboards(
	FName const& SessionName)
{
	UE_LOG_AB(Warning, TEXT("warning. FlushLeaderboards feature is not available."));
	return false;
}

bool FOnlineLeaderboardAccelByte::WriteOnlinePlayerRatings(
	FName const& SessionName
	, int32 LeaderboardId
	, TArray<FOnlinePlayerScore> const& PlayerScores)
{
	UE_LOG_AB(Warning, TEXT("warning. WriteOnlinePlayerRatings feature is not available."));
	return false;
}