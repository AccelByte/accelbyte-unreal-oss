// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateGameSessionV2.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemSessionSettings.h"

FOnlineAsyncTaskAccelByteCreateGameSessionV2::FOnlineAsyncTaskAccelByteCreateGameSessionV2(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InHostingPlayerId, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface, false)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InHostingPlayerId.AsShared());
}

void FOnlineAsyncTaskAccelByteCreateGameSessionV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *UserId->ToDebugString(), *SessionName.ToString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	FAccelByteModelsV2GameSessionCreateRequest CreateRequest;
	
	FString JoinTypeString;
	NewSessionSettings.Get(SETTING_SESSION_JOIN_TYPE, JoinTypeString);
	CreateRequest.JoinType = SessionInterface->GetJoinabilityFromString(JoinTypeString);

	NewSessionSettings.Get(SETTING_SESSION_MATCHPOOL, CreateRequest.MatchPool);

	// Try and get session template name for creating the session, and error out if not found
	if (!NewSessionSettings.Get(SETTING_SESSION_TEMPLATE_NAME, CreateRequest.ConfigurationName))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game session as a session template was not provided! A session setting must be present for SETTING_SESSION_TEMPLATE_NAME associated with a valid session template on the backend!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// If this is a dedicated session, then we need to request a DS as well
	if (NewSessionSettings.bIsDedicated)
	{
		FAccelByteModelsV2DSRequest DsRequest;
		NewSessionSettings.Get(SETTING_GAMESESSION_CLIENTVERSION, DsRequest.ClientVersion);
		NewSessionSettings.Get(SETTING_GAMESESSION_DEPLOYMENT, DsRequest.Deployment);
		NewSessionSettings.Get(SETTING_GAMEMODE, DsRequest.GameMode);

		FString RegionListString{};
		NewSessionSettings.Get(SETTING_GAMESESSION_REQUESTEDREGIONS, RegionListString);
		DsRequest.RequestedRegions = SessionInterface->ConvertToRegionArray(RegionListString);

		CreateRequest.DSRequest = DsRequest;
	}

	// Add the number of public and private connections for this session to the session settings key/value set so that it
	// serializes to the session
	NewSessionSettings.Set(SETTING_SESSION_NUM_PUBLIC_CONNECTIONS, NewSessionSettings.NumPublicConnections);
	NewSessionSettings.Set(SETTING_SESSION_NUM_PRIVATE_CONNECTIONS, NewSessionSettings.NumPrivateConnections);

	CreateRequest.Attributes.JsonObject = SessionInterface->ConvertSessionSettingsToJsonObject(NewSessionSettings);

	// Add the hosting player to a team object in the request
	FAccelByteModelsV2GameSessionTeam NewTeam;
	NewTeam.UserIDs.Add(UserId->GetAccelByteId());
	CreateRequest.Teams.Add(NewTeam);

	const THandler<FAccelByteModelsV2GameSession> OnCreateGameSessionSuccessDelegate = THandler<FAccelByteModelsV2GameSession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateGameSessionV2::OnCreateGameSessionSuccess);
	const FErrorHandler OnCreateGameSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateGameSessionV2::OnCreateGameSessionError);
	ApiClient->Session.CreateGameSession(CreateRequest, OnCreateGameSessionSuccessDelegate, OnCreateGameSessionErrorDelegate);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameSessionV2::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	if (bWasSuccessful)
	{
		// If we successfully created the session on backend, then we want to fill out the rest of the data on the session
		// in session interface
		SessionInterface->FinalizeCreateGameSession(SessionName, CreatedGameSession);
	}
	else
	{
		// Otherwise, nuke the session so that we can retry creating a session after
		SessionInterface->RemoveNamedSession(SessionName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameSessionV2::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameSessionV2::OnCreateGameSessionSuccess(const FAccelByteModelsV2GameSession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *Result.ID);

	CreatedGameSession = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameSessionV2::OnCreateGameSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to create game session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
