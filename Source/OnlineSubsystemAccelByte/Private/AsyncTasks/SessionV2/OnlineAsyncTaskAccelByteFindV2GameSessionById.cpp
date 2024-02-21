// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindV2GameSessionById.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteFindV2GameSessionById::FOnlineAsyncTaskAccelByteFindV2GameSessionById(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const FUniqueNetId& InSessionId, const FOnSingleSessionResultCompleteDelegate& InDelegate)
	// Initialize as a server task if we are running a dedicated server, as this doubles as a server task. Otherwise, use
	// no flags to indicate that it is a client task.
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, (IsRunningDedicatedServer()) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
	, SessionId(FUniqueNetIdAccelByteResource::CastChecked(InSessionId))
	, Delegate(InDelegate)
{
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingPlayerId.AsShared());
	}
}

void FOnlineAsyncTaskAccelByteFindV2GameSessionById::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId->ToDebugString());

	// Send the API call based on whether we are a server or a client
	OnGetGameSessionDetailsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2GameSession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteFindV2GameSessionById::OnGetGameSessionDetailsSuccess);
	OnGetGameSessionDetailsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteFindV2GameSessionById::OnGetGameSessionDetailsError);;
	if (IsRunningDedicatedServer())
	{
		FRegistry::ServerSession.GetGameSessionDetails(SessionId->ToString(), OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate);
	}
	else
	{
		if (ApiClient.IsValid())
		{
			ApiClient->Session.GetGameSessionDetails(SessionId->ToString(), OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate);
		}
		else
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2GameSessionById::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize finding a game session by ID as our session interface is invalid!"));
			return;
		}

		SessionInterface->ConstructGameSessionFromBackendSessionModel(FoundGameSession, FoundSessionResult.Session);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2GameSessionById::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, FoundSessionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2GameSessionById::OnGetGameSessionDetailsSuccess(const FAccelByteModelsV2GameSession& InFoundGameSession)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FoundGameSession = InFoundGameSession;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2GameSessionById::OnGetGameSessionDetailsError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to find game session with ID '%s'! Error code: %d; Error message: %s"), *SessionId->ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
