// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRegisterPlayerV1.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserCacheAccelByte.h"
#include "Core/AccelByteRegistry.h"

FOnlineAsyncTaskAccelByteRegisterPlayersV1::FOnlineAsyncTaskAccelByteRegisterPlayersV1(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers, bool InBWasInvited, bool InBIsSpectator)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, SessionName(InSessionName)
	, Players(InPlayers)
	, bWasInvited(InBWasInvited)
	, bIsSpectator(InBIsSpectator)
{
	LocalUserNum = Subsystem->GetLocalUserNumCached();
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	if (Players.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not register players to session as the task was passed no player IDs!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
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
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(Subsystem->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnRegisterPlayersCompleteDelegates(SessionName, SuccessfullyRegisteredPlayers, bWasSuccessful);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::GetAllUserInformation()
{
	SetLastUpdateTimeToCurrentTime();

	// #NOTE (Maxwell): This method should only retrieve user information if this is a client side register players call.
	// Server normally does not need to know detailed player information, however clients need display names and other
	// meta for in-game purposes. In addition, on a server build this call _will_ crash. Thus only run this on server builds...
#if !UE_SERVER
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user information for players joining as our user store interface is invalid!"));
		return;
	}

	TArray<FString> IdsToQuery;
	for (const TSharedRef<const FUniqueNetId>& PlayerId : Players)
	{
		const TSharedRef<const FUniqueNetIdAccelByteUser> CompositePlayerId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(PlayerId);
		IdsToQuery.Add(CompositePlayerId->GetAccelByteId());
	}

	// #NOTE (Maxwell): Not binding a callback to this method, as we don't need the players information currently, just
	// cache the data for now so that later down the line other calls can use this cached data
	UserStore->QueryUsersByAccelByteIds(0, IdsToQuery, FOnQueryUsersComplete());
#endif
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::RegisterAllPlayers()
{
	SetLastUpdateTimeToCurrentTime();

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	check(SessionInterface != nullptr);

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	ensure(Session != nullptr);
	
	// For each player that we want to register to the session, we want to check if the player is already in the session,
	// if not, we want to add them to the local copy of the session, and if we are the session host, we want to take the
	// responsibility of registering the player to the session on the backend.
	for (int32 PlayerIndex = 0; PlayerIndex < Players.Num(); PlayerIndex++)
	{
		const TSharedRef<const FUniqueNetIdAccelByteUser> Player = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Players[PlayerIndex]);
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

		const THandler<FAccelByteModelsSessionBrowserAddPlayerResponse> OnRegisterPlayerSuccessDelegate = THandler<FAccelByteModelsSessionBrowserAddPlayerResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerSuccess);
		const FErrorHandler OnRegisterPlayerErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerError, Player->GetAccelByteId());
		// NOTE(damar): SessionId with dashes is custom match (?)
		bool bIsCustomMatch = SessionId.Contains(TEXT("-"));
		if(bIsCustomMatch)
		{
#if UE_SERVER
			FRegistry::ServerSessionBrowser.RegisterPlayer(SessionId, Player->GetAccelByteId(), bIsSpectator, OnRegisterPlayerSuccessDelegate, OnRegisterPlayerErrorDelegate);
#else
			ApiClient->SessionBrowser.RegisterPlayer(SessionId, Player->GetAccelByteId(), bIsSpectator, OnRegisterPlayerSuccessDelegate, OnRegisterPlayerErrorDelegate);
#endif
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Attempted to register player '%s' to session '%s', player already added from matchmaking service!"), *Player->ToDebugString(), *SessionId);
		}
	}
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerSuccess(const FAccelByteModelsSessionBrowserAddPlayerResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Status: %s"), LOG_BOOL_FORMAT(Result.Status));

	SetLastUpdateTimeToCurrentTime();
	PendingPlayerRegistrations.Decrement();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterPlayersV1::OnRegisterPlayerError(int32 ErrorCode, const FString& ErrorMessage, FString PlayerId)
{
	UE_LOG_AB(Warning, TEXT("Failed to register player '%s' from session! Error code: %d; Error message: %s"), *PlayerId, ErrorCode, *ErrorMessage);
	PendingPlayerRegistrations.Decrement();
	SetLastUpdateTimeToCurrentTime();
}
