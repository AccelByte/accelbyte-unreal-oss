// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteBackfillTicket.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

FOnlineAsyncTaskAccelByteDeleteBackfillTicket::FOnlineAsyncTaskAccelByteDeleteBackfillTicket(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnDeleteBackfillTicketComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteDeleteBackfillTicket::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_VALIDATE(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface), "Failed to delete backfill ticket as we do not have a valid session interface!");

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(Session != nullptr, "Failed to delete backfill ticket for session as we do not have a local session stored with the provided name!");

	FString BackfillTicketId{};
	AB_ASYNC_TASK_VALIDATE(Session->SessionSettings.Get(SETTING_MATCHMAKING_BACKFILL_TICKET_ID, BackfillTicketId), "Failed to delete backfill ticket as we do not have a valid backfill ticket ID stored!");

	SERVER_API_CLIENT_CHECK_GUARD()

	OnDeleteBackfillTicketSuccessDelegate = AccelByte::TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteBackfillTicket::OnDeleteBackfillTicketSuccess);
	OnDeleteBackfillTicketErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteBackfillTicket::OnDeleteBackfillTicketError);;
	ServerApiClient->ServerMatchmakingV2.DeleteBackfillTicket(BackfillTicketId, OnDeleteBackfillTicketSuccessDelegate, OnDeleteBackfillTicketErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteBackfillTicket::Finalize()
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

		Session->SessionSettings.Remove(SETTING_MATCHMAKING_BACKFILL_TICKET_ID);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteBackfillTicket::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteBackfillTicket::OnDeleteBackfillTicketSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteBackfillTicket::OnDeleteBackfillTicketError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Request to delete backfill ticket failed on backend!", ErrorCode, ErrorMessage);
}
