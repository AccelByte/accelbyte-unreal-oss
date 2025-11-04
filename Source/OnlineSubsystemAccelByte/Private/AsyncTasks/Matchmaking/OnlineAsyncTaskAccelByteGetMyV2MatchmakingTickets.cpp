// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets.h"

#include "OnlineAsyncTaskAccelByteStartV2Matchmaking.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId, const FName InSessionName, const FString& InMatchPool)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, MatchPool(InMatchPool)
	, ActiveTicketId()
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

void FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalPlayerId: %s; MatchPool: %s"), *UserId->ToDebugString(), *MatchPool);

	SendGetMyTicketRequest();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bWasSuccessful)
	{
		// find the ticket that is still not matched from responses we get
		for (auto& MyTicketResult : MyTicketResults)
		{
			const auto UnmatchedTicket = MyTicketResult.Data.FindByPredicate([](FAccelByteModelsV2MatchmakingTicketStatus Ticket)
			{
				return !Ticket.MatchFound;
			});

			// found the unmatched ticket, break out of loop
			if(UnmatchedTicket != nullptr)
			{
				ActiveTicketId = UnmatchedTicket->MatchTicketID;
				break;
			}
		}

		if(ActiveTicketId.IsEmpty())
		{
			// unmatched ticket not found
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Active ticket not found"));
			return;
		}

		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize task for starting matchmaking as our session interface is invalid!"));
			return;
		}

		// Convert our session search handle into an AccelByte subclass so that we can store ticket ID
		SessionInterface->CurrentMatchmakingSearchHandle = MakeShared<FOnlineSessionSearchAccelByte>();
		SessionInterface->CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;
		SessionInterface->CurrentMatchmakingSearchHandle->SearchingPlayerId = UserId;
		SessionInterface->CurrentMatchmakingSearchHandle->SearchingSessionName = SessionName;
		SessionInterface->CurrentMatchmakingSearchHandle->MatchPool = MatchPool;
		SessionInterface->CurrentMatchmakingSearchHandle->TicketId = ActiveTicketId;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s, ActiveTicketId: %s"), *SessionName.ToString(), *ActiveTicketId);

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for starting matchmaking as our session interface is invalid!"));
		return;
	}

	SessionInterface->TriggerOnGetMyActiveMatchTicketCompleteDelegates(bWasSuccessful, SessionName, SessionInterface->CurrentMatchmakingSearchHandle);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::OnGetMyMatchTicketSuccess(
	const FAccelByteModelsV2MatchmakingTicketStatuses& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Matchpool: %s, at offset: %d"), *MatchPool, Offset);

	MyTicketResults.Emplace(Result);

	if(Result.Pagination.Next.IsEmpty())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	Offset += Limit;
	SendGetMyTicketRequest();
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::OnGetMyMatchTicketError(int32 ErrorCode,
	const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get my match tickets! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::SendGetMyTicketRequest()
{
	OnGetMyMatchTicketSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2MatchmakingTicketStatuses>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::OnGetMyMatchTicketSuccess);
	OnGetMyMatchTicketErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets::OnGetMyMatchTicketError);;

	API_FULL_CHECK_GUARD(MatchmakingV2);
	MatchmakingV2->GetMyMatchTickets(OnGetMyMatchTicketSuccessDelegate, OnGetMyMatchTicketErrorDelegate, MatchPool, Limit, Offset);
}
