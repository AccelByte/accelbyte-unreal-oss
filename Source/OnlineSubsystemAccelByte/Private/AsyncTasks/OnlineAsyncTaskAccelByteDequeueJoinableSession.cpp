// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineAsyncTaskAccelByteDequeueJoinableSession.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "GameServerApi/AccelByteServerMatchmakingApi.h"
#include "Core/AccelByteRegistry.h"
#include "Core/AccelByteError.h"
#include "Interfaces/OnlineSessionInterface.h"

FOnlineAsyncTaskAccelByteDequeueJoinableSession::FOnlineAsyncTaskAccelByteDequeueJoinableSession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnDequeueJoinableSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteDequeueJoinableSession::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	check(SessionInterface != nullptr);

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	check(Session != nullptr);

	if (!Session->SessionInfo.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot dequeue joinable session as our session info is nullptr!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByte> ABSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByte>(Session->SessionInfo);
	if (!ABSessionInfo.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot dequeue joinable session as our session info is nullptr!"));
		return;
	}

	FVoidHandler OnDequeueJoinableSessionSuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteDequeueJoinableSession::OnDequeueJoinableSessionSuccess);
	FErrorHandler OnDequeueJoinableSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteDequeueJoinableSession::OnDequeueJoinableSessionError);
	FRegistry::ServerMatchmaking.DequeueJoinableSession(ABSessionInfo->GetSessionId().ToString(), OnDequeueJoinableSessionSuccessDelegate, OnDequeueJoinableSessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to dequeue joinable session on backend."));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableSession::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		Session->SessionSettings.bAllowJoinInProgress = false;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableSession::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableSession::OnDequeueJoinableSessionSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Successfully dequeued joinable session for '%s' on DSM!"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableSession::OnDequeueJoinableSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to dequeue joinable session for '%s' on DSM! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
