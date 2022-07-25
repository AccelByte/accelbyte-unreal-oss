// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLeaveV2GameSession.h"

FOnlineAsyncTaskAccelByteLeaveV2GameSession::FOnlineAsyncTaskAccelByteLeaveV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InSessionId, const FOnLeaveSessionComplete& InDelegate)
    : FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionId(InSessionId)
	, Delegate(InDelegate)
{
    UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PlayerId: %s; SessionId: %s"), *UserId->ToDebugString(), *SessionId);

    const FVoidHandler OnLeaveGameSessionSuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteLeaveV2GameSession::OnLeaveGameSessionSuccess);
    const FErrorHandler OnLeaveGameSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteLeaveV2GameSession::OnLeaveGameSessionError);
    ApiClient->Session.LeaveGameSession(SessionId, OnLeaveGameSessionSuccessDelegate, OnLeaveGameSessionErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Regardless of whether we successfully left or not, we still want to destroy the session or remove the restored
	// instance. Session worker on backend should remove us after a timeout even if the call fails.
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	// Try and find a named session instance from the session interface to remove it
	FNamedOnlineSession* FoundSession = SessionInterface->GetNamedSessionById(SessionId);
	if (FoundSession != nullptr)
	{
		RemovedSessionName = FoundSession->SessionName;
		SessionInterface->RemoveNamedSession(FoundSession->SessionName);
		bRemovedNamedSession = true;
	}
	else
	{
		bRemovedRestoreSession = SessionInterface->RemoveRestoreSessionById(SessionId);
	}

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	if (bRemovedNamedSession)
	{
		SessionInterface->TriggerOnDestroySessionCompleteDelegates(RemovedSessionName, bWasSuccessful);
	}
	Delegate.ExecuteIfBound(true, SessionId);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::OnLeaveGameSessionSuccess()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::OnLeaveGameSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
    UE_LOG_AB(Warning, TEXT("Failed to leave game session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
    CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
