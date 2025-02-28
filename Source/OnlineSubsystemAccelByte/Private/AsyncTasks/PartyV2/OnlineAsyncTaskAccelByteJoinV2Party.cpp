// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJoinV2Party.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/AccelByteRegistry.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteJoinV2Party::FOnlineAsyncTaskAccelByteJoinV2Party(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InLocalUserId
	, const FName& InSessionName
	, bool bInHasLocalUserJoined)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, bHasLocalUserJoined(bInHasLocalUserJoined)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteJoinV2Party::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to join party as our session interface instance is invalid!");

	FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(JoinedSession != nullptr, "Failed to join party as the session that we are trying to join for is invalid!");

	const FString SessionId = JoinedSession->GetSessionIdStr();
	AB_ASYNC_TASK_VALIDATE(!SessionId.Equals(TEXT("InvalidSession")), "Failed to join party as the session we are trying to join has an invalid ID!");

	if (bHasLocalUserJoined)
	{
		// Local player has already joined this session as far as the backend is concerned. With that in mind, grab the
		// session data from the pending named session created and manually call OnJoinPartySuccess with that data.
		// That way, we skip the unnecessary JoinParty API call and still go through normal flow of setting up
		// the local session cache with proper session data. If some how we do not have valid data for the pending named
		// session, then we will fall back to doing the join party API call to retrieve that data. Session service will
		// return a no-op success response if we do end up calling while joined.
		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(JoinedSession->SessionInfo);
		if (SessionInfo.IsValid())
		{
			TSharedPtr<FAccelByteModelsV2PartySession> PartySessionData = SessionInfo->GetBackendSessionDataAsPartySession();
			if (PartySessionData.IsValid())
			{
				OnJoinPartySuccess(PartySessionData.ToSharedRef().Get());
				return;
			}
		}

		// No early return here as we want to flow into the join API call if the above conditions are not met
	}

	OnJoinPartySuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV2Party::OnJoinPartySuccess);
	OnJoinPartyErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV2Party::OnJoinPartyError);
	API_FULL_CHECK_GUARD(Session);
	Session->JoinParty(SessionId, OnJoinPartySuccessDelegate, OnJoinPartyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2Party::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize task to join party as our session interface is invalid!"));
		bWasSuccessful = false;
		JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
		return;
	}

	if (bWasSuccessful)
	{
		// We successfully joined on backend, thus reflect that state here!
		FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
		if (!ensure(JoinedSession != nullptr))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join party as our session instance to write data to is not valid!"));
			bWasSuccessful = false;
			JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError;
			return;
		}

		JoinedSession->SessionState = EOnlineSessionState::Pending;

		// This will seem pretty silly, but take the open slots for the session and set them to the max number of slots. This
		// way registering and unregistering throughout the lifetime of the session will show proper counts.
		//
		// #NOTE Party sessions only have closed slots, so just use the private connection count
		JoinedSession->NumOpenPrivateConnections = JoinedSession->SessionSettings.NumPrivateConnections;

		// Remove any restored session instance or invite for this joined session so that it does not linger
		const FString SessionId = JoinedSession->GetSessionIdStr();
		SessionInterface->RemoveRestoreSessionById(SessionId);
		SessionInterface->RemoveInviteById(SessionId);

		// Update the session data in the session interface with the data we received on join, that way if any updates
		// occured between query and join we would catch them
		SessionInterface->UpdateInternalPartySession(SessionName, PartyInfo);

		// Set the party attribute with past sesssion info from the current user
		SessionInterface->UpdatePartySessionStorageWithPastSessionInfo(UserId);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsMPV2PartySessionJoinedPayload PartySessionJoinedPayload{};
			PartySessionJoinedPayload.UserId = UserId->GetAccelByteId();
			PartySessionJoinedPayload.PartySessionId = JoinedSession->GetSessionIdStr();
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionJoinedPayload>(PartySessionJoinedPayload));
		}
	}
	else
	{
		// Retrieve the pending joined session so that we can remove any pending invites or restore sessions by ID
		FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
		if (!ensure(JoinedSession != nullptr))
		{
			return;
		}

		const FString SessionId = JoinedSession->GetSessionIdStr();
		SessionInterface->RemoveRestoreSessionById(SessionId);
		SessionInterface->RemoveInviteById(SessionId);

		// Remove pending session in session interface so that developer can retry joining, or create a new session
		SessionInterface->RemoveNamedSession(SessionName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2Party::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for joining a party as our session interface is invalid!"));
		return;
	}

	SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, JoinSessionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteJoinV2Party::OnJoinPartySuccess(const FAccelByteModelsV2PartySession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PartyId: %s"), *Result.ID);

	PartyInfo = Result;
	JoinSessionResult = EOnJoinSessionCompleteResult::Success;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2Party::OnJoinPartyError(int32 ErrorCode, const FString& ErrorMessage)
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
	
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to join party on backend!", ErrorCode, ErrorMessage);
}
