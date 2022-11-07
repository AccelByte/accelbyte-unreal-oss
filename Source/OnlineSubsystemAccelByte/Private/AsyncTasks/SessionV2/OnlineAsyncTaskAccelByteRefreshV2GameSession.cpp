// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRefreshV2GameSession.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Api/AccelByteSessionApi.h"

FOnlineAsyncTaskAccelByteRefreshV2GameSession::FOnlineAsyncTaskAccelByteRefreshV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnRefreshSessionComplete& InDelegate)
	// Initialize as a server task if we are running a server task, as this doubles as a server task. Otherwise, use no flags 
	: FOnlineAsyncTaskAccelByte(InABInterface, (IsRunningDedicatedServer()) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	if (!IsRunningDedicatedServer())
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (ensure(SessionInterface.IsValid()))
		{
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
			if (ensure(Session != nullptr))
			{
				UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(Session->LocalOwnerId);
			}
		}
	}
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	check(SessionInterface.IsValid());

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_ENSURE(Session != nullptr, "Could not refresh game session named '%s' as the session does not exist locally!", *SessionName.ToString());

	const FString SessionId = Session->GetSessionIdStr();
	AB_ASYNC_TASK_ENSURE(!SessionId.Equals(TEXT("InvalidSession")), "Could not refresh game session named '%s' as there is not a valid session ID associated!", *SessionName.ToString());

	// Send the API call based on whether we are a server or a client
	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteRefreshV2GameSession, RefreshGameSession, THandler<FAccelByteModelsV2GameSession>);
	if (IsRunningDedicatedServer())
	{
		FRegistry::ServerSession.GetGameSessionDetails(SessionId, OnRefreshGameSessionSuccessDelegate, OnRefreshGameSessionErrorDelegate);
	}
	else
	{
		ApiClient->Session.GetGameSessionDetails(SessionId, OnRefreshGameSessionSuccessDelegate, OnRefreshGameSessionErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		// We don't care about this out flag in this case
		bool bIsConnectingToP2P = false;
		SessionInterface->UpdateInternalGameSession(SessionName, RefreshedGameSession, bIsConnectingToP2P);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::OnRefreshGameSessionSuccess(const FAccelByteModelsV2GameSession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	RefreshedGameSession = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::OnRefreshGameSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Request to refresh game session failed on backend!", ErrorCode, ErrorMessage);
}
