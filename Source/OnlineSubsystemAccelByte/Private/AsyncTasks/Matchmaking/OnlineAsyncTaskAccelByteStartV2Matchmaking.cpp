// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteStartV2Matchmaking.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteStartV2Matchmaking"

FOnlineAsyncTaskAccelByteStartV2Matchmaking::FOnlineAsyncTaskAccelByteStartV2Matchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<FOnlineSessionSearchAccelByte>& InSearchHandle, const FName& InSessionName, const FString& InMatchPool, const FOnStartMatchmakingComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SearchHandle(InSearchHandle)
	, SessionName(InSessionName)
	, MatchPool(InMatchPool)
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(SearchHandle->SearchingPlayerId.ToSharedRef());
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalPlayerId: %s; SessionName: %s; MatchPool: %s"), *UserId->ToDebugString(), *SessionName.ToString(), *MatchPool);

	SearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;
	const bool bHasCachedLatencies = ApiClient->Qos.GetCachedLatencies().Num() > 0;
	if (!bHasCachedLatencies)
	{
		// If for some reason we have no latencies cached on the SDK, make a request to get latencies
		const THandler<TArray<TPair<FString, float>>>& OnGetLatenciesSuccessDelegate = THandler<TArray<TPair<FString, float>>>::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetLatenciesSuccess);
		const FErrorHandler& OnGetLatenciesErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetLatenciesError);

		ApiClient->Qos.GetServerLatencies(OnGetLatenciesSuccessDelegate, OnGetLatenciesErrorDelegate);

		return;
	}

	CreateMatchTicket();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bWasSuccessful)
	{
		SearchHandle->TicketId = CreateMatchTicketResponse.MatchTicketId;
	}
	else
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize task for starting matchmaking as our session interface is invalid!"));
			return;
		}

		// Set the search handle's state to failed since we were not successful in starting matchmaking
		SearchHandle->SearchState = EOnlineAsyncTaskState::Failed;

		// Reset the search handle and settings stored on the session interface
		SessionInterface->CurrentMatchmakingSearchHandle.Reset();
		SessionInterface->CurrentMatchmakingSessionSettings = {};
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for starting matchmaking as our session interface is invalid!"));
		return;
	}

	if (bWasSuccessful)
	{
		SessionInterface->TriggerOnMatchmakingStartedDelegates();
	}

	FSessionMatchmakingResults EmptyResults; // Results will always be empty as this is just us creating the ticket. Actual results will be filled in the search handle.
	Delegate.ExecuteIfBound(SessionName, ((bWasSuccessful) ? ONLINE_ERROR(EOnlineErrorResult::Success) : ONLINE_ERROR(EOnlineErrorResult::RequestFailure)), EmptyResults);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetLatenciesSuccess(const TArray<TPair<FString, float>>& InLatencies)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CreateMatchTicket();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetLatenciesError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to query region latencies from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CreateMatchTicket();
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::CreateMatchTicket()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// First, check if the player is in a party session. If so, we can grab the ID and use it for matchmaking.
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to create match ticket as our session interface is invalid!");

	// Now, create the match ticket on the backend
	const THandler<FAccelByteModelsV2MatchmakingCreateTicketResponse> OnStartMatchmakingSuccessDelegate = THandler<FAccelByteModelsV2MatchmakingCreateTicketResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnStartMatchmakingSuccess);
	const FErrorHandler OnStartMatchmakingErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnStartMatchmakingError);
	
	FAccelByteModelsV2MatchTicketOptionalParams Optionals;
	Optionals.Latencies = ApiClient->Qos.GetCachedLatencies();	
	Optionals.SessionId = GetTicketSessionId();

	const TSharedRef<FJsonObject> AttributesJsonObject = SessionInterface->ConvertSearchParamsToJsonObject(SearchHandle->QuerySettings.SearchParams);
	Optionals.Attributes.JsonObject = AttributesJsonObject;

	ApiClient->MatchmakingV2.CreateMatchTicket(MatchPool, OnStartMatchmakingSuccessDelegate, OnStartMatchmakingErrorDelegate, Optionals);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

FString FOnlineAsyncTaskAccelByteStartV2Matchmaking::GetTicketSessionId() const
{
	// First check if we have a session ID explicitly set in the matchmaking handle
	FString MatchmakingHandleSessionId;
	if (SearchHandle->QuerySettings.Get(SETTING_MATCHMAKING_SESSION_ID, MatchmakingHandleSessionId) && !MatchmakingHandleSessionId.IsEmpty())
	{
		return MatchmakingHandleSessionId;
	}
	
	// Otherwise, check if we have a party session that we are apart of, if so, use that ID
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	check(SessionInterface.IsValid());

	FNamedOnlineSession* PartySession = SessionInterface->GetPartySession();
	if (PartySession != nullptr)
	{
		FString PartySessionId = PartySession->GetSessionIdStr();
		if (!PartySessionId.Equals(TEXT("InvalidSession")))
		{
			return PartySessionId;
		}
	}

	// We have no session ID to attach, return empty
	return TEXT("");
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnStartMatchmakingSuccess(const FAccelByteModelsV2MatchmakingCreateTicketResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TicketId: %s"), *Result.MatchTicketId);

	CreateMatchTicketResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnStartMatchmakingError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to start matchmaking as the call to create a ticket failed! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE
