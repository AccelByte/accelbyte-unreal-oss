// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUnregisterPlayerV1.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Core/AccelByteRegistry.h"

FOnlineAsyncTaskAccelByteUnregisterPlayersV1::FOnlineAsyncTaskAccelByteUnregisterPlayersV1(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, SessionName(InSessionName)
	, Players(InPlayers)
{
	LocalUserNum = Subsystem->GetLocalUserNumCached();
}

void FOnlineAsyncTaskAccelByteUnregisterPlayersV1::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	if (Players.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister players from session as our task was passed an empty array of players!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	PendingPlayerUnregistrations.Set(Players.Num());

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	check(SessionInterface != nullptr);

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not unregister players from session as we failed to get session with name '%s'!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const FString SessionId = Session->GetSessionIdStr();
	if (SessionId.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not unregister players from session as the session ID for '%s' is blank!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// For each player that we want to unregister to the session, we want to check if the player is already in the session,
	// if not, we want to remove them from the local copy of the session, and if we are the session host, we want to take the
	// responsibility of unregistering the player from the session on the backend.
	for (int32 PlayerIndex = 0; PlayerIndex < Players.Num(); PlayerIndex++)
	{
		const TSharedRef<const FUniqueNetIdAccelByteUser> Player = FUniqueNetIdAccelByteUser::CastChecked(Players[PlayerIndex]);

		FUniqueNetIdMatcher PlayerMatch(Player.Get());
		int IndexOfPlayer = Session->RegisteredPlayers.IndexOfByPredicate(PlayerMatch);
		if (IndexOfPlayer == INDEX_NONE)
		{
			PendingPlayerUnregistrations.Decrement();
			UE_LOG_AB(Warning, TEXT("Cannot unregister player '%s' from session '%s' as the player is not included in the registered players of the session!"), *Player->GetAccelByteId(), *SessionId);
			return;
		}

		FOnlineSubsystemAccelByteUtils::AddUserDisconnectedTime(Player->GetAccelByteId(), FDateTime::Now().ToIso8601());

		// First, remove the player from the registered player array on the session and update open connection slots
		Session->RegisteredPlayers.RemoveAtSwap(IndexOfPlayer);
		Session->NumOpenPublicConnections++;

		// Next, signal to session manager that we have unregistered a player from the session
		THandler<FAccelByteModelsSessionBrowserAddPlayerResponse> OnUnregisterPlayerFromSessionSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsSessionBrowserAddPlayerResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnregisterPlayersV1::OnUnregisterPlayerFromSessionSuccess);
		FErrorHandler OnUnregisterPlayerFromSessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnregisterPlayersV1::OnUnregisterPlayerFromSessionError, Player->GetAccelByteId());
		// NOTE(damar): SessionId with dashes is custom match (?)
		bool bIsCustomMatch = SessionId.Contains(TEXT("-"));
		if(bIsCustomMatch)
		{
#if UE_SERVER
			FRegistry::ServerSessionBrowser.UnregisterPlayer(SessionId, Player->GetAccelByteId(), OnUnregisterPlayerFromSessionSuccessDelegate, OnUnregisterPlayerFromSessionErrorDelegate);
#else
			ApiClient->SessionBrowser.UnregisterPlayer(SessionId, Player->GetAccelByteId(), OnUnregisterPlayerFromSessionSuccessDelegate, OnUnregisterPlayerFromSessionErrorDelegate);
#endif
		}
		else
		{
			// TODO(damar): Remove from session using ServerMatchmaking.
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterPlayersV1::Tick()
{
	Super::Tick();
	
	if (PendingPlayerUnregistrations.GetValue() <= 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteUnregisterPlayersV1::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(Subsystem->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnUnregisterPlayersCompleteDelegates(SessionName, SuccessfullyUnregisteredPlayers, bWasSuccessful);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterPlayersV1::OnUnregisterPlayerFromSessionSuccess(const FAccelByteModelsSessionBrowserAddPlayerResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Status: %s"), LOG_BOOL_FORMAT(Result.Status));

	SetLastUpdateTimeToCurrentTime();
	PendingPlayerUnregistrations.Decrement();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterPlayersV1::OnUnregisterPlayerFromSessionError(int32 ErrorCode, const FString& ErrorMessage, FString PlayerId)
{
	UE_LOG_AB(Warning, TEXT("Failed to unregister player '%s' from session! Error code: %d; Error message: %s"), *PlayerId, ErrorCode, *ErrorMessage);
	PendingPlayerUnregistrations.Decrement();
	SetLastUpdateTimeToCurrentTime();
}
