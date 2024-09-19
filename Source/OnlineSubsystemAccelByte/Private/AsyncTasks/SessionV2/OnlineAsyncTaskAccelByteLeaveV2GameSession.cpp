// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLeaveV2GameSession.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteLeaveV2GameSession::FOnlineAsyncTaskAccelByteLeaveV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InSessionId, const FOnLeaveSessionComplete& InDelegate, bool bInUserKicked)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionId(InSessionId)
	, Delegate(InDelegate)
	, bUserKicked(bInUserKicked)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PlayerId: %s; SessionId: %s"), *UserId->ToDebugString(), *SessionId);

	if (bUserKicked)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("User is kicked, skipping leave game session and mark this task as success."));
		return;
	}

	OnLeaveGameSessionSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLeaveV2GameSession::OnLeaveGameSessionSuccess);
	OnLeaveGameSessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLeaveV2GameSession::OnLeaveGameSessionError);
	API_CLIENT_CHECK_GUARD();
	ApiClient->Session.LeaveGameSession(SessionId, OnLeaveGameSessionSuccessDelegate, OnLeaveGameSessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Regardless of whether we successfully left or not, we still want to destroy the session or remove the restored
	// instance. Session worker on backend should remove us after a timeout even if the call fails.
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize leaving game session as our session interface is invalid!"));
		return;
	}

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

	if (bWasSuccessful)
	{
		SessionInterface->CurrentMatchmakingSearchHandle.Reset();
	}

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2GameSessionLeftPayload GameSessionLeftPayload{};
		GameSessionLeftPayload.UserId = UserId->GetAccelByteId();
		GameSessionLeftPayload.GameSessionId = SessionId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2GameSessionLeftPayload>(GameSessionLeftPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2GameSession::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for leaving game session as our session interface is invalid!"));
		return;
	}

	if (bRemovedNamedSession)
	{
		SessionInterface->TriggerOnDestroySessionCompleteDelegates(RemovedSessionName, bWasSuccessful);
	}
	Delegate.ExecuteIfBound(bWasSuccessful, SessionId);

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
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to leave game session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
