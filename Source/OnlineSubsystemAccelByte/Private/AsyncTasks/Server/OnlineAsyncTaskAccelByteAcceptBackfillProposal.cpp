// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAcceptBackfillProposal.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteAcceptBackfillProposal::FOnlineAsyncTaskAccelByteAcceptBackfillProposal(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& InProposal, FAccelByteModelsV2MatchmakingBackfillAcceptanceOptionalParam const& InOptionalParameter, bool bInStopBackfilling, const FOnAcceptBackfillProposalComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionName(InSessionName)
	, Proposal(InProposal)
	, OptionalParameter(InOptionalParameter)
	, bStopBackfilling(bInStopBackfilling)
	, Delegate(InDelegate)
	, GameSessionInfo()
{
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; BackfillTicketId: %s; ProposalId: %s"), *SessionName.ToString(), *Proposal.BackfillTicketID, *Proposal.ProposalID);

	AccelByte::FServerApiClientPtr ServerApiClient = AccelByte::FMultiRegistry::GetServerApiClient();
	AB_ASYNC_TASK_VALIDATE(ServerApiClient.IsValid(), "Failed to accept backfill proposal for session as we could not get a server API client!");

	OnAcceptBackfillProposalSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2GameSession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAcceptBackfillProposal::OnAcceptBackfillProposalSuccess);
	OnAcceptBackfillProposalErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAcceptBackfillProposal::OnAcceptBackfillProposalError);;
	ServerApiClient->ServerMatchmakingV2.AcceptBackfillProposal(Proposal.BackfillTicketID, Proposal.ProposalID, bStopBackfilling, OnAcceptBackfillProposalSuccessDelegate, OnAcceptBackfillProposalErrorDelegate, OptionalParameter);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// If we successfully accepted the proposal and we are removing this backfill ticket, then update session settings to
		// remove there.
		FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
		if (!ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session interface instance from online subsystem!"));
			return;
		}

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		if (!ensureAlways(Session != nullptr))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session instance to update stored backfill ticket ID!"));
			return;
		}

		if (bStopBackfilling)
		{
			Session->SessionSettings.Remove(SETTING_MATCHMAKING_BACKFILL_TICKET_ID);
		}

		// We don't care about this out flag in this case
		bool bIsConnectingToP2P;
		SessionInterface->UpdateInternalGameSession(SessionName, GameSessionInfo, bIsConnectingToP2P);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsDSBackfillProposalAcceptedPayload DSBackfillProposalAcceptedPayload{};
			DSBackfillProposalAcceptedPayload.PodName = AccelByte::FRegistry::ServerDSM.GetServerName();
			DSBackfillProposalAcceptedPayload.BackfillTicketId = GameSessionInfo.BackfillTicketID;
			DSBackfillProposalAcceptedPayload.ProposalId = Proposal.ProposalID;
			DSBackfillProposalAcceptedPayload.MatchPool = GameSessionInfo.MatchPool;
			DSBackfillProposalAcceptedPayload.GameSessionId = GameSessionInfo.ID;
			DSBackfillProposalAcceptedPayload.ProposedTeams = Proposal.ProposedTeams;
			DSBackfillProposalAcceptedPayload.AddedTickets = Proposal.AddedTickets;
			PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSBackfillProposalAcceptedPayload>(DSBackfillProposalAcceptedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::OnAcceptBackfillProposalSuccess(const FAccelByteModelsV2GameSession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	GameSessionInfo = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::OnAcceptBackfillProposalError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Request to accept backfill proposal failed on backend!", ErrorCode, ErrorMessage);
}
