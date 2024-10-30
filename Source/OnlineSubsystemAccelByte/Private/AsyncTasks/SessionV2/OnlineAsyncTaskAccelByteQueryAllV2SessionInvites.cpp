// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryAllV2SessionInvites.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PlayerId: %s"), *UserId->ToDebugString());

	OnGetGameSessionInvitesSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetGameSessionInvitesSuccess);
	OnGetGameSessionInvitesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetGameSessionInvitesError);
	API_CLIENT_CHECK_GUARD();
	ApiClient->Session.GetMyGameSessions(OnGetGameSessionInvitesSuccessDelegate, OnGetGameSessionInvitesErrorDelegate, EAccelByteV2SessionMemberStatus::INVITED);

	OnGetPartySessionInvitesSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedPartyQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetPartySessionInvitesSuccess);
	OnGetPartySessionInvitesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetPartySessionInvitesError);
	ApiClient->Session.GetMyParties(OnGetPartySessionInvitesSuccessDelegate, OnGetPartySessionInvitesErrorDelegate, EAccelByteV2SessionMemberStatus::INVITED);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::Tick()
{
	Super::Tick();

	if (bHasReceivedGameSessionInviteResponse && bHasReceivedPartySessionInviteResponse)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize querying all session invites as our session interface is invalid!"));
			return;
		}

		// Invites that are already in the session interface are stale by this point, so just clear and move over our new array
		SessionInterface->SessionInvites.Empty();
		SessionInterface->SessionInvites = TArray<FOnlineSessionInviteAccelByte>(MoveTemp(Invites));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for querying all session invites as our session interface is invalid!"));
		return;
	}

	SessionInterface->TriggerOnQueryAllInvitesCompleteDelegates(bWasSuccessful, UserId.ToSharedRef().Get());
	SessionInterface->TriggerOnInviteListUpdatedDelegates();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetGameSessionInvitesSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result)
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GameSessionInviteCount: %d"), Result.Data.Num());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to construct game session invite instances as our session interface is invalid!");

	for (const FAccelByteModelsV2GameSession& Session : Result.Data)
	{
		FOnlineSessionInviteAccelByte Invite{};
		Invite.SessionType = EAccelByteV2SessionType::GameSession;
		Invite.RecipientId = UserId;

		if (!SessionInterface->ConstructGameSessionFromBackendSessionModel(Session, Invite.Session.Session))
		{
			UE_LOG_AB(Warning, TEXT("Failed to construct session result for invite to session '%s'!"), *Session.ID);
		}

		// #TODO #SESSIONv2 Need a way to get the ID of the user who invited you to a session in the REST API

		Invites.Emplace(Invite);
	}

	bHasReceivedGameSessionInviteResponse = true;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetGameSessionInvitesError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get game session invites from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetPartySessionInvitesSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result)
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PartySessionInviteCount: %d"), Result.Data.Num());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to construct party session invite instances as our session interface is invalid!");

	for (const FAccelByteModelsV2PartySession& Session : Result.Data)
	{
		FOnlineSessionInviteAccelByte Invite{};
		Invite.SessionType = EAccelByteV2SessionType::PartySession;
		Invite.RecipientId = UserId;

		if (!SessionInterface->ConstructPartySessionFromBackendSessionModel(Session, Invite.Session.Session))
		{
			UE_LOG_AB(Warning, TEXT("Failed to construct session result for invite to session '%s'!"), *Session.ID);
		}

		// #TODO #SESSIONv2 Need a way to get the ID of the user who invited you to a session in the REST API

		Invites.Emplace(Invite);
	}

	bHasReceivedPartySessionInviteResponse = true;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites::OnGetPartySessionInvitesError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get party session invites from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
