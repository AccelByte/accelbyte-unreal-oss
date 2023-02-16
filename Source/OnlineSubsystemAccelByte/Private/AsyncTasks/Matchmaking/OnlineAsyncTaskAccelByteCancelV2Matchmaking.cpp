// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCancelV2Matchmaking.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

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

	const FVoidHandler OnDeleteMatchTicketSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCancelV2Matchmaking::OnDeleteMatchTicketSuccess);
	const FErrorHandler OnDeleteMatchTicketErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCancelV2Matchmaking::OnDeleteMatchTicketError);

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

		SearchHandle->SearchState = EOnlineAsyncTaskState::Done; // #TODO Find better state for this potentially?

		SessionInterface->CurrentMatchmakingSearchHandle.Reset();
		SessionInterface->CurrentMatchmakingSessionSettings = {};
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
	UE_LOG_AB(Warning, TEXT("Failed to cancel matchmaking ticket '%s' as the request to delete failed on the backend! Error code: %d; Error message: %s"), *SearchHandle->TicketId, ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
