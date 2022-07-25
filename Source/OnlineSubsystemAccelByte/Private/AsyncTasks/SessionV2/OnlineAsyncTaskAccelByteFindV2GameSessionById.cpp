// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindV2GameSessionById.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteFindV2GameSessionById::FOnlineAsyncTaskAccelByteFindV2GameSessionById(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const FUniqueNetId& InSessionId, const FOnSingleSessionResultCompleteDelegate& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionId(StaticCastSharedRef<const FUniqueNetIdAccelByteResource>(InSessionId.AsShared()))
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InSearchingPlayerId.AsShared());
}

void FOnlineAsyncTaskAccelByteFindV2GameSessionById::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId->ToDebugString());

	const THandler<FAccelByteModelsV2GameSession> OnGetGameSessionDetailsSuccessDelegate = THandler<FAccelByteModelsV2GameSession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteFindV2GameSessionById::OnGetGameSessionDetailsSuccess);
	const FErrorHandler OnGetGameSessionDetailsErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteFindV2GameSessionById::OnGetGameSessionDetailsError);
	ApiClient->Session.GetGameSessionDetails(SessionId->ToString(), OnGetGameSessionDetailsSuccessDelegate, OnGetGameSessionDetailsErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2GameSessionById::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		ensure(SessionInterface.IsValid());

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
