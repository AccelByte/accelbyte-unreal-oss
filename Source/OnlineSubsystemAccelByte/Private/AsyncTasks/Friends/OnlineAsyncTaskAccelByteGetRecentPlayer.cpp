// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#if 1 // MMv1 Deprecation

#include "OnlineAsyncTaskAccelByteGetRecentPlayer.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"

#include "Api/AccelByteUserApi.h"
#include "Api/AccelByteSessionBrowserApi.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Models/AccelByteSessionBrowserModels.h"

#include "Core/AccelByteReport.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetRecentPlayer::FOnlineAsyncTaskAccelByteGetRecentPlayer(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString &InNamespace)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Namespace(InNamespace)
{
	FReport::LogDeprecated(FString(__FUNCTION__),
		TEXT("The session browser is deprecated and replaced by game sessions. For more information, see https://docs.accelbyte.io/gaming-services/services/play/peer-to-peer-via-relay-server/"));
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, Namespace: %s"), *UserId->ToDebugString(), *Namespace);

	const THandler<FAccelByteModelsSessionBrowserRecentPlayerGetResult> SuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsSessionBrowserRecentPlayerGetResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerSuccess);
	const FErrorHandler ErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerError);

	// #AB (apin) limit only for 50 for splitgate
	API_FULL_CHECK_GUARD(SessionBrowser, ErrorString);
	SessionBrowser->GetRecentPlayer(UserId->GetAccelByteId(), SuccessDelegate, ErrorDelegate, 0, 50);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TArray<TSharedRef<FOnlineRecentPlayerAccelByte>> RecentPlayers;
		for (const FAccelByteUserInfoRef& RecentPlayerInfo : RecentPlayersQueried)
		{
			RecentPlayers.Add(MakeShared<FOnlineRecentPlayerAccelByte>(RecentPlayerInfo));
		}

		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());
		FriendsInterface->RecentPlayersMap.Emplace(UserId.ToSharedRef(), RecentPlayers);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
	FriendsInterface->TriggerOnQueryRecentPlayersCompleteDelegates(UserId.ToSharedRef().Get(), Namespace, bWasSuccessful, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerSuccess(const FAccelByteModelsSessionBrowserRecentPlayerGetResult& InResult)
{
	TRY_PIN_SUBSYSTEM();

	// Construct array of user IDs to query for recent players
	TArray<FString> UsersToQuery;
	for (const FAccelByteModelsSessionBrowserRecentPlayerData& Result : InResult.Data)
	{
		UsersToQuery.Add(Result.Other_id);
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
		FOnQueryUsersComplete OnQueryRecentPlayersCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetRecentPlayer::OnQueryRecentPlayersComplete);
		UserStore->QueryUsersByAccelByteIds(LocalUserNum, UsersToQuery, OnQueryRecentPlayersCompleteDelegate, true);
	}));
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::OnGetRecentPlayerError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("recent-players-retrieve-failed");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetRecentPlayer::OnQueryRecentPlayersComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried)
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

#endif