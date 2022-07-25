// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateGameSessionV2.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

FOnlineAsyncTaskAccelByteUpdateGameSessionV2::FOnlineAsyncTaskAccelByteUpdateGameSessionV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
{
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	ensure(SessionInterface.IsValid());

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	ensure(Session != nullptr);

	UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(Session->OwningUserId);
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
	UpdateRequest.Teams = SessionInfo->GetTeamAssignments();
	
	// Check if joinability has changed and if so send it along to the backend
	FString JoinTypeString;
	NewSessionSettings.Get(SETTING_SESSION_JOIN_TYPE, JoinTypeString);
	
	EAccelByteV2SessionJoinability JoinType = SessionInterface->GetJoinabilityFromString(JoinTypeString);
	if (JoinType != GameSessionBackendData->JoinType)
	{
		UpdateRequest.JoinType = JoinType;
	}

	// Update requested regions for the DS request if the settings have changed
	const FString OldRequestedRegionsString = SessionInterface->ConvertToRegionListString(GameSessionBackendData->DSInformation.RequestedRegions);
	FString NewRequestedRegionsString;
	NewSessionSettings.Get(SETTING_GAMESESSION_REQUESTEDREGIONS, NewRequestedRegionsString);
	if (!NewRequestedRegionsString.Equals(OldRequestedRegionsString))
	{
		UpdateRequest.DSRequest.RequestedRegions = SessionInterface->ConvertToRegionArray(NewRequestedRegionsString);
	}

	const THandler<FAccelByteModelsV2GameSession> OnUpdateGameSessionSuccessDelegate = THandler<FAccelByteModelsV2GameSession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdateGameSessionV2::OnUpdateGameSessionSuccess);
	const FErrorHandler OnUpdateGameSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdateGameSessionV2::OnUpdateGameSessionError);

	ApiClient->Session.UpdateGameSession(SessionInfo->GetSessionId().ToString(), UpdateRequest, OnUpdateGameSessionSuccessDelegate, OnUpdateGameSessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateGameSessionV2::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		ensure(SessionInterface.IsValid());

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		ensure(Session != nullptr);

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
		ensure(SessionInfo.IsValid());

		SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(NewSessionData));
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
