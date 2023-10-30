// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRejectBackfillProposal.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteRejectBackfillProposal::FOnlineAsyncTaskAccelByteRejectBackfillProposal(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& InProposal, bool bInStopBackfilling, const FOnRejectBackfillProposalComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionName(InSessionName)
	, Proposal(InProposal)
	, bStopBackfilling(bInStopBackfilling)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteRejectBackfillProposal::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; BackfillTicketId: %s; ProposalId: %s"), *SessionName.ToString(), *Proposal.BackfillTicketID, *Proposal.ProposalID);

	AccelByte::FServerApiClientPtr ServerApiClient = AccelByte::FMultiRegistry::GetServerApiClient();
	AB_ASYNC_TASK_ENSURE(ServerApiClient.IsValid(), "Failed to reject backfill proposal for session as we could not get a server API client!");

	OnRejectBackfillProposalSuccessDelegate = AccelByte::TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRejectBackfillProposal::OnRejectBackfillProposalSuccess);
	OnRejectBackfillProposalErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRejectBackfillProposal::OnRejectBackfillProposalError);;
	ServerApiClient->ServerMatchmakingV2.RejectBackfillProposal(Proposal.BackfillTicketID, Proposal.ProposalID, bStopBackfilling, OnRejectBackfillProposalSuccessDelegate, OnRejectBackfillProposalErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectBackfillProposal::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		if (bStopBackfilling)
		{
			// If we successfully rejected the proposal and we are removing this backfill ticket, then update session settings to
			// remove there.
			FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
			if (!ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface)))
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

			Session->SessionSettings.Remove(SETTING_MATCHMAKING_BACKFILL_TICKET_ID);
		}

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsDSBackfillProposalRejectedPayload DSBackfillProposalRejectedPayload{};
			DSBackfillProposalRejectedPayload.PodName = AccelByte::FRegistry::ServerDSM.GetServerName();
			DSBackfillProposalRejectedPayload.BackfillTicketId = Proposal.BackfillTicketID;
			DSBackfillProposalRejectedPayload.ProposalId = Proposal.ProposalID;
			DSBackfillProposalRejectedPayload.MatchPool = Proposal.MatchPool;
			DSBackfillProposalRejectedPayload.GameSessionId = Proposal.MatchSessionID;
			DSBackfillProposalRejectedPayload.ProposedTeams = Proposal.ProposedTeams;
			DSBackfillProposalRejectedPayload.AddedTickets = Proposal.AddedTickets;
			PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSBackfillProposalRejectedPayload>(DSBackfillProposalRejectedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectBackfillProposal::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectBackfillProposal::OnRejectBackfillProposalSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectBackfillProposal::OnRejectBackfillProposalError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Request to reject backfill proposal failed on backend!", ErrorCode, ErrorMessage);
}
