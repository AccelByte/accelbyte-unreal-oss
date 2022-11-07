// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateGameSessionV2.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSessionSettingsAccelByte.h"
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
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to create game session as our session interface is invalid!");

	FAccelByteModelsV2GameSessionCreateRequest CreateRequest;
	
	// Try and get session template name for creating the session, and error out if not found
	AB_ASYNC_TASK_ENSURE(NewSessionSettings.Get(SETTING_SESSION_TEMPLATE_NAME, CreateRequest.ConfigurationName), "Failed to create game session as a session template was not provided! A session setting must be present for SETTING_SESSION_TEMPLATE_NAME associated with a valid session template on the backend!");

	FString JoinTypeString;
	if (NewSessionSettings.Get(SETTING_SESSION_JOIN_TYPE, JoinTypeString) && !JoinTypeString.IsEmpty())
	{
		const EAccelByteV2SessionJoinability Joinability = SessionInterface->GetJoinabilityFromString(JoinTypeString);
		if (Joinability != EAccelByteV2SessionJoinability::EMPTY)
		{
			CreateRequest.Joinability = Joinability;
		}
	}

	FString ServerTypeString{};
	if (NewSessionSettings.Get(SETTING_SESSION_SERVER_TYPE, ServerTypeString) && !ServerTypeString.IsEmpty())
	{
		const EAccelByteV2SessionConfigurationServerType ServerType = SessionInterface->GetServerTypeFromString(ServerTypeString);
		if (ServerType != EAccelByteV2SessionConfigurationServerType::EMPTY)
		{
			CreateRequest.Type = ServerType;
		}
	}

	FString ClientVersion{};
	if (NewSessionSettings.Get(SETTING_GAMESESSION_CLIENTVERSION, ClientVersion) && !ClientVersion.IsEmpty())
	{
		CreateRequest.ClientVersion = ClientVersion;
	}

	FString Deployment;
	if (NewSessionSettings.Get(SETTING_GAMESESSION_DEPLOYMENT, Deployment) && !Deployment.IsEmpty())
	{
		CreateRequest.Deployment = Deployment;
	}

	FString ServerName{};
	if (NewSessionSettings.Get(SETTING_GAMESESSION_SERVERNAME, ServerName) && !ServerName.IsEmpty())
	{
		CreateRequest.ServerName = ServerName;
	}

	FOnlineSessionSettingsAccelByte::Get(NewSessionSettings, SETTING_GAMESESSION_REQUESTEDREGIONS, CreateRequest.RequestedRegions);

	int32 MinimumPlayers = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_MINIMUM_PLAYERS, MinimumPlayers) && MinimumPlayers > 0)
	{
		CreateRequest.MinPlayers = MinimumPlayers;
	}

	int32 MaximumPlayers = SessionInterface->GetSessionMaxPlayerCount(SessionName);
	if (MaximumPlayers > 0)
	{
		CreateRequest.MaxPlayers = MaximumPlayers;
	}

	int32 InactiveTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INACTIVE_TIMEOUT, InactiveTimeout) && InactiveTimeout > 0)
	{
		CreateRequest.InactiveTimeout = InactiveTimeout;
	}

	int32 InviteTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INVITE_TIMEOUT, InviteTimeout) && InviteTimeout > 0)
	{
		CreateRequest.InviteTimeout = InviteTimeout;
	}

	FString MatchPool{};
	if (NewSessionSettings.Get(SETTING_SESSION_MATCHPOOL, MatchPool) && !MatchPool.IsEmpty())
	{
		CreateRequest.MatchPool = MatchPool;
	}

	CreateRequest.Attributes.JsonObject = SessionInterface->ConvertSessionSettingsToJsonObject(NewSessionSettings);

	const THandler<FAccelByteModelsV2GameSession> OnCreateGameSessionSuccessDelegate = THandler<FAccelByteModelsV2GameSession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateGameSessionV2::OnCreateGameSessionSuccess);
	const FErrorHandler OnCreateGameSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateGameSessionV2::OnCreateGameSessionError);
	ApiClient->Session.CreateGameSession(CreateRequest, OnCreateGameSessionSuccessDelegate, OnCreateGameSessionErrorDelegate);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateGameSessionV2::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize game session creation as our session interface is invalid!"));
		return;
	}

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
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for creating game session as our session interface is invalid!"));
		return;
	}

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
