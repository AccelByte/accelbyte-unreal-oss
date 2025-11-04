// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteV2GetRecentPlayer.h"

#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteV2GetRecentPlayer::FOnlineAsyncTaskAccelByteV2GetRecentPlayer(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId, const FString& InNamespace)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Namespace(InNamespace)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

void FOnlineAsyncTaskAccelByteV2GetRecentPlayer::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PlayerId: %s"), *UserId->ToDebugString());

	OnGetRecentPlayersSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2SessionRecentPlayers>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteV2GetRecentPlayer::OnGetRecentPlayersSuccess);
	OnGetRecentPlayersErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteV2GetRecentPlayer::OnGetRecentPlayersError);
	API_FULL_CHECK_GUARD(Session, ErrorStr);
	Session->GetRecentPlayers(OnGetRecentPlayersSuccessDelegate, OnGetRecentPlayersErrorDelegate, GetRecentPlayerLimit);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentPlayer::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TRY_PIN_SUBSYSTEM();

		TArray<TSharedRef<FOnlineRecentPlayerAccelByte>> RecentPlayers;
		for (const FAccelByteUserInfoRef& RecentPlayerInfo : RecentPlayersQueried)
		{
			FString AccelByteId = RecentPlayerInfo->Id->GetAccelByteId();
			TSharedPtr<FOnlineRecentPlayerAccelByte> NewRecent = MakeShared<FOnlineRecentPlayerAccelByte>(RecentPlayerInfo);
			NewRecent->LastSeen = RecentPlayerResultMap[AccelByteId].LastPlayedTime;
			RecentPlayers.Add(NewRecent.ToSharedRef());
		}

		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());
		FriendsInterface->RecentPlayersMap.Emplace(UserId.ToSharedRef(), RecentPlayers);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentPlayer::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
	if(!FriendsInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("failed to trigger delegate after completing get recent player because the friend interface instance is invalid!"));
		return;
	}
	FriendsInterface->TriggerOnQueryRecentPlayersCompleteDelegates(UserId.ToSharedRef().Get(), Namespace, bWasSuccessful, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentPlayer::OnGetRecentPlayersSuccess(const FAccelByteModelsV2SessionRecentPlayers& InResult)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	//store result in user ID map for faster lookup
	for(const FAccelByteModelsV2SessionRecentPlayer& PlayerData : InResult.Data)
	{
		RecentPlayerResultMap.Emplace(PlayerData.UserId, PlayerData);
	}
	
	// Construct array of user IDs to query for recent players
	TArray<FString> UsersToQuery;
	for (const FAccelByteModelsV2SessionRecentPlayer& Result : InResult.Data)
	{
		UsersToQuery.Add(Result.UserId);
	}

	// Query user information about these recent players
	FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not get information on recent players as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		OnQueryRecentPlayersCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(
			this, &FOnlineAsyncTaskAccelByteV2GetRecentPlayer::OnQueryRecentPlayersComplete);
		UserStore->QueryUsersByAccelByteIds(LocalUserNum, UsersToQuery, OnQueryRecentPlayersCompleteDelegate, true);
	}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteV2GetRecentPlayer::OnGetRecentPlayersError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("failed to get AB session recent player with error code %d, %s"), ErrorCode, *ErrorMessage);
	ErrorStr = TEXT("recent-players-retrieve-failed");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteV2GetRecentPlayer::OnQueryRecentPlayersComplete(bool bSuccess, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bSuccess)
	{
		RecentPlayersQueried = UsersQueried;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorStr = TEXT("recent-players-retrieve-player-info-failed");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}