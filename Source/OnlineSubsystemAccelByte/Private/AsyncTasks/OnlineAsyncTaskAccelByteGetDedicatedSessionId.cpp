// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetDedicatedSessionId.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Core/AccelByteRegistry.h"
#include "GameServerApi/AccelByteServerDSMApi.h"

FOnlineAsyncTaskAccelByteGetDedicatedSessionId::FOnlineAsyncTaskAccelByteGetDedicatedSessionId(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
{
}

void FOnlineAsyncTaskAccelByteGetDedicatedSessionId::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	check(SessionInterface != nullptr);

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	check(Session != nullptr);

	// First check if we already have a session ID attached to the session information, if so, we don't need to do anything else
	if (Session->SessionInfo.IsValid() && Session->SessionInfo->GetSessionId().IsValid())
	{
		SessionId = Session->SessionInfo->GetSessionId().ToString();
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	// If we already have a session ID from an environment variable, skip making a request and use that ID
	SessionId = FPlatformMisc::GetEnvironmentVariable(TEXT("NOMAD_META_session_id"));
	if (!SessionId.IsEmpty())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	// Do the same if we have a session ID from the command line
	FParse::Value(FCommandLine::Get(), TEXT("SessionId="), SessionId);
	if (!SessionId.IsEmpty())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	// Otherwise, make the request to get session ID for this server from the DSMC
	THandler<FAccelByteModelsServerSessionResponse> OnGetSessionIdSuccess = THandler<FAccelByteModelsServerSessionResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetDedicatedSessionId::OnGetSessionIdSuccess);
	FErrorHandler OnGetSessionIdError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteGetDedicatedSessionId::OnGetSessionIdError);
	FRegistry::ServerDSM.GetSessionId(OnGetSessionIdSuccess, OnGetSessionIdError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetDedicatedSessionId::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		StaticCastSharedPtr<FOnlineSessionInfoAccelByte>(Session->SessionInfo)->SetSessionId(SessionId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetDedicatedSessionId::OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; SessionId: %s"), *SessionName.ToString(), *Result.Session_id);

	SessionId = Result.Session_id;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Found session ID '%s'! Querying status of this session from backend!"), *SessionId);
}

void FOnlineAsyncTaskAccelByteGetDedicatedSessionId::OnGetSessionIdError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get session ID from backend for session '%s'! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
}
