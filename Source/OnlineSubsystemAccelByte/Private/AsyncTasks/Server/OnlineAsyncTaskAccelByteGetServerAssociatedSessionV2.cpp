// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetServerAssociatedSessionV2.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "GameServerApi/AccelByteServerSessionApi.h"

FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName)
    : FOnlineAsyncTaskAccelByte(InABInterface, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
    , SessionName(InSessionName)
{
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    // Try and get session ID from environment variables in case this server was explicitly spawned for a session. If
    // there is not a session ID in the environment variables for this server, then we make a request to get the
    // associated session ID for the server.
	SessionId = FPlatformMisc::GetEnvironmentVariable(TEXT("NOMAD_META_session_id"));
    if (SessionId.IsEmpty())
    {
		const THandler<FAccelByteModelsServerSessionResponse> OnGetSessionIdSuccessDelegate = THandler<FAccelByteModelsServerSessionResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetSessionIdSuccess);
		const FErrorHandler OnGetSessionIdErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetSessionIdError);
        FRegistry::ServerDSM.GetSessionId(OnGetSessionIdSuccessDelegate, OnGetSessionIdErrorDelegate);        
    }
    else
    {
        RequestSessionData();
    }

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::Finalize()
{
    Super::Finalize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

    if (bWasSuccessful)
    {
        // Super janky, but we want to remove the stub game session that we created to reflect creating state and
        // replace it with a new session based on the information retrieved from the backend.
        // #TODO Look into potentially having a better flow for this...
        SessionInterface->RemoveNamedSession(SessionName);

        FOnlineSession NewSession;
        SessionInterface->ConstructGameSessionFromBackendSessionModel(BackendSessionInfo, NewSession);

        FNamedOnlineSession* CreatedSession = SessionInterface->AddNamedSession(SessionName, NewSession);
        CreatedSession->SessionState = EOnlineSessionState::Pending;
    }
    else
    {
        SessionInterface->RemoveNamedSession(SessionName);
    }

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::TriggerDelegates()
{
    Super::TriggerDelegates();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);
    if (bWasSuccessful)
    {
        SessionInterface->TriggerOnGetServerAssociatedSessionCompleteDelegates(SessionName);
    }

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Result)
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *Result.Session_id);

    SessionId = Result.Session_id;
    RequestSessionData();

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetSessionIdError(int32 ErrorCode, const FString& ErrorMessage)
{
    UE_LOG_AB(Warning, TEXT("Failed to get session ID associated with server on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
    CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::RequestSessionData()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId);

	const THandler<FAccelByteModelsV2GameSession> OnGetGameSessionDetailsSuccessDelegate = THandler<FAccelByteModelsV2GameSession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetGameSessionDetailsSuccess);
	const FErrorHandler OnGetGameSessionDetailsErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetGameSessionDetailsError);
    FRegistry::ServerSession.GetGameSessionDetails(SessionId, OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetGameSessionDetailsSuccess(const FAccelByteModelsV2GameSession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId);

    BackendSessionInfo = Result;
    CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2::OnGetGameSessionDetailsError(int32 ErrorCode, const FString& ErrorMessage)
{
    UE_LOG_AB(Warning, TEXT("Failed to get information on session associated with this server from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
    CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
