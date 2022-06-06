// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetRecentPlayer.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteUserApi.h"
#include "Api/AccelByteSessionBrowserApi.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Models/AccelByteSessionBrowserModels.h"

FOnlineAsyncTaskAccelByteGetRecentPlayer::FOnlineAsyncTaskAccelByteGetRecentPlayer(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString &InNamespace)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Namespace(InNamespace)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, Namespace: %s"), *UserId->ToDebugString(), *Namespace);

	const THandler<FAccelByteModelsSessionBrowserRecentPlayerGetResult> SuccessDelegate = THandler<FAccelByteModelsSessionBrowserRecentPlayerGetResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerSuccess);
	const FErrorHandler ErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerError);

	// #AB (apin) limit only for 50 for splitgate
	ApiClient->SessionBrowser.GetRecentPlayer(UserId->GetAccelByteId(), SuccessDelegate, ErrorDelegate, 0, 50);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TArray<TSharedRef<FOnlineRecentPlayerAccelByte>> RecentPlayers;
		for (const TSharedRef<FAccelByteUserInfo>& RecentPlayerInfo : RecentPlayersQueried)
		{
			RecentPlayers.Add(MakeShared<FOnlineRecentPlayerAccelByte>(RecentPlayerInfo.Get()));
		}

		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendsInterface->RecentPlayersMap.Emplace(UserId.ToSharedRef(), RecentPlayers);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	FriendsInterface->TriggerOnQueryRecentPlayersCompleteDelegates(UserId.ToSharedRef().Get(), Namespace, bWasSuccessful, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerSuccess(const FAccelByteModelsSessionBrowserRecentPlayerGetResult& InResult)
{
	// Construct array of user IDs to query for recent players
	TArray<FString> UsersToQuery;
	for (const FAccelByteModelsSessionBrowserRecentPlayerData& Result : InResult.Data)
	{
		UsersToQuery.Add(Result.Other_id);
	}

	// Query user information about these recent players
	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not get information on recent players as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnQueryUsersComplete OnQueryRecentPlayersCompleteDelegate = FOnQueryUsersComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetRecentPlayer::OnQueryRecentPlayersComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, UsersToQuery, OnQueryRecentPlayersCompleteDelegate, true);
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("recent-players-retrieve-failed");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::OnQueryRecentPlayersComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	if (bIsSuccessful)
	{
		RecentPlayersQueried = UsersQueried;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorString = TEXT("recent-players-retrieve-player-info-failed");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}

