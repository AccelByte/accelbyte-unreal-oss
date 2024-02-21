// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCancelV2Matchmaking.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "Core/AccelByteError.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteCancelV2Matchmaking::FOnlineAsyncTaskAccelByteCancelV2Matchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<FOnlineSessionSearchAccelByte>& InSearchHandle, const FName& InSessionName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SearchHandle(InSearchHandle)
	, SessionName(InSessionName)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(SearchHandle->SearchingPlayerId.ToSharedRef());
}

void FOnlineAsyncTaskAccelByteCancelV2Matchmaking::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; TicketId: %s"), *UserId->ToDebugString(), *SearchHandle->TicketId);

	OnDeleteMatchTicketSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCancelV2Matchmaking::OnDeleteMatchTicketSuccess);
	OnDeleteMatchTicketErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCancelV2Matchmaking::OnDeleteMatchTicketError);

	ApiClient->MatchmakingV2.DeleteMatchTicket(SearchHandle->TicketId, OnDeleteMatchTicketSuccessDelegate, OnDeleteMatchTicketErrorDelegate);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCancelV2Matchmaking::Finalize()
{
	Super::Finalize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize the task of canceling matchmaking as our session interface is invalid!"));
			return;
		}

		SessionInterface->StopMatchTicketCheckPoll();

		SearchHandle->SearchState = EOnlineAsyncTaskState::Done; // #TODO Find better state for this potentially?

		SessionInterface->AddCanceledTicketId(SearchHandle->TicketId);
		SessionInterface->CurrentMatchmakingSearchHandle.Reset();
		SessionInterface->CurrentMatchmakingSessionSettings = {};

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsMPV2MatchmakingCanceledPayload MatchmakingCanceledPayload{};
			MatchmakingCanceledPayload.UserId = UserId->GetAccelByteId();
			MatchmakingCanceledPayload.MatchTicketId = SearchHandle->TicketId;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2MatchmakingCanceledPayload>(MatchmakingCanceledPayload));
		}
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCancelV2Matchmaking::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for canceling matchmaking as our session interface is invalid!"));
		return;
	}

	SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, bWasSuccessful);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	
}

void FOnlineAsyncTaskAccelByteCancelV2Matchmaking::OnDeleteMatchTicketSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCancelV2Matchmaking::OnDeleteMatchTicketError(int32 ErrorCode, const FString& ErrorMessage)
{
	if (ErrorCode == static_cast<int32>(AccelByte::ErrorCodes::MatchmakingV2MatchTicketNotFound))
	{
		// Since the backend was not able to find the ticket associated with our search handle to delete, we want to treat
		// this cancel as a success. This way, if the client is out of sync enough to think that they are still matchmaking,
		// they can still cancel and reflect that their ticket has been removed.
		UE_LOG_AB(Verbose, TEXT("Ticket was not found by matchmaking service when trying to cancel, treating this as a successful cancel!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	UE_LOG_AB(Warning, TEXT("Failed to cancel matchmaking ticket '%s' as the request to delete failed on the backend! Error code: %d; Error message: %s"), *SearchHandle->TicketId, ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
