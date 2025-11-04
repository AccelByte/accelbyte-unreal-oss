// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateBackfillTicket.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Core/AccelByteServerApiClient.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

FOnlineAsyncTaskAccelByteCreateBackfillTicket::FOnlineAsyncTaskAccelByteCreateBackfillTicket(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FString& InMatchPool, const FOnCreateBackfillTicketComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionName(InSessionName)
	, MatchPool(InMatchPool)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteCreateBackfillTicket::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_VALIDATE(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface), "Failed to create new backfill ticket as we do not have a valid session interface!");

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(Session != nullptr, "Failed to create backfill ticket for session as we do not have a local session stored with the provided name!");

	// Check if we already have a pool passed into this task. If not, then we want to try and grab it from the session itself.
	if (MatchPool.IsEmpty())
	{
		AB_ASYNC_TASK_VALIDATE(Session->SessionSettings.Get(SETTING_SESSION_MATCHPOOL, MatchPool), "Failed to create backfill ticket for session as there is not a match pool associated with it.");
	}

	SERVER_API_CLIENT_CHECK_GUARD();

	OnCreateBackfillTicketSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2MatchmakingCreateBackfillTicketResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateBackfillTicket::OnCreateBackfillTicketSuccess);
	OnCreateBackfillTicketErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateBackfillTicket::OnCreateBackfillTicketError);;
	ServerApiClient->ServerMatchmakingV2.CreateBackfillTicket(MatchPool, Session->GetSessionIdStr(), OnCreateBackfillTicketSuccessDelegate, OnCreateBackfillTicketErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateBackfillTicket::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	if (bWasSuccessful)
	{
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

		Session->SessionSettings.Set(SETTING_MATCHMAKING_BACKFILL_TICKET_ID, BackfillTicketId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateBackfillTicket::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateBackfillTicket::OnCreateBackfillTicketSuccess(const FAccelByteModelsV2MatchmakingCreateBackfillTicketResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	BackfillTicketId = Result.Id;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateBackfillTicket::OnCreateBackfillTicketError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Request to create backfill ticket failed on backend!", ErrorCode, ErrorMessage);
}
