// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJoinV2GameSessionByCode.h"

FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode::FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InPartyCode)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Code(InPartyCode)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s; PartyCode: %s"), *UserId->ToDebugString(), *SessionName.ToString(), *Code);

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_ENSURE(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface), "Failed to join party by code as our session interface instance is invalid!");

	FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_ENSURE(JoinedSession != nullptr, "Failed to join party by code as the session that we are trying to join for is invalid!");

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode, JoinGameSessionByCode, THandler<FAccelByteModelsV2GameSession>);
	ApiClient->Session.JoinGameSessionByCode(Code, OnJoinGameSessionByCodeSuccessDelegate, OnJoinGameSessionByCodeErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Regardless of whether we were successful in joining this party by code or not, destroy the previously created named
	// session instance. If successful, a new one will be made in subsequent calls.
	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (!FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize joining a game session by code as our session interface is invalid!"));
		bWasSuccessful = false;
		JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
		
		return;
	}

	SessionInterface->RemoveNamedSession(SessionName);

	if (!bWasSuccessful)
	{
		return;
	}

	// Hacky, but this was an easy way to handle the party join without having to rewrite a lot of logic. Basically, we want
	// to take the information that we just retrieved from the join by code, and turn it into a FOnlineSessionSearchResult.
	// Then, we just pass that to the real JoinSession, which then will process the data and in turn create the final
	// FNamedOnlineSession. This unfortunately does make an extra call to the join endpoint itself, however this endpoint
	// is no-op when you are already in the session you are attempting to join, so shouldn't be too huge of an issue.
	FOnlineSession ConstructedSession{};
	if (!SessionInterface->ConstructGameSessionFromBackendSessionModel(JoinedGameSession, ConstructedSession))
	{
		return;
	}

	FOnlineSessionSearchResult JoinResult{};
	JoinResult.Session = ConstructedSession;
	SessionInterface->JoinSession(UserId.ToSharedRef().Get(), SessionName, JoinResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// Don't execute the join delegate here, as the call to JoinSession should eventually trigger that delegate for listeners
		return;
	}

	// In the event of an error trying to join by code, notify the caller through the join session delegate that we failed to join
	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (!FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface))
	{
		return;
	}

	SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, JoinSessionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode::OnJoinGameSessionByCodeSuccess(const FAccelByteModelsV2GameSession& GameSession)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("JoinedID: %s"), *GameSession.ID);

	JoinSessionResult = EOnJoinSessionCompleteResult::Success;
	JoinedGameSession = GameSession;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode::OnJoinGameSessionByCodeError(int32 ErrorCode, const FString& ErrorMessage)
{
	switch(ErrorCode)
	{
		case static_cast<int32>(AccelByte::ErrorCodes::SessionJoinSessionFull):
			JoinSessionResult = EOnJoinSessionCompleteResult::SessionIsFull;
			break;
		case static_cast<int32>(AccelByte::ErrorCodes::SessionGameNotFound):
			JoinSessionResult = EOnJoinSessionCompleteResult::SessionDoesNotExist;
			break;
		default:
			JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
	}
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to join party using code '%s'!", ErrorCode, ErrorMessage, *Code);
}
