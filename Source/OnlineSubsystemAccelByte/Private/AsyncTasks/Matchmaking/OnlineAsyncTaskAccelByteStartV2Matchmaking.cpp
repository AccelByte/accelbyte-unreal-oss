// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteStartV2Matchmaking.h"

#include "Core/AccelByteError.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSessionSettingsAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteStartV2Matchmaking"

FOnlineAsyncTaskAccelByteStartV2Matchmaking::FOnlineAsyncTaskAccelByteStartV2Matchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<FOnlineSessionSearchAccelByte>& InSearchHandle, const FName& InSessionName, const FString& InMatchPool, const FOnStartMatchmakingComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SearchHandle(InSearchHandle)
	, SessionName(InSessionName)
	, MatchPool(InMatchPool)
	, OnlineError(FOnlineError())
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(SearchHandle->SearchingPlayerId.ToSharedRef());
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalPlayerId: %s; SessionName: %s; MatchPool: %s"), *UserId->ToDebugString(), *SessionName.ToString(), *MatchPool);

	API_CLIENT_CHECK_GUARD(OnlineError);

	OnGetLatenciesSuccessDelegate = TDelegateUtils<THandler<TArray<TPair<FString, float>>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetLatenciesSuccess);
	OnGetLatenciesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetLatenciesError);
	
	if (SearchHandle->GetIsP2PMatchmaking())
	{		
		ApiClient->TurnManager.GetTurnServerLatencies(OnGetLatenciesSuccessDelegate, OnGetLatenciesErrorDelegate);
		return;
	}
	else
	{
		const bool bHasCachedLatencies = ApiClient->Qos.GetCachedLatencies().Num() > 0;
		if (!bHasCachedLatencies)
		{
			// If for some reason we have no latencies cached on the SDK, make a request to get latencies
			ApiClient->Qos.GetServerLatencies(OnGetLatenciesSuccessDelegate, OnGetLatenciesErrorDelegate);
			return;
		}
	}

	CreateMatchTicket();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize task for starting matchmaking as our session interface is invalid!"));
		return;
	}

	if (bWasSuccessful)
	{
		SearchHandle->TicketId = CreateMatchTicketResponse.MatchTicketId;
		SessionInterface->FinalizeStartMatchmakingComplete();

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsMPV2MatchmakingRequestedPayload MatchmakingRequestedPayload{};
			MatchmakingRequestedPayload.UserId = UserId->GetAccelByteId();
			MatchmakingRequestedPayload.MatchPool = MatchPool;
			MatchmakingRequestedPayload.PartySessionId = GetTicketSessionId();
			MatchmakingRequestedPayload.Attributes = FJsonObjectWrapper{};
			MatchmakingRequestedPayload.Attributes.JsonObject = AttributesJsonObject;
			MatchmakingRequestedPayload.MatchTicketId = CreateMatchTicketResponse.MatchTicketId;
			MatchmakingRequestedPayload.QueueTime = CreateMatchTicketResponse.QueueTime;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2MatchmakingRequestedPayload>(MatchmakingRequestedPayload));
		}
	}
	else
	{
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
	TRY_PIN_SUBSYSTEM()

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for starting matchmaking as our session interface is invalid!"));
		return;
	}
	
	// Empty session id won't get OnMatchStartedNotif, so trigger OnMatchmakingStarted here
	if (bWasSuccessful && GetTicketSessionId().IsEmpty())
	{
		SessionInterface->TriggerOnMatchmakingStartedDelegates();
	}

	FSessionMatchmakingResults EmptyResults; // Results will always be empty as this is just us creating the ticket. Actual results will be filled in the search handle.
	Delegate.ExecuteIfBound(SessionName, OnlineError, EmptyResults);

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
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// First, check if the player is in a party session. If so, we can grab the ID and use it for matchmaking.
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to create match ticket as our session interface is invalid!");

	// Now, create the match ticket on the backend
	OnStartMatchmakingSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2MatchmakingCreateTicketResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnStartMatchmakingSuccess);
	OnStartMatchmakingErrorDelegate = TDelegateUtils<FCreateMatchmakingTicketErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnStartMatchmakingError);

	// Check if the caller has specified specific regions to matchmake to. If there are specifically requested regions,
	// then filter the cached latencies, removing regions that do not match. Otherwise, just attach all latencies to the
	// request.
	API_CLIENT_CHECK_GUARD(OnlineError);
	TArray<TPair<FString, float>> Latencies = ApiClient->Qos.GetCachedLatencies();
	TArray<FString> RequestedRegions{};
	if (FOnlineSearchSettingsAccelByte::Get(SearchHandle->QuerySettings, SETTING_GAMESESSION_REQUESTEDREGIONS, RequestedRegions) && RequestedRegions.Num() > 0)
	{
		// Filter the latencies array to contain only latency information for the requested regions
		Latencies = Latencies.FilterByPredicate([&RequestedRegions](const TPair<FString, float>& Latency) {
			return RequestedRegions.Contains(Latency.Get<0>());
		});
	}

	Optionals.Latencies = Latencies;
	
	Optionals.SessionId = GetTicketSessionId();

	AttributesJsonObject = SessionInterface->ConvertSearchParamsToJsonObject(SearchHandle->QuerySettings);
	Optionals.Attributes.JsonObject = AttributesJsonObject;

	if(SearchHandle->GetSearchStorage().IsValid())
	{
		if(SearchHandle->GetSearchStorage()->Values.Num() > 4)
		{
			const FString ErrorMsg = TEXT("Storage cannot have more than 4 items"); 
			UE_LOG_AB(Warning, TEXT("Failed to start matchmaking as the call to create a ticket failed! Error code: %d; Error message: %s"), ErrorCodes::InvalidRequest, *ErrorMsg);
			OnlineError = ONLINE_ERROR(EOnlineErrorResult::InvalidParams, FString::FromInt(static_cast<int32>(ErrorCodes::InvalidRequest)), FText::FromString(ErrorMsg));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to start matchmaking, storage cannot have more than 4 items!"));
			return;
		}
	
		StorageJsonObject = SearchHandle->GetSearchStorage();
	}
	else
	{
		StorageJsonObject = MakeShared<FJsonObject>();
	}
	
	StorageJsonObject->SetStringField(STORAGE_SESSION_NAME, SessionName.ToString());
	Optionals.Storage.JsonObject = StorageJsonObject;

	switch (SearchHandle.Get().GameSessionExclusion.CurrentType)
	{
		case FAccelBtyeModelsGameSessionExcludedSession::ExclusionType::ALL_MEMBER_CACHED_SESSION:
		case FAccelBtyeModelsGameSessionExcludedSession::ExclusionType::N_PAST_SESSION:
			ObtainPartyStorageExcludedSessionInfoThenCreateMatchTicket();
			break;
		case FAccelBtyeModelsGameSessionExcludedSession::ExclusionType::EXPLICIT_LIST:
			Optionals.ExcludedGameSessionIDs = SearchHandle.Get().GameSessionExclusion.GetExcludedGameSessionIDs();
			ApiClient->MatchmakingV2.CreateMatchTicket(MatchPool, OnStartMatchmakingSuccessDelegate, OnStartMatchmakingErrorDelegate, Optionals);
			break;
		case FAccelBtyeModelsGameSessionExcludedSession::ExclusionType::NONE:
			ApiClient->MatchmakingV2.CreateMatchTicket(MatchPool, OnStartMatchmakingSuccessDelegate, OnStartMatchmakingErrorDelegate, Optionals);
			break;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#pragma region PARTY_STORAGE_RELATED
void FOnlineAsyncTaskAccelByteStartV2Matchmaking::ObtainPartyStorageExcludedSessionInfoThenCreateMatchTicket()
{
	TRY_PIN_SUBSYSTEM();
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to create match ticket as our session interface is invalid!");
	API_CLIENT_CHECK_GUARD(OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// If doesn't have party, only use the local cached past session ID, no need to use the cached entire party info from backend
	auto PartyNameSession = SessionInterface->GetPartySession();
	if (PartyNameSession == nullptr)
	{
		auto PastSessionIDs = SessionInterface->PartySessionStorageLocalUserManager.PastSessionManager.GetPastSessionIDs(UserId);
		if (SearchHandle.Get().GameSessionExclusion.CurrentType == FAccelBtyeModelsGameSessionExcludedSession::ExclusionType::ALL_MEMBER_CACHED_SESSION)
		{
			Optionals.ExcludedGameSessionIDs = PastSessionIDs;
		}
		else if (SearchHandle.Get().GameSessionExclusion.CurrentType == FAccelBtyeModelsGameSessionExcludedSession::ExclusionType::N_PAST_SESSION)
		{
			auto ExcessToRemove = PastSessionIDs.Num() - SearchHandle.Get().GameSessionExclusion.ExcludedPastSessionCount;
			if (ExcessToRemove > 0)
			{
				PastSessionIDs.RemoveAt(0, ExcessToRemove, true);
			}
			Optionals.ExcludedGameSessionIDs = PastSessionIDs;
		}
		
		ApiClient->MatchmakingV2.CreateMatchTicket(MatchPool, OnStartMatchmakingSuccessDelegate, OnStartMatchmakingErrorDelegate, Optionals);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	// ELSE if current user has an empty partyID
	FString PartyID = PartyNameSession->GetSessionIdStr();
	if (PartyID.Len() == 0)
	{
		ApiClient->MatchmakingV2.CreateMatchTicket(MatchPool, OnStartMatchmakingSuccessDelegate, OnStartMatchmakingErrorDelegate, Optionals);
		return;
	}

	OnGetPartySessionStorageSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySessionStorage>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetPartySessionStorageSuccessCreateMatchTicket);
	OnGetPartySessionStorageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetPartySessionStorageError);
	ApiClient->Session.GetPartySessionStorage(PartyID, OnGetPartySessionStorageSuccessDelegate, OnGetPartySessionStorageErrorDelegate);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetPartySessionStorageSuccessCreateMatchTicket(const FAccelByteModelsV2PartySessionStorage& Result)
{
	API_CLIENT_CHECK_GUARD(OnlineError);
	TArray<FString> UniqueSessionIDs = FOnlineSessionV2AccelByte::ExtractExcludedSessionFromPartySessionStorage(SearchHandle.Get(), Result);
	Optionals.ExcludedGameSessionIDs = UniqueSessionIDs;
	ApiClient->MatchmakingV2.CreateMatchTicket(MatchPool, OnStartMatchmakingSuccessDelegate, OnStartMatchmakingErrorDelegate, Optionals);
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnGetPartySessionStorageError(int32 ErrorCode, const FString& ErrorMessage)
{
	//Regardless what happen to the session storage, still proceed to matchmaking
	API_CLIENT_CHECK_GUARD(OnlineError);
	ApiClient->MatchmakingV2.CreateMatchTicket(MatchPool, OnStartMatchmakingSuccessDelegate, OnStartMatchmakingErrorDelegate, Optionals);
}
#pragma endregion

FString FOnlineAsyncTaskAccelByteStartV2Matchmaking::GetTicketSessionId() const
{
	auto SubsystemPin = AccelByteSubsystem.Pin();
	if (!SubsystemPin.IsValid()) {
		return "";
	}

	// First check if we have a session ID explicitly set in the matchmaking handle
	FString MatchmakingHandleSessionId;
	if (SearchHandle->QuerySettings.Get(SETTING_MATCHMAKING_SESSION_ID, MatchmakingHandleSessionId) && !MatchmakingHandleSessionId.IsEmpty())
	{
		return MatchmakingHandleSessionId;
	}
	
	// Otherwise, check if we have a party session that we are apart of, if so, use that ID
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
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
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteStartV2Matchmaking::OnStartMatchmakingError(int32 ErrorCode, const FString& ErrorMessage, const FErrorCreateMatchmakingTicketV2& CreateTicketErrorInfo)
{
	// If the existing ticket id is not empty it means this user already started matchmaking in this match pool.
	// In that case restore the ticket to SearchHandle and count it as success
	if (CreateTicketErrorInfo.ExistingTicketID.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("Failed to start matchmaking as the call to create a ticket failed! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorMessage));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		UE_LOG_AB(Log, TEXT("Failed to start new matchmaking as current user already in matchmaking, restoring existing match ticket id %s"), *CreateTicketErrorInfo.ExistingTicketID);
		CreateMatchTicketResponse.MatchTicketId = CreateTicketErrorInfo.ExistingTicketID;
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

#undef ONLINE_ERROR_NAMESPACE
