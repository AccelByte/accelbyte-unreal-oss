// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteStartMatchmaking.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "GameServerApi/AccelByteServerMatchmakingApi.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"

/**
 * Index of the player that is acting as host of this matchmaking session
 */
#define HOST_PLAYER_INDEX 0

FOnlineAsyncTaskAccelByteStartMatchmaking::FOnlineAsyncTaskAccelByteStartMatchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TArray<TSharedRef<const FUniqueNetId>>& InLocalPlayers, FName InSessionName, const FOnlineSessionSettings& InNewSessionSettings, TSharedRef<FOnlineSessionSearch>& InSearchSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalPlayers(InLocalPlayers)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
	, SearchSettings(InSearchSettings)
{
	// #NOTE(Maxwell): Use the first local player ID that initated matchmaking as the one that we are starting a matchmaking request for
	if (LocalPlayers.IsValidIndex(0))
	{
		UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(LocalPlayers[0]);
	}
}

void FOnlineAsyncTaskAccelByteStartMatchmaking::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Session Name: %s"), *SessionName.ToString());

	if (SearchSettings->SearchState != EOnlineAsyncTaskState::NotStarted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start matchmaking as the search settings handle passed into the matchmaking call is in an invalid state. Should be NotStarted but is %s!"), *EOnlineAsyncTaskState::ToString(SearchSettings->SearchState));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (LocalPlayers.Num() < 1)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Failed to create matchmaking session '%s', must have at least one local player provided for matchmaking."), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	// AccelByte matchmaking requires that you are in a party when you send a request to start matchmaking, thus here we
	// check if the player is already in a party on the party interface. If we are not, then the call will fail.
	const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Failed to create matchmaking session '%s', as we could not get the party interface to check if the host is in a party."), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// We really only need to make sure that the first user (the one initiating the matchmaking request) is in a party
	// so just grab the first user from local players and check that user
	if (!PartyInterface->IsPlayerInAnyParty(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(LocalPlayers[HOST_PLAYER_INDEX]).Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Failed to create matchmaking session '%s', as the host player '%s' was not in a party!"), *SessionName.ToString(), *LocalPlayers[0]->ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Setup the session attributes for matchmaking
	TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	
	SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
	if (ApiClient->Qos.GetCachedLatencies().Num() <= 0)
	{
		const THandler<TArray<TPair<FString, float>>> OnGetServerLatenciesSuccessDelegate = THandler<TArray<TPair<FString, float>>>::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartMatchmaking::OnGetServerLatenciesSuccess);
		FErrorHandler OnGetServerLatenciesErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartMatchmaking::OnGetServerLatenciesError);
		ApiClient->Qos.GetServerLatencies(OnGetServerLatenciesSuccessDelegate, OnGetServerLatenciesErrorDelegate);
	}
	else
	{
		Latencies = ApiClient->Qos.GetCachedLatencies();
		AccelByte::Api::Lobby::FMatchmakingResponse OnStartMatchmakingResponseReceivedDelegate = AccelByte::Api::Lobby::FMatchmakingResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartMatchmaking::OnStartMatchmakingResponseReceived);
		ApiClient->Lobby.SetStartMatchmakingResponseDelegate(OnStartMatchmakingResponseReceivedDelegate);
		CreateMatchmakingSessionAndStartMatchmaking();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartMatchmaking::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (!bWasSuccessful)
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartMatchmaking::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (!bWasSuccessful)
	{
		const TSharedPtr<FOnlineSessionAccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionAccelByte>(Subsystem->GetSessionInterface());
		if (SessionInterface.IsValid())
		{
			FErrorInfo Info;
			Info.ErrorMessage = ErrorStringKey;
			SessionInterface->TriggerOnMatchmakingFailedDelegates(Info);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartMatchmaking::OnGetServerLatenciesSuccess(const TArray<TPair<FString, float>>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Latencies = Result;

	AccelByte::Api::Lobby::FMatchmakingResponse OnStartMatchmakingResponseReceivedDelegate = AccelByte::Api::Lobby::FMatchmakingResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartMatchmaking::OnStartMatchmakingResponseReceived);
	ApiClient->Lobby.SetStartMatchmakingResponseDelegate(OnStartMatchmakingResponseReceivedDelegate);
	CreateMatchmakingSessionAndStartMatchmaking();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartMatchmaking::OnGetServerLatenciesError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get server latencies for matchmaking! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	ErrorStringKey = TEXT("matchmaking-failed-get-latencies");
}

void FOnlineAsyncTaskAccelByteStartMatchmaking::CreateMatchmakingSessionAndStartMatchmaking()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Get the game mode specified in the search settings to determine what game mode to match against
	FString GameMode;
	SearchSettings->QuerySettings.Get(SETTING_GAMEMODE, GameMode);

	// For matchmaking, we only want to set the server name if we override it as a command line param, otherwise we will
	// always default to trying to find a local server in matchmaking when we may not want that
	FString ServerName;
	FParse::Value(FCommandLine::Get(), TEXT("ServerName="), ServerName);
	
	// Also allow for a version override in the command line to specify a specific version to attempt to match to
	FString ClientVersion;
	FParse::Value(FCommandLine::Get(), TEXT("ClientVersion="), ClientVersion);
	
	ApiClient->Lobby.SendStartMatchmaking(GameMode, ServerName, ClientVersion, Latencies);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent matchmaking request for session '%s' with game mode '%s' and server name of '%s'"), *SessionName.ToString(), *GameMode, *ServerName);
}

void FOnlineAsyncTaskAccelByteStartMatchmaking::OnStartMatchmakingResponseReceived(const FAccelByteModelsMatchmakingResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result Code : %s"), *Result.Code);

	if (Result.Code != TEXT("0"))
	{
		if (Result.Code == TEXT("11607") /* members banned from matchmaking */)
		{
			// #AB #TODO (Diar)
			// Set the correct error string key into key "matchmaking-game-temporarily-banned"
			ErrorStringKey = TEXT("matchmaking-game-failed-user-banned");
		}
		else
		{
			ErrorStringKey = TEXT("unable-to-enter-matchmaking");
		}
		
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
