// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdatePartyV2.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineAsyncTaskAccelByteRefreshV2PartySession.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteUpdatePartyV2::FOnlineAsyncTaskAccelByteUpdatePartyV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
{
	TRY_PIN_SUBSYSTEM_CONSTRUCTOR()

	IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
	if (ensure(SessionInterface.IsValid()))
	{
		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		if (ensure(Session != nullptr))
		{
			UserId = FUniqueNetIdAccelByteUser::CastChecked(Session->LocalOwnerId.ToSharedRef());
		}
	}
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to update party session as our session interface is invalid!");

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Failed to update party session as our local session instance is invalid!");

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(OnlineSession->SessionInfo);
	AB_ASYNC_TASK_VALIDATE(SessionInfo.IsValid(), "Failed to update party session as our session info instance is invalid!");

	TSharedPtr<FAccelByteModelsV2PartySession> PartySessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	AB_ASYNC_TASK_VALIDATE(PartySessionBackendData.IsValid(), "Failed to update party session as our backend session information is invalid!");

	FAccelByteModelsV2PartyUpdateRequest UpdateRequest;

	// Set version for update to be the current version on backend
	UpdateRequest.Version = PartySessionBackendData->Version;

	// Currently we just want to update our attributes based on the new settings object passed in
	UpdateRequest.Attributes.JsonObject = SessionInterface->ConvertSessionSettingsToJsonObject(NewSessionSettings);
	
	// Check if joinability has changed and if so send it along to the backend
	FString JoinTypeString;
	NewSessionSettings.Get(SETTING_SESSION_JOIN_TYPE, JoinTypeString);
	
	const EAccelByteV2SessionJoinability JoinType = SessionInterface->GetJoinabilityFromString(JoinTypeString);
	if (JoinType != PartySessionBackendData->Configuration.Joinability && JoinType != EAccelByteV2SessionJoinability::EMPTY)
	{
		UpdateRequest.Joinability = JoinType;
	}

	int32 MinimumPlayers = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_MINIMUM_PLAYERS, MinimumPlayers) && MinimumPlayers != PartySessionBackendData->Configuration.MinPlayers)
	{
		UpdateRequest.MinPlayers = MinimumPlayers;
	}

	int32 InactiveTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INACTIVE_TIMEOUT, InactiveTimeout) && MinimumPlayers != PartySessionBackendData->Configuration.InactiveTimeout)
	{
		UpdateRequest.InactiveTimeout = InactiveTimeout;
	}

	int32 InviteTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INVITE_TIMEOUT, InviteTimeout) && InviteTimeout > MinimumPlayers != PartySessionBackendData->Configuration.InviteTimeout)
	{
		UpdateRequest.InviteTimeout = InviteTimeout;
	}

	OnUpdatePartySessionSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionSuccess);
	OnUpdatePartySessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionError);
	API_FULL_CHECK_GUARD(Session);
	Session->UpdateParty(OnlineSession->GetSessionIdStr(), UpdateRequest, OnUpdatePartySessionSuccessDelegate, OnUpdatePartySessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TRY_PIN_SUBSYSTEM();

		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize updating party session as our session interface is invalid!"));
			return;
		}

		SessionInterface->UpdateInternalPartySession(SessionName, NewSessionData);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for updating party session as our session interface is invalid!"));
		return;
	}

	if (bWasConflictError)
	{
		SessionInterface->TriggerOnSessionUpdateConflictErrorDelegates(SessionName, NewSessionSettings);
	}

	SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, bWasSuccessful);
	SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionSuccess(const FAccelByteModelsV2PartySession& BackendSessionData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UpdatedSessionVersion: %lli"), BackendSessionData.Version);

	NewSessionData = BackendSessionData;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to update party session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);

	// If this is a version conflict error, we want to refresh the local session data before completing the task
	if (ErrorCode != StaticCast<int32>(AccelByte::ErrorCodes::SessionUpdateVersionMismatch))
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	bWasConflictError = true;
	RefreshSession();
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::RefreshSession()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Could not refresh party session named '%s' as our session interface is invalid!", *SessionName.ToString());

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Could not refresh party session named '%s' as the session does not exist locally!", *SessionName.ToString());

	const FString SessionId = OnlineSession->GetSessionIdStr();
	AB_ASYNC_TASK_VALIDATE(!SessionId.Equals(TEXT("InvalidSession")), "Could not refresh party session named '%s' as there is not a valid session ID associated!", *SessionName.ToString());

	OnRefreshPartySessionSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePartyV2::OnRefreshPartySessionSuccess);
	OnRefreshPartySessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePartyV2::OnRefreshPartySessionError);;
	API_FULL_CHECK_GUARD(Session);
	Session->GetPartyDetails(SessionId, OnRefreshPartySessionSuccessDelegate, OnRefreshPartySessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::OnRefreshPartySessionSuccess(const FAccelByteModelsV2PartySession& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		SessionInterface->UpdateInternalPartySession(SessionName, Result);
	}

	// If we had to refresh the session, then the overall update request still failed
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::OnRefreshPartySessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Request to refresh party session failed on backend!", ErrorCode, ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
