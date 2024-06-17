// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteV2GetRecentTeamPlayer.h"

#include "OnlineFriendsInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId, const FString& InNamespace)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Namespace(InNamespace)
{
	LocalUserNum = InLocalUserNum;
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
	}
}

void FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PlayerId: %s"), *UserId->ToDebugString());

	OnGetRecentTeamPlayersSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2SessionRecentPlayers>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::OnGetRecentTeamPlayersSuccess);
	OnGetRecentTeamPlayersErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::OnGetRecentTeamPlayersError);
	API_CLIENT_CHECK_GUARD();
	ApiClient->Session.GetRecentTeamPlayers(OnGetRecentTeamPlayersSuccessDelegate, OnGetRecentTeamPlayersErrorDelegate, GetRecentTeamPlayerLimit);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TArray<TSharedRef<FOnlineRecentPlayerAccelByte>> RecentPlayers;
		for (const FAccelByteUserInfoRef& RecentPlayerInfo : RecentTeamPlayersQueried)
		{
			FString AccelByteId = RecentPlayerInfo->Id->GetAccelByteId();
			TSharedPtr<FOnlineRecentPlayerAccelByte> NewRecent = MakeShared<FOnlineRecentPlayerAccelByte>(RecentPlayerInfo);
			NewRecent->LastSeen = RecentTeamPlayerResultMap[AccelByteId].LastPlayedTime;
			RecentPlayers.Add(NewRecent.ToSharedRef());
		}

		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendsInterface->RecentTeamPlayersMap.Emplace(UserId.ToSharedRef(), RecentPlayers);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
	if(!FriendsInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("failed to trigger delegate after completing get recent player because the friend interface instance is invalid!"));
		return;
	}
	FriendsInterface->TriggerOnQueryRecentTeamPlayersCompleteDelegates(LocalUserNum, Namespace, bWasSuccessful, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::OnGetRecentTeamPlayersSuccess(const FAccelByteModelsV2SessionRecentPlayers& InResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	//store result in user ID map for faster lookup
	for(const FAccelByteModelsV2SessionRecentPlayer& PlayerData : InResult.Data)
	{
		RecentTeamPlayerResultMap.Emplace(PlayerData.UserId, PlayerData);
	}
	
	// Construct array of user IDs to query for recent players
	TArray<FString> UsersToQuery;
	for (const FAccelByteModelsV2SessionRecentPlayer& Result : InResult.Data)
	{
		UsersToQuery.Add(Result.UserId);
	}

	// Query user information about these recent players
	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not get information on recent players as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		OnQueryRecentTeamPlayersCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(
			this, &FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::OnQueryRecentTeamPlayersComplete);
		UserStore->QueryUsersByAccelByteIds(LocalUserNum, UsersToQuery, OnQueryRecentTeamPlayersCompleteDelegate, true);
	}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::OnGetRecentTeamPlayersError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("failed to get AB session recent player with error code %d, %s"), ErrorCode, *ErrorMessage);
	ErrorStr = TEXT("recent-players-retrieve-failed");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer::OnQueryRecentTeamPlayersComplete(bool bSuccess, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bSuccess)
	{
		RecentTeamPlayersQueried = UsersQueried;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorStr = TEXT("recent-players-retrieve-player-info-failed");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}