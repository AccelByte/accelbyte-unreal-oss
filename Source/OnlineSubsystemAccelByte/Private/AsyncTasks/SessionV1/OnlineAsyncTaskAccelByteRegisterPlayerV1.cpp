// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRegisterPlayerV1.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserCacheAccelByte.h"
#include "Core/AccelByteRegistry.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRegisterPlayersV1::FOnlineAsyncTaskAccelByteRegisterPlayersV1(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers, bool InBWasInvited, bool InBIsSpectator)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, SessionName(InSessionName)
	, Players(InPlayers)
	, bWasInvited(InBWasInvited)
	, bIsSpectator(InBIsSpectator)
{
	TRY_PIN_SUBSYSTEM_CONSTRUCTOR()

	LocalUserNum = SubsystemPin->GetLocalUserNumCached();
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	if (Players.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not register players to session as the task was passed no player IDs!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
	check(SessionInterface != nullptr);

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not register player as we failed to get session with name '%s'!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Set up the number of players that we are attempting to register so that we can issue individual calls for each
	PendingPlayerRegistrations.Set(Players.Num());
	
	// Try and get our current session ID and bail if we can't
	SessionId = Session->GetSessionIdStr();
	if (SessionId.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not register player as we failed to get session ID for session with name '%s'!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	GetAllUserInformation();
	RegisterAllPlayers();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::Tick()
{
	Super::Tick();

	if (PendingPlayerRegistrations.GetValue() <= 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(SubsystemPin->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnRegisterPlayersCompleteDelegates(SessionName, SuccessfullyRegisteredPlayers, bWasSuccessful);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::GetAllUserInformation()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	SetLastUpdateTimeToCurrentTime();

	// #NOTE (Maxwell): This method should only retrieve user information if this is a client side register players call.
	// Server normally does not need to know detailed player information, however clients need display names and other
	// meta for in-game purposes. In addition, on a server build this call _will_ crash. Thus only run this on server builds...
	if (!IsRunningDedicatedServer())
	{
		FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
		if (!UserStore.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user information for players joining as our user store interface is invalid!"));
			return;
		}

		TArray<FString> IdsToQuery;
		for (const TSharedRef<const FUniqueNetId>& PlayerId : Players)
		{
			const TSharedRef<const FUniqueNetIdAccelByteUser> CompositePlayerId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);
			IdsToQuery.Add(CompositePlayerId->GetAccelByteId());
		}

		// #NOTE (Maxwell): Not binding a callback to this method, as we don't need the players information currently, just
		// cache the data for now so that later down the line other calls can use this cached data
		UserStore->QueryUsersByAccelByteIds(0, IdsToQuery, FOnQueryUsersComplete());
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::RegisterAllPlayers()
{
	TRY_PIN_SUBSYSTEM()

	SetLastUpdateTimeToCurrentTime();

	const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to register players to session as our session interface is invalid!");

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(Session != nullptr, "Failed to register players to session as our local session instance is invalid!");
	
	// For each player that we want to register to the session, we want to check if the player is already in the session,
	// if not, we want to add them to the local copy of the session, and if we are the session host, we want to take the
	// responsibility of registering the player to the session on the backend.
	for (int32 PlayerIndex = 0; PlayerIndex < Players.Num(); PlayerIndex++)
	{
		const TSharedRef<const FUniqueNetIdAccelByteUser> Player = FUniqueNetIdAccelByteUser::CastChecked(Players[PlayerIndex]);
		FUniqueNetIdMatcher PlayerMatch(Player.Get());
		if (Session->RegisteredPlayers.IndexOfByPredicate(PlayerMatch) != INDEX_NONE)
		{
			UE_LOG_AB(Warning, TEXT("Attempted to register player '%s' to session '%s' when player is already registered to the session!"), *Player->ToDebugString(), *SessionId);
			PendingPlayerRegistrations.Decrement();
			continue;
		}

		Session->RegisteredPlayers.Add(Player);
		FOnlineSubsystemAccelByteUtils::AddUserJoinTime(Player->GetAccelByteId(), FDateTime::Now().ToIso8601());

		// #AB Apin: splitgate use different type of calculating open connections
		if (bIsSpectator)
		{
			Session->NumOpenPrivateConnections--;
		}
		else
		{
			Session->NumOpenPublicConnections--;
		}

		const THandler<FAccelByteModelsSessionBrowserAddPlayerResponse> OnRegisterPlayerSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsSessionBrowserAddPlayerResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerSuccess, PlayerIndex);
		const FErrorHandler OnRegisterPlayerErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerError, Player->GetAccelByteId(), PlayerIndex);
		// NOTE(damar): SessionId with dashes is custom match (?)
		bool bIsCustomMatch = SessionId.Contains(TEXT("-"));
		if(bIsCustomMatch)
		{
#if UE_SERVER
			FRegistry::ServerSessionBrowser.RegisterPlayer(SessionId, Player->GetAccelByteId(), bIsSpectator, OnRegisterPlayerSuccessDelegate, OnRegisterPlayerErrorDelegate);
#else
			API_CLIENT_CHECK_GUARD();
			ApiClient->SessionBrowser.RegisterPlayer(SessionId, Player->GetAccelByteId(), bIsSpectator, OnRegisterPlayerSuccessDelegate, OnRegisterPlayerErrorDelegate);
#endif
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Attempted to register player '%s' to session '%s', player already added from matchmaking service!"), *Player->ToDebugString(), *SessionId);
			PendingPlayerRegistrations.Decrement();
		}
	}
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerSuccess(const FAccelByteModelsSessionBrowserAddPlayerResponse& Result, int32 Index)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Status: %s"), LOG_BOOL_FORMAT(Result.Status));

	SuccessfullyRegisteredPlayers.Add(Players[Index]);
	SetLastUpdateTimeToCurrentTime();
	PendingPlayerRegistrations.Decrement();	

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerError(int32 ErrorCode, const FString& ErrorMessage, FString PlayerId, int32 Index)
{
	UE_LOG_AB(Warning, TEXT("Failed to register player '%s' from session! Error code: %d; Error message: %s"), *PlayerId, ErrorCode, *ErrorMessage);
	PendingPlayerRegistrations.Decrement();
	SetLastUpdateTimeToCurrentTime();
}
