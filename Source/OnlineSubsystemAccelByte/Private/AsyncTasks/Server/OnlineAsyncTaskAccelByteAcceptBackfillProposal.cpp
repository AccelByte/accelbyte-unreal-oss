// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAcceptBackfillProposal.h"

FOnlineAsyncTaskAccelByteAcceptBackfillProposal::FOnlineAsyncTaskAccelByteAcceptBackfillProposal(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& InProposal, bool bInStopBackfilling, const FOnAcceptBackfillProposalComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionName(InSessionName)
	, Proposal(InProposal)
	, bStopBackfilling(bInStopBackfilling)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; BackfillTicketId: %s; ProposalId: %s"), *SessionName.ToString(), *Proposal.BackfillTicketID, *Proposal.ProposalID);

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteAcceptBackfillProposal, AcceptBackfillProposal, FVoidHandler);
	FRegistry::ServerMatchmakingV2.AcceptBackfillProposal(Proposal.BackfillTicketID, Proposal.ProposalID, bStopBackfilling, OnAcceptBackfillProposalSuccessDelegate, OnAcceptBackfillProposalErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::OnAcceptBackfillProposalSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptBackfillProposal::OnAcceptBackfillProposalError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Request to accept backfill proposal failed on backend!", ErrorCode, *ErrorMessage);
}
