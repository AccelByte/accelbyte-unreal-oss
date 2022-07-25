// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJoinV2Party.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/AccelByteRegistry.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteJoinV2Party::FOnlineAsyncTaskAccelByteJoinV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteJoinV2Party::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
	ensure(JoinedSession != nullptr);

	THandler<FAccelByteModelsV2PartySession> OnJoinPartySuccessDelegate = THandler<FAccelByteModelsV2PartySession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteJoinV2Party::OnJoinPartySuccess);
	FErrorHandler OnJoinPartyErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteJoinV2Party::OnJoinPartyError);
	ApiClient->Session.JoinParty(JoinedSession->GetSessionIdStr(), OnJoinPartySuccessDelegate, OnJoinPartyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV2Party::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	if (bWasSuccessful)
	{
		// We successfully joined on backend, thus reflect that state here!
		FNamedOnlineSession* JoinedSession = SessionInterface->GetNamedSession(SessionName);
		ensure(JoinedSession != nullptr);

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(JoinedSession->SessionInfo);
		ensure(SessionInfo.IsValid());

		SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(PartyInfo));
		JoinedSession->SessionState = EOnlineSessionState::Pending;

		// Since we've joined the party, we want to go through each member that is marked as joined to the party and make calls
		// to register them to the session
		TArray<FUniqueNetIdRef> PlayersToRegister{};
		for (const FAccelByteModelsV2SessionUser& Member : PartyInfo.Members)
		{
			if (Member.Status != EAccelByteV2SessionMemberStatus::JOINED)
			{
				continue;
			}

			FAccelByteUniqueIdComposite CompositeId;
			CompositeId.Id = Member.ID;
			CompositeId.PlatformType = Member.PlatformID;
			CompositeId.PlatformId = Member.PlatformUserID;

			TSharedPtr<const FUniqueNetIdAccelByteUser> PlayerId = FUniqueNetIdAccelByteUser::Create(CompositeId);
			ensure(PlayerId.IsValid());

			PlayersToRegister.Emplace(PlayerId.ToSharedRef());
		}

		SessionInterface->RegisterPlayers(SessionName, PlayersToRegister, false);

		// Additionally, pass to the session interface to remove any restored session instance that we were tracking for this
		// session, if any exists.
		SessionInterface->RemoveRestoreSessionById(JoinedSession->GetSessionIdStr());

		// Also, try and remove the party invite from our list of invites.
		SessionInterface->RemoveInviteById(JoinedSession->GetSessionIdStr());
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
	ensure(SessionInterface.IsValid());

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
	UE_LOG_AB(Warning, TEXT("Failed to join party session as the request failed on the backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	JoinSessionResult = EOnJoinSessionCompleteResult::UnknownError; // #TODO Flesh out this error so developers know exactly what went wrong with join
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
