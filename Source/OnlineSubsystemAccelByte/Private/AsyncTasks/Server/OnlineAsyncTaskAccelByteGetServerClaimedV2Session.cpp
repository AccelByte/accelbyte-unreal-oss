// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetServerClaimedV2Session.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "GameServerApi/AccelByteServerSessionApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::FOnlineAsyncTaskAccelByteGetServerClaimedV2Session(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FString& InSessionId)
    : FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
    , SessionName(InSessionName)
    , SessionId(InSessionId)
{
}

void FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; SessionId: %s"), *SessionName.ToString(), *SessionId);

    AB_ASYNC_TASK_VALIDATE(!SessionId.IsEmpty(), "Session ID must not be empty!");

    SERVER_API_CLIENT_CHECK_GUARD();

    OnGetGameSessionDetailsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2GameSession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::OnGetGameSessionDetailsSuccess);
    OnGetGameSessionDetailsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::OnGetGameSessionDetailsError);;
	ServerApiClient->ServerSession.GetGameSessionDetails(SessionId, OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::Finalize()
{
    TRY_PIN_SUBSYSTEM();

    Super::Finalize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
    {
        AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize data for the session that has claimed this server as our session interface is invalid!"));
        return;
    }

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

void FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::TriggerDelegates()
{
    TRY_PIN_SUBSYSTEM();

    Super::TriggerDelegates();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
    {
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for notifying about claimed session as our session interface is invalid!"));
		return;
    }

	SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);
    if (bWasSuccessful)
    {
        SessionInterface->TriggerOnServerReceivedSessionDelegates(SessionName);
    }

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::OnGetGameSessionDetailsSuccess(const FAccelByteModelsV2GameSession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId);

    BackendSessionInfo = Result;
    CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerClaimedV2Session::OnGetGameSessionDetailsError(int32 ErrorCode, const FString& ErrorMessage)
{
    AB_ASYNC_TASK_REQUEST_FAILED("Failed to get information from backend on session that has claimed this server!", ErrorCode, ErrorMessage);
}
