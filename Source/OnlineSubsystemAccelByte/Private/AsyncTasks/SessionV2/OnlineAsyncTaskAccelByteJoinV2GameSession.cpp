// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJoinV2GameSession.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteJoinV2GameSession::FOnlineAsyncTaskAccelByteJoinV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, bool bInIsRestoreSession)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, bIsRestoreSession(bInIsRestoreSession)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s"), *UserId->ToDebugString(), *SessionName.ToString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to join game session as our session interface instance is invalid!");

	FNamedOnlineSession* SessionToJoin = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_ENSURE(SessionToJoin != nullptr, "Failed to join game session as our session instance is invalid!"); // Using ensure here as JoinSession should create a valid session before this is executed

	const FString SessionId = SessionToJoin->GetSessionIdStr();
	AB_ASYNC_TASK_ENSURE(!SessionId.Equals(TEXT("InvalidSession")), "Failed to join game session as the session ID was invalid!");

	if (bIsRestoreSession)
	{
		AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteJoinV2GameSession, GetGameSessionDetails, THandler<FAccelByteModelsV2GameSession>);
		ApiClient->Session.GetGameSessionDetails(SessionId, OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate);
	}
	else
	{
		AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteJoinV2GameSession, JoinGameSession, THandler<FAccelByteModelsV2GameSession>);
		ApiClient->Session.JoinGameSession(SessionId, OnJoinGameSessionSuccessDelegate, OnJoinGameSessionErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize joining a game session as our session interface is invalid!"));
		bWasSuccessful = false;
		JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
		return;
	}

	if (bWasSuccessful)
	{
		// Grab the session that we put in the creating state from the interface and put it in pending state, additionally
		// copy over the backend session info to the session for later use in updates.
		FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
		if (!ensure(JoinedSession != nullptr))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize joining a game session as our local session instance is invalid!"));
			bWasSuccessful = false;
			JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
			return;
		}

		JoinedSession->SessionState = EOnlineSessionState::Pending;

		// This will seem pretty silly, but take the open slots for the session and set them to the max number of slots. This
		// way registering and unregistering throughout the lifetime of the session will show proper counts.
		if (UpdatedBackendSessionInfo.Configuration.Joinability == EAccelByteV2SessionJoinability::INVITE_ONLY || UpdatedBackendSessionInfo.Configuration.Joinability == EAccelByteV2SessionJoinability::CLOSED)
		{
			JoinedSession->NumOpenPrivateConnections = JoinedSession->SessionSettings.NumPrivateConnections;
		}
		else
		{
			JoinedSession->NumOpenPublicConnections = JoinedSession->SessionSettings.NumPublicConnections;
		}

		// Remove any restored session instance or invite that we had for this session, since we joined it
		const FString SessionId = JoinedSession->GetSessionIdStr();
		SessionInterface->RemoveRestoreSessionById(SessionId);
		SessionInterface->RemoveInviteById(SessionId);

		// Update the game session data with what we received on join, that way if anything updated between the query and us
		// joining, we would apply that to the joined session
		SessionInterface->UpdateInternalGameSession(SessionName, UpdatedBackendSessionInfo, bJoiningP2P, true);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsMPV2GameSessionJoinedPayload GameSessionJoinedPayload{};
			GameSessionJoinedPayload.UserId = UserId->GetAccelByteId();
			GameSessionJoinedPayload.GameSessionId = SessionId;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2GameSessionJoinedPayload>(GameSessionJoinedPayload));
		}
	}
	else
	{
		// Remove pending session in session interface so that developer can retry joining, or create a new session
		SessionInterface->RemoveNamedSession(SessionName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (!bJoiningP2P)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates to joining game session as our session interface is invalid!"));
			return;
		}

		SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, JoinSessionResult);

		if(JoinSessionResult != EOnJoinSessionCompleteResult::Success)
		{
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}

		FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
		if (!ensure(JoinedSession != nullptr))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates to joining a game session as our local session instance is invalid!"));
			return;
		}

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(JoinedSession->SessionInfo);
		if (!ensure(SessionInfo.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates to joining a game session as our local session information instance is invalid!"));
			return;
		}

		if(SessionInfo->HasConnectionInfo())
		{
			SessionInterface->TriggerOnSessionServerUpdateDelegates(SessionName);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::OnJoinGameSessionSuccess(const FAccelByteModelsV2GameSession& InUpdatedBackendSessionInfo)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UpdatedBackendSessionInfo = InUpdatedBackendSessionInfo;
	JoinSessionResult = EOnJoinSessionCompleteResult::Success;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::OnJoinGameSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	switch(ErrorCode)
	{
		case static_cast<int32>(ErrorCodes::SessionJoinSessionFull):
			JoinSessionResult = EOnJoinSessionCompleteResult::SessionIsFull;
			break;
		case static_cast<int32>(ErrorCodes::SessionGameNotFound):
			JoinSessionResult = EOnJoinSessionCompleteResult::SessionDoesNotExist;
			break;
		default:
			JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
	}

	AB_ASYNC_TASK_REQUEST_FAILED("Failed to join game session on backend!", ErrorCode, ErrorMessage);
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::OnGetGameSessionDetailsSuccess(const FAccelByteModelsV2GameSession& InUpdatedBackendSessionInfo)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UpdatedBackendSessionInfo = InUpdatedBackendSessionInfo;
	JoinSessionResult = EOnJoinSessionCompleteResult::Success;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::OnGetGameSessionDetailsError(int32 ErrorCode, const FString& ErrorMessage)
{
	JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError; // #TODO #SESSIONv2 Maybe expand this to use a better error later?
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to restore game session on backend!", ErrorCode, ErrorMessage);
}

