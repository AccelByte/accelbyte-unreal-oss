// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteStartV1Matchmaking.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/OnlineSessionNames.h"
#endif // ENGINE_MAJOR_VERSION >= 5
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

using namespace AccelByte;

/**
 * Index of the player that is acting as host of this matchmaking session
 */
#define HOST_PLAYER_INDEX 0

FOnlineAsyncTaskAccelByteStartV1Matchmaking::FOnlineAsyncTaskAccelByteStartV1Matchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TArray<TSharedRef<const FUniqueNetId>>& InLocalPlayers, FName InSessionName, const FOnlineSessionSettings& InNewSessionSettings, TSharedRef<FOnlineSessionSearch>& InSearchSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalPlayers(InLocalPlayers)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
	, SearchSettings(InSearchSettings)
{
	// #NOTE(Maxwell): Use the first local player ID that initated matchmaking as the one that we are starting a matchmaking request for
	if (LocalPlayers.IsValidIndex(0))
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(LocalPlayers[0]);
	}
}

void FOnlineAsyncTaskAccelByteStartV1Matchmaking::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Session Name: %s"), *SessionName.ToString());

	if (SearchSettings->SearchState != EOnlineAsyncTaskState::NotStarted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start matchmaking as the search settings handle passed into the matchmaking call is in an invalid state. Should be NotStarted but is %s!"), EOnlineAsyncTaskState::ToString(SearchSettings->SearchState));
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
	const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Failed to create matchmaking session '%s', as we could not get the party interface to check if the host is in a party."), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (!IsApiClientValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Failed to create matchmaking session '%s', as we could not get the API client."), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// We really only need to make sure that the first user (the one initiating the matchmaking request) is in a party
	// so just grab the first user from local players and check that user
	if (!PartyInterface->IsPlayerInAnyParty(FUniqueNetIdAccelByteUser::CastChecked(LocalPlayers[HOST_PLAYER_INDEX]).Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Failed to create matchmaking session '%s', as the host player '%s' was not in a party!"), *SessionName.ToString(), *LocalPlayers[0]->ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Setup the session attributes for matchmaking
	TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());

	API_CLIENT_CHECK_GUARD(ErrorStringKey);
	API_CHECK_GUARD(Qos, ErrorStringKey);
	SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
	if (Qos->GetCachedLatencies().Num() <= 0)
	{
		const THandler<TArray<TPair<FString, float>>> OnGetServerLatenciesSuccessDelegate = TDelegateUtils<THandler<TArray<TPair<FString, float>>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV1Matchmaking::OnGetServerLatenciesSuccess);
		FErrorHandler OnGetServerLatenciesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV1Matchmaking::OnGetServerLatenciesError);
		Qos->GetServerLatencies(OnGetServerLatenciesSuccessDelegate, OnGetServerLatenciesErrorDelegate);
	}
	else
	{
		Latencies = Qos->GetCachedLatencies();
		API_CHECK_GUARD(Lobby, ErrorStringKey);
		AccelByte::Api::Lobby::FMatchmakingResponse OnStartMatchmakingResponseReceivedDelegate = TDelegateUtils<AccelByte::Api::Lobby::FMatchmakingResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV1Matchmaking::OnStartMatchmakingResponseReceived);
		Lobby->SetStartMatchmakingResponseDelegate(OnStartMatchmakingResponseReceivedDelegate);
		CreateMatchmakingSessionAndStartMatchmaking();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV1Matchmaking::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (!bWasSuccessful)
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV1Matchmaking::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(SubsystemPin->GetSessionInterface());
	if (!bWasSuccessful)
	{
		if (SessionInterface.IsValid())
		{
			FErrorInfo Info;
			Info.ErrorMessage = ErrorStringKey;
			SessionInterface->TriggerOnMatchmakingFailedDelegates(Info);
		}
	}
	else
	{
		SessionInterface->TriggerOnMatchmakingStartedDelegates();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV1Matchmaking::OnGetServerLatenciesSuccess(const TArray<TPair<FString, float>>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Latencies = Result;

	AccelByte::Api::Lobby::FMatchmakingResponse OnStartMatchmakingResponseReceivedDelegate = TDelegateUtils<AccelByte::Api::Lobby::FMatchmakingResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV1Matchmaking::OnStartMatchmakingResponseReceived);
	API_FULL_CHECK_GUARD(Lobby,ErrorStringKey);
	Lobby->SetStartMatchmakingResponseDelegate(OnStartMatchmakingResponseReceivedDelegate);
	CreateMatchmakingSessionAndStartMatchmaking();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV1Matchmaking::OnGetServerLatenciesError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get server latencies for matchmaking! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	ErrorStringKey = TEXT("matchmaking-failed-get-latencies");
}

void FOnlineAsyncTaskAccelByteStartV1Matchmaking::CreateMatchmakingSessionAndStartMatchmaking()
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

	TMap<FString, FString> PartyAttribute;
	FString MapName;
	if(SearchSettings->QuerySettings.Get(SETTING_MAPNAME, MapName))
	{
		PartyAttribute.Add(SETTING_MAPNAME.ToString(), MapName);
	}
	int32 NumBots = 0;
	if(SearchSettings->QuerySettings.Get(SETTING_NUMBOTS, NumBots))
	{
		PartyAttribute.Add(SETTING_NUMBOTS.ToString(), FString::FromInt(NumBots));
	}
	
	API_FULL_CHECK_GUARD(Lobby,ErrorStringKey);
	Lobby->SendStartMatchmaking(GameMode, ServerName, ClientVersion, Latencies, PartyAttribute);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent matchmaking request for session '%s' with game mode '%s' and server name of '%s'"), *SessionName.ToString(), *GameMode, *ServerName);
}

void FOnlineAsyncTaskAccelByteStartV1Matchmaking::OnStartMatchmakingResponseReceived(const FAccelByteModelsMatchmakingResponse& Result)
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
