// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateV1GameSession.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"

FOnlineAsyncTaskAccelByteUpdateV1GameSession::FOnlineAsyncTaskAccelByteUpdateV1GameSession(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName, FOnlineSessionSettings& InUpdatedSessionSettings, uint32 InCurrentPlayer, bool InBShouldRefreshOnlineData)
    : FOnlineAsyncTaskAccelByte(InABInterface, true)
    , SessionName(InSessionName)
    , UpdatedSessionSettings(InUpdatedSessionSettings)
    , CurrentPlayer(InCurrentPlayer)
    , bShouldRefreshOnlineData(InBShouldRefreshOnlineData)
{
    LocalUserNum = Subsystem->GetLocalUserNumCached();
}

void FOnlineAsyncTaskAccelByteUpdateV1GameSession::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

    const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    check(SessionInterface != nullptr);

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not update game session as we failed to get session with name '%s'!"), *SessionName.ToString());
        CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
        return;
    }

    SessionId = Session->GetSessionIdStr();
    if (SessionId.IsEmpty())
    {
        AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not update game session as we failed to get session ID for session with name '%s'!"), *SessionName.ToString());
        CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
        return;
    }

    THandler<FAccelByteModelsSessionBrowserData> OnUpdateGameSessionSuccessDelegate = THandler<FAccelByteModelsSessionBrowserData>::CreateLambda([this](const FAccelByteModelsSessionBrowserData& Result) {
        CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
        UE_LOG_AB(Warning, TEXT("Update session '%s' success"), *SessionName.ToString());
    });

    FErrorHandler OnUpdateGameSessionErrorDelegate = FErrorHandler::CreateLambda([this](int32 ErrorCode, const FString& ErrorMessage) {
        CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
        UE_LOG_AB(Error, TEXT("Failed to update session '%s' from backend! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
    });

    API_CLIENT_CHECK_GUARD();
    ApiClient->SessionBrowser.UpdateGameSession(SessionId, UpdatedSessionSettings.NumPublicConnections, CurrentPlayer, OnUpdateGameSessionSuccessDelegate, OnUpdateGameSessionErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off task to update %s game sessions!"), *SessionName.ToString());
}

void FOnlineAsyncTaskAccelByteUpdateV1GameSession::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    check(Subsystem != nullptr);
    const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, bWasSuccessful);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}