// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateGameSessionV2.h"

#include "OnlineSessionSettingsAccelByte.h"
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
		if (ensure(SessionInterface.IsValid()))
		{
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
			if (ensure(Session != nullptr))
			{
				UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(Session->LocalOwnerId);
			}
		}
	}
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to update game session as our session interface is invalid!");

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_ENSURE(Session != nullptr, "Failed to update game session as our local session instance is invalid!");

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	AB_ASYNC_TASK_ENSURE(SessionInfo.IsValid(), "Failed to update game session as our local session info instance is invalid!");

	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2GameSession>(SessionInfo->GetBackendSessionData());
	AB_ASYNC_TASK_ENSURE(GameSessionBackendData.IsValid(), "Failed to update game session as our local backend session info is invalid!");

	FAccelByteModelsV2GameSessionUpdateRequest UpdateRequest;

	// Set version for update to be the current version on backend
	UpdateRequest.Version = GameSessionBackendData->Version;

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
	const TArray<FString> OldRequestedRegions = GameSessionBackendData->Configuration.RequestedRegions;
	TArray<FString> NewRequestedRegions;
	FOnlineSessionSettingsAccelByte::Get(NewSessionSettings, SETTING_GAMESESSION_REQUESTEDREGIONS, NewRequestedRegions);

	bool bUpdateRequestedRegions = NewRequestedRegions.Num() != OldRequestedRegions.Num();
	if(!bUpdateRequestedRegions)
	{
		for(const auto& NewRegion : NewRequestedRegions)
		{
			if(!OldRequestedRegions.Contains(NewRegion))
			{
				bUpdateRequestedRegions = true;
				break;
			}
		}
	}

	if(bUpdateRequestedRegions)
	{
		UpdateRequest.RequestedRegions = NewRequestedRegions;
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
	if (NewSessionSettings.Get(SETTING_SESSION_INACTIVE_TIMEOUT, InactiveTimeout) && InactiveTimeout != GameSessionBackendData->Configuration.InactiveTimeout)
	{
		UpdateRequest.InactiveTimeout = InactiveTimeout;
	}

	int32 InviteTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INVITE_TIMEOUT, InviteTimeout) && InviteTimeout != GameSessionBackendData->Configuration.InviteTimeout)
	{
		UpdateRequest.InviteTimeout = InviteTimeout;
	}

	FString ServerTypeString{};
	NewSessionSettings.Get(SETTING_SESSION_SERVER_TYPE, ServerTypeString);
	const EAccelByteV2SessionConfigurationServerType ServerType = SessionInterface->GetServerTypeFromString(ServerTypeString);
	if (ServerType != GameSessionBackendData->Configuration.Type && ServerType != EAccelByteV2SessionConfigurationServerType::EMPTY)
	{
		UpdateRequest.Type = ServerType;
	}

	// #NOTE Team assignments will override the session's members list on the backend!
	UpdateRequest.Teams = SessionInfo->GetTeamAssignments();

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
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize updating game session as our session interface is invalid!"));
			return;
		}

		SessionInterface->UpdateInternalGameSession(SessionName, NewSessionData);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for updating game session as our session interface is invalid!"));
		return;
	}

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
