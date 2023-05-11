#include "OnlineAsyncTaskAccelByteFindV1GameSessionById.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

FOnlineAsyncTaskAccelByteFindV1GameSessionById::FOnlineAsyncTaskAccelByteFindV1GameSessionById(
	FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId,
	const FUniqueNetId& InSessionId, const FOnSingleSessionResultCompleteDelegate& InDelegate):
	FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, (IsRunningDedicatedServer()) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
	, SessionId(FUniqueNetIdAccelByteResource::CastChecked(InSessionId))
	, Delegate(InDelegate)
{
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingPlayerId.AsShared());
	}
}

void FOnlineAsyncTaskAccelByteFindV1GameSessionById::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId->ToDebugString());

	// Send the API call based on whether we are a server or a client
	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteFindV1GameSessionById, GetGameSessionDetails, THandler<FAccelByteModelsSessionBrowserData>);
	if (IsRunningDedicatedServer())
	{
		FRegistry::ServerSessionBrowser.GetGameSessionBySessionId(SessionId->ToString(), OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate);
	}
	else
	{
		ApiClient->SessionBrowser.GetGameSession(SessionId->ToString(), OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate );
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1GameSessionById::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize finding a game session by ID as our session interface is invalid!"));
			return;
		}

		SessionInterface->ConstructGameSessionFromBackendSessionModel(FoundGameSession, FoundSessionResult.Session);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1GameSessionById::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, FoundSessionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1GameSessionById::OnGetGameSessionDetailsSuccess(
	const FAccelByteModelsSessionBrowserData& InFoundGameSession)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FoundGameSession = InFoundGameSession;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1GameSessionById::OnGetGameSessionDetailsError(int32 ErrorCode,
	const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to find game session with ID '%s'! Error code: %d; Error message: %s"), *SessionId->ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
