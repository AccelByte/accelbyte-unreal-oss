// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJoinV2PartyByCode.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteJoinV2PartyByCode::FOnlineAsyncTaskAccelByteJoinV2PartyByCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InPartyCode)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, PartyCode(InPartyCode)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteJoinV2PartyByCode::Initialize()
{
		TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s; PartyCode: %s"), *UserId->ToDebugString(), *SessionName.ToString(), *PartyCode);

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_VALIDATE(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(),  SessionInterface), "Failed to join party by code as our session interface instance is invalid!");

	FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(JoinedSession != nullptr, "Failed to join party by code as the session that we are trying to join for is invalid!");

	OnJoinPartyByCodeSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV2PartyByCode::OnJoinPartyByCodeSuccess);
	OnJoinPartyByCodeErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV2PartyByCode::OnJoinPartyByCodeError);;
	API_FULL_CHECK_GUARD(Session);
	Session->JoinPartyByCode(PartyCode, OnJoinPartyByCodeSuccessDelegate, OnJoinPartyByCodeErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2PartyByCode::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Regardless of whether we were successful in joining this party by code or not, destroy the previously created named
	// session instance. If successful, a new one will be made in subsequent calls.
	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (!FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(),  SessionInterface))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize task to join party by code as our session interface is invalid!"));
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
	if (!SessionInterface->ConstructPartySessionFromBackendSessionModel(JoinedPartySession, ConstructedSession))
	{
		return;
	}

	FOnlineSessionSearchResult JoinResult{};
	JoinResult.Session = ConstructedSession;
	SessionInterface->JoinSession(UserId.ToSharedRef().Get(), SessionName, JoinResult);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2PartySessionJoinedPayload PartySessionJoinedPayload{};
		PartySessionJoinedPayload.UserId = UserId->GetAccelByteId();
		PartySessionJoinedPayload.PartySessionId = JoinResult.GetSessionIdStr();
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionJoinedPayload>(PartySessionJoinedPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2PartyByCode::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// Don't execute the join delegate here, as the call to JoinSession should eventually trigger that delegate for listeners
		return;
	}

	// In the event of an error trying to join by code, notify the caller through the join session delegate that we failed to join
	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (!FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(),  SessionInterface))
	{
		return;
	}

	SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, JoinSessionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2PartyByCode::OnJoinPartyByCodeSuccess(const FAccelByteModelsV2PartySession& PartySession)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("JoinedID: %s"), *PartySession.ID);

	JoinSessionResult = EOnJoinSessionCompleteResult::Success;
	JoinedPartySession = PartySession;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2PartyByCode::OnJoinPartyByCodeError(int32 ErrorCode, const FString& ErrorMessage)
{
	switch(ErrorCode)
	{
		case static_cast<int32>(ErrorCodes::SessionJoinSessionFull):
			JoinSessionResult = EOnJoinSessionCompleteResult::SessionIsFull;
			break;
		// intended fallthrough for BE backward compatibility, old BE use this code when session not exist
		case static_cast<int32>(ErrorCodes::SessionGameNotFound):
		case static_cast<int32>(ErrorCodes::SessionPartyNotFound):
			JoinSessionResult = EOnJoinSessionCompleteResult::SessionDoesNotExist;
			break;
		default:
			JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
	}
	
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to join party using code '%s'!", ErrorCode, ErrorMessage, *PartyCode);
}
