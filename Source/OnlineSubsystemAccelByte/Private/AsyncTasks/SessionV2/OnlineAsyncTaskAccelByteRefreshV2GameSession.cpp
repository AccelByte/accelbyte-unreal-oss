// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRefreshV2GameSession.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Api/AccelByteSessionApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRefreshV2GameSession::FOnlineAsyncTaskAccelByteRefreshV2GameSession(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InLocalUserId
	, const FName& InSessionName
	, const FOnRefreshSessionComplete& InDelegate
	, bool IsDedicatedServer /* = false */)
	// Initialize as a server task if we are running a server task, as this doubles as a server task. Otherwise, use no flags 
	: FOnlineAsyncTaskAccelByte(InABInterface
		, INVALID_CONTROLLERID
		, IsDedicatedServer ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	TRY_PIN_SUBSYSTEM_CONSTRUCTOR()

	if (!IsDedicatedServer)
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	TRY_PIN_SUBSYSTEM();

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	check(SessionInterface.IsValid());

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Could not refresh game session named '%s' as the session does not exist locally!", *SessionName.ToString());

	const FString SessionId = OnlineSession->GetSessionIdStr();
	AB_ASYNC_TASK_VALIDATE(!SessionId.Equals(TEXT("InvalidSession")), "Could not refresh game session named '%s' as there is not a valid session ID associated!", *SessionName.ToString());

	// Send the API call based on whether we are a server or a client
	OnRefreshGameSessionSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2GameSession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshV2GameSession::OnRefreshGameSessionSuccess);
	OnRefreshGameSessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshV2GameSession::OnRefreshGameSessionError);;
	if (IsDS.GetValue())
	{
		SERVER_API_CLIENT_CHECK_GUARD();
		ServerApiClient->ServerSession.GetGameSessionDetails(SessionId, OnRefreshGameSessionSuccessDelegate, OnRefreshGameSessionErrorDelegate);
	}
	else
	{
		API_FULL_CHECK_GUARD(Session);
		Session->GetGameSessionDetails(SessionId, OnRefreshGameSessionSuccessDelegate, OnRefreshGameSessionErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		if (bWasSessionRemoved)
		{
			SessionInterface->RemoveNamedSession(SessionName);
		}
		else
		{
			// We don't care about this out flag in this case
			bool bIsConnectingToP2P = false;
			SessionInterface->UpdateInternalGameSession(SessionName, RefreshedGameSession, bIsConnectingToP2P);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2GameSession::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
		if (ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
		{
			if (bWasSessionRemoved)
			{
				SessionInterface->TriggerOnSessionRemovedDelegates(SessionName);
			}
			else
			{
				SessionInterface->TriggerOnSessionUpdateReceivedDelegates(SessionName);
			}
		}
	}

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
	if (ErrorCode == static_cast<int32>(AccelByte::ErrorCodes::SessionGameNotFound))
	{
		// Game session was not found when we attempted to refresh, treat this as the session being removed
		UE_LOG_AB(Verbose, TEXT("Game session '%s' was not found when attempting to refresh, treating it as removed!"), *SessionName.ToString());
		bWasSessionRemoved = true;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	AB_ASYNC_TASK_REQUEST_FAILED("Request to refresh game session failed on backend!", ErrorCode, ErrorMessage);
}
