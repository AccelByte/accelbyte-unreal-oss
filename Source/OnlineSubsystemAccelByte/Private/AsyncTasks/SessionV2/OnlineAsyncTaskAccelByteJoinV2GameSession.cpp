// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJoinV2GameSession.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"

FOnlineAsyncTaskAccelByteJoinV2GameSession::FOnlineAsyncTaskAccelByteJoinV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s"), *UserId->ToDebugString(), *SessionName.ToString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	FNamedOnlineSession* SessionToJoin = SessionInterface->GetNamedSession(SessionName);
	ensure(SessionToJoin != nullptr); // Using ensure here as JoinSession should create a valid session before this is executed

	const THandler<FAccelByteModelsV2GameSession> OnJoinGameSessionSuccessDelegate = THandler<FAccelByteModelsV2GameSession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteJoinV2GameSession::OnJoinGameSessionSuccess);
	const FErrorHandler OnJoinGameSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteJoinV2GameSession::OnJoinGameSessionError);
	ApiClient->Session.JoinGameSession(SessionToJoin->GetSessionIdStr(), OnJoinGameSessionSuccessDelegate, OnJoinGameSessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSession::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	if (bWasSuccessful)
	{
		// Grab the session that we put in the creating state from the interface and put it in pending state, additionally
		// copy over the backend session info to the session for later use in updates.
		FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
		ensure(JoinedSession != nullptr);

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(JoinedSession->SessionInfo);
		ensure(SessionInfo.IsValid());

		SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(UpdatedBackendSessionInfo));
		JoinedSession->SessionState = EOnlineSessionState::Pending;

		// Additionally, pass to the session interface to remove any restored session instance that we were tracking for this
		// session, if any exists.
		SessionInterface->RemoveRestoreSessionById(JoinedSession->GetSessionIdStr());

		// Also, remove any invite we may have for this game session
		SessionInterface->RemoveInviteById(JoinedSession->GetSessionIdStr());
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

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, JoinSessionResult);

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
	UE_LOG_AB(Warning, TEXT("Failed to join game session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError; // #TODO #SESSIONv2 Maybe expand this to use a better error later?
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
