// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateGameSessionV2.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "GameServerApi/AccelByteServerSessionApi.h"
#include "Api/AccelByteSessionApi.h"

FOnlineAsyncTaskAccelByteUpdateGameSessionV2::FOnlineAsyncTaskAccelByteUpdateGameSessionV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings)
	// Initialize as a server task if we are running a server task, as this doubles as a server task. Otherwise, use no flags
	: FOnlineAsyncTaskAccelByte(InABInterface, (IsRunningDedicatedServer()) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
{
	if (!IsRunningDedicatedServer())
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		ensure(SessionInterface.IsValid());

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		ensure(Session != nullptr);

		UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(Session->LocalOwnerId);
	}
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	ensure(Session != nullptr);

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	ensure(SessionInfo.IsValid());

	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2GameSession>(SessionInfo->GetBackendSessionData());
	ensure(GameSessionBackendData.IsValid());

	FAccelByteModelsV2GameSessionUpdateRequest UpdateRequest;

	// Set version for update to be the current version on backend
	UpdateRequest.Version = GameSessionBackendData->Version;

	// Check if we have an updated match pool and if so, send it with the update
	FString NewSettingsMatchPool;
	NewSessionSettings.Get(SETTING_SESSION_MATCHPOOL, NewSettingsMatchPool);
	if (!NewSettingsMatchPool.Equals(GameSessionBackendData->MatchPool))
	{
		UpdateRequest.MatchPool = NewSettingsMatchPool;
	}

	// Currently we just want to update our attributes based on the new settings object passed in
	UpdateRequest.Attributes.JsonObject = SessionInterface->ConvertSessionSettingsToJsonObject(NewSessionSettings);
	
	// Check if joinability has changed and if so send it along to the backend
	FString JoinTypeString;
	NewSessionSettings.Get(SETTING_SESSION_JOIN_TYPE, JoinTypeString);
	
	const EAccelByteV2SessionJoinability JoinType = SessionInterface->GetJoinabilityFromString(JoinTypeString);
	if (JoinType != GameSessionBackendData->Configuration.Joinability && JoinType != EAccelByteV2SessionJoinability::EMPTY)
	{
		UpdateRequest.Joinability = JoinType;
	}

	// Update requested regions for the DS request if the settings have changed
	const FString OldRequestedRegionsString = SessionInterface->ConvertToRegionListString(GameSessionBackendData->Configuration.RequestedRegions);
	FString NewRequestedRegionsString;
	NewSessionSettings.Get(SETTING_GAMESESSION_REQUESTEDREGIONS, NewRequestedRegionsString);
	if (!NewRequestedRegionsString.Equals(OldRequestedRegionsString))
	{
		UpdateRequest.RequestedRegions = SessionInterface->ConvertToRegionArray(NewRequestedRegionsString);
	}

	FString ClientVersion{};
	if (NewSessionSettings.Get(SETTING_GAMESESSION_CLIENTVERSION, ClientVersion) && ClientVersion != GameSessionBackendData->Configuration.ClientVersion)
	{
		UpdateRequest.ClientVersion = ClientVersion;
	}

	FString Deployment{};
	if (NewSessionSettings.Get(SETTING_GAMESESSION_CLIENTVERSION, Deployment) && Deployment != GameSessionBackendData->Configuration.Deployment)
	{
		UpdateRequest.Deployment = Deployment;
	}

	int32 MinimumPlayers = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_MINIMUM_PLAYERS, MinimumPlayers) && MinimumPlayers != GameSessionBackendData->Configuration.MinPlayers)
	{
		UpdateRequest.MinPlayers = MinimumPlayers;
	}

	int32 StoredMaximumPlayers = SessionInterface->GetSessionMaxPlayerCount(SessionName);
	if (StoredMaximumPlayers != GameSessionBackendData->Configuration.MaxPlayers)
	{
		UpdateRequest.MaxPlayers = StoredMaximumPlayers;
	}

	int32 InactiveTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INACTIVE_TIMEOUT, InactiveTimeout) && MinimumPlayers != GameSessionBackendData->Configuration.InactiveTimeout)
	{
		UpdateRequest.InactiveTimeout = InactiveTimeout;
	}

	int32 InviteTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INVITE_TIMEOUT, InviteTimeout) && InviteTimeout > MinimumPlayers != GameSessionBackendData->Configuration.InviteTimeout)
	{
		UpdateRequest.InviteTimeout = InviteTimeout;
	}

	// #TODO Need to find out something to do with teams... Though modifying teams will kick player if they are not
	// included... Bummer.

	// Send the API call based on whether we are a server or a client
	const THandler<FAccelByteModelsV2GameSession> OnUpdateGameSessionSuccessDelegate = THandler<FAccelByteModelsV2GameSession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdateGameSessionV2::OnUpdateGameSessionSuccess);
	const FErrorHandler OnUpdateGameSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdateGameSessionV2::OnUpdateGameSessionError);
	if (IsRunningDedicatedServer())
	{
		FRegistry::ServerSession.UpdateGameSession(SessionInfo->GetSessionId().ToString(), UpdateRequest, OnUpdateGameSessionSuccessDelegate, OnUpdateGameSessionErrorDelegate);
	}
	else
	{
		ApiClient->Session.UpdateGameSession(SessionInfo->GetSessionId().ToString(), UpdateRequest, OnUpdateGameSessionSuccessDelegate, OnUpdateGameSessionErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		ensure(SessionInterface.IsValid());

		SessionInterface->UpdateInternalGameSession(SessionName, NewSessionData);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::OnUpdateGameSessionSuccess(const FAccelByteModelsV2GameSession& BackendSessionData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UpdatedSessionVersion: %d"), BackendSessionData.Version);

	NewSessionData = BackendSessionData;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::OnUpdateGameSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to update game session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
