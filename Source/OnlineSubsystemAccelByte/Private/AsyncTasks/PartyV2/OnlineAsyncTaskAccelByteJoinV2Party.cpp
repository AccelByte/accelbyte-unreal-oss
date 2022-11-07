// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJoinV2Party.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/AccelByteRegistry.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteJoinV2Party::FOnlineAsyncTaskAccelByteJoinV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, bool bInIsRestoreSession)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, bIsRestoreSession(bInIsRestoreSession)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteJoinV2Party::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to join party as our session interface instance is invalid!");

	FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_ENSURE(JoinedSession != nullptr, "Failed to join party as the session that we are trying to join for is invalid!");

	const FString SessionId = JoinedSession->GetSessionIdStr();
	AB_ASYNC_TASK_ENSURE(!SessionId.Equals(TEXT("InvalidSession")), "Failed to join party as the session we are trying to join has an invalid ID!");

	// If we are just restoring the session, then we just want to get the up to date session details and construct the
	// session as normal. Otherwise, we want to go through the actual join flow.
	if (bIsRestoreSession)
	{
		AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteJoinV2Party, GetPartyDetails, THandler<FAccelByteModelsV2PartySession>);
		ApiClient->Session.GetPartyDetails(SessionId, OnGetPartyDetailsSuccessDelegate, OnGetPartyDetailsErrorDelegate);
	}
	else
	{
		AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteJoinV2Party, JoinParty, THandler<FAccelByteModelsV2PartySession>);
		ApiClient->Session.JoinParty(SessionId, OnJoinPartySuccessDelegate, OnJoinPartyErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2Party::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize task to join party as our session interface is invalid!"));
		return;
	}

	if (bWasSuccessful)
	{
		// We successfully joined on backend, thus reflect that state here!
		FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
		if (!ensure(JoinedSession != nullptr))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join party as our session instance to write data to is not valid!"));
			return;
		}

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(JoinedSession->SessionInfo);
		if (!ensure(SessionInfo.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join party as our session's information instance is invalid!"));
			return;
		}

		SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(PartyInfo));
		JoinedSession->SessionState = EOnlineSessionState::Pending;

		// This will seem pretty silly, but take the open slots for the session and set them to the max number of slots. This
		// way registering and unregistering throughout the lifetime of the session will show proper counts.
		//
		// #NOTE Party sessions only have closed slots, so just use the private connection count
		JoinedSession->NumOpenPrivateConnections = JoinedSession->SessionSettings.NumPrivateConnections;

		// Register all members marked as joined to the session
		SessionInterface->RegisterPlayers(SessionName, SessionInfo->GetJoinedMembers(), false);

		const FString SessionId = JoinedSession->GetSessionIdStr();

		// Additionally, pass to the session interface to remove any restored session instance that we were tracking for this
		// session, if any exists.
		SessionInterface->RemoveRestoreSessionById(SessionId);

		// Also, try and remove the party invite from our list of invites.
		SessionInterface->RemoveInviteById(SessionId);
	}
	else
	{
		// Remove pending session in session interface so that developer can retry joining, or create a new session
		SessionInterface->RemoveNamedSession(SessionName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2Party::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
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
	JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError; // #TODO #SESSIONv2 Maybe expand this to use a better error later?
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to join party on backend!", ErrorCode, ErrorMessage);
}

void FOnlineAsyncTaskAccelByteJoinV2Party::OnGetPartyDetailsSuccess(const FAccelByteModelsV2PartySession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PartyId: %s"), *Result.ID);

	PartyInfo = Result;
	JoinSessionResult = EOnJoinSessionCompleteResult::Success;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2Party::OnGetPartyDetailsError(int32 ErrorCode, const FString& ErrorMessage)
{
	JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError; // #TODO #SESSIONv2 Maybe expand this to use a better error later?
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to restored joined party on backend!", ErrorCode, ErrorMessage);
}
