// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineAsyncTaskAccelByteEnqueueJoinableSession.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameServerApi/AccelByteServerMatchmakingApi.h"
#include "Core/AccelByteRegistry.h"
#include "Core/AccelByteError.h"

FOnlineAsyncTaskAccelByteEnqueueJoinableSession::FOnlineAsyncTaskAccelByteEnqueueJoinableSession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnEnqueueJoinableSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableSession::Initialize()
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
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot enqueue joinable session as our session info is nullptr!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByte> ABSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByte>(Session->SessionInfo);
	if (!ABSessionInfo.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot enqueue joinable session as our session info is nullptr!"));
		return;
	}

	FVoidHandler OnEnqueueJoinableSessionSuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteEnqueueJoinableSession::OnEnqueueJoinableSessionSuccess);
	FErrorHandler OnEnqueueJoinableSessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteEnqueueJoinableSession::OnEnqueueJoinableSessionError);
	// #NOTE(Maxwell): Enqueuing joinable session still requires using matchmaking APIs, however we are now querying session manager.
	// Splitgate doesn't need this so I am commenting it out, but maybe we'll need to figure out a way to have session manager enqueue?
	//FRegistry::ServerMatchmaking.EnqueueJoinableSession(ABSessionInfo->SessionResult, OnEnqueueJoinableSessionSuccessDelegate, OnEnqueueJoinableSessionErrorDelegate);

	// #TODO(Maxwell): Remove when we solve the above issue...
	UE_LOG_AB(Warning, TEXT("Skipping joinable session enqueue as session manager doesn't support enqueuing joinable."));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to enqueue joinable session on backend."));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableSession::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		Session->SessionSettings.bAllowJoinInProgress = true;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableSession::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableSession::OnEnqueueJoinableSessionSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Successfully enqueued joinable session for '%s' to DSM!"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableSession::OnEnqueueJoinableSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to enqueue joinable session for '%s' to DSM! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
