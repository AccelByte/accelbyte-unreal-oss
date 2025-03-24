// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"

#include "Models/AccelByteDSMModels.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName, const FOnQueryDedicatedSessionInfoComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
	if (SessionInterface == nullptr)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query session status as our session interface is invalid!"));
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query session status as our session instance is invalid! SessionName: %s"), *SessionName.ToString());
		return;
	}

	if (!Session->bHosting)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Cannot query session status as we are not the host of the session! Marking task as success!"));
		return;
	}
	
	if (!Session->SessionInfo.IsValid() || !Session->SessionInfo->GetSessionId().IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query session status as our session ID is invalid! Session ID: %s"), (Session->SessionInfo->GetSessionId().IsValid()) ? *Session->SessionInfo->GetSessionId().ToString() : TEXT("nullptr"));
		return;
	}

	SessionId = Session->SessionInfo->GetSessionId().ToString();

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to register dedicated session with Armada as our identity interface was invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Dedicated server APIs need authentication with client credentials if we are not already authenticated
	//
	// Client authentication is an async process, so we'll have to do that, wait for the delegate result, and
	// from there register either a local server or a server on Armada.
	if (!IdentityInterface->IsServerAuthenticated())
	{
		const FOnAuthenticateServerComplete OnAuthenticateServerCompleteDelegate = TDelegateUtils<FOnAuthenticateServerComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnAuthenticateServerComplete);
		IdentityInterface->AuthenticateAccelByteServer(OnAuthenticateServerCompleteDelegate);

		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Authenticating server with client credentials to get session information, Session name: %s"), *SessionName.ToString());
		return;
	}

	TryQueryDedicatedSessionInfo();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s; Channel: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *Channel);

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		Session->SessionSettings.Set(SETTING_CHANNELNAME, Channel, EOnlineDataAdvertisementType::ViaOnlineService);
		Session->SessionSettings.bAllowJoinInProgress = bIsSessionJoinable;
		
		TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
		if (SessionInfo.IsValid())
		{
			SessionInfo->SetTeams(Teams);
			SessionInfo->SetParties(Parties);
			SessionInfo->SetSessionResult(SessionResult);
		}

		Session->RegisteredPlayers.Append(CurrentPlayers);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = nullptr;
	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);

		Delegate.ExecuteIfBound(bWasSuccessful, SessionName, SessionInfo);
	}
	else
	{
		Delegate.ExecuteIfBound(bWasSuccessful, SessionName, nullptr);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnAuthenticateServerComplete(bool bAuthenticationSuccessful)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bAuthenticationSuccessful: %s"), LOG_BOOL_FORMAT(bAuthenticationSuccessful));

	if (!bAuthenticationSuccessful)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query info for session with ID '%s' as we failed to authenticate our server!"), *SessionId);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// If we had to authenticate our server from here, then we also need to recreate session manager API class
	TryQueryDedicatedSessionInfo();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::TryQueryDedicatedSessionInfo()
{
	SERVER_API_CLIENT_CHECK_GUARD();
	
	THandler<FAccelByteModelsServerSessionResponse> OnGetSessionIdSuccess = TDelegateUtils<THandler<FAccelByteModelsServerSessionResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnGetSessionIdSuccess);
	FErrorHandler OnGetSessionIdError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnGetSessionIdError);
	ServerApiClient->ServerDSM.GetSessionId(OnGetSessionIdSuccess, OnGetSessionIdError);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnQueryMatchSessionSuccess(const FAccelByteModelsMatchmakingResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; Channel: %s"), *SessionName.ToString(), *Result.Channel);

	Channel = Result.Channel;

	// Channel gets returned from the backend with the format "{namespace}:{channel}", need to substring this to only get channel
	int32 SeparatorIndex;
	if (Channel.FindChar(':', SeparatorIndex))
	{
		Channel = Channel.Mid(SeparatorIndex + 1);
	}

	// Check whether the current session is marked joinable or not, affects player registration
	bIsSessionJoinable = Result.Joinable;

	// Get all initial users that are in this session query so that we can add them to the session
	int32 TeamIndex = 0;
	for (const FAccelByteModelsMatchingAlly& Ally : Result.Matching_allies)
	{
		for (const FAccelByteModelsMatchingParty& Party : Ally.Matching_parties)
		{
			TPartyMemberArray UsersInParty;
			for (const FAccelByteModelsUser& User : Party.Party_members)
			{
				FAccelByteUniqueIdComposite CompositeId;
				CompositeId.Id = User.User_id;
				TSharedRef<const FUniqueNetIdAccelByteUser> MatchedUserId = FUniqueNetIdAccelByteUser::Create(CompositeId);
				CurrentPlayers.AddUnique(MatchedUserId);

				Teams.Add(MatchedUserId, TeamIndex);
				UsersInParty.AddUnique(MatchedUserId);
			}
			Parties.Add(UsersInParty);
		}
		TeamIndex++;
	}

	SessionResult = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully queried match session information for session '%s'!"), *SessionId);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnQueryCustomMatchSessionSuccess(const FAccelByteModelsSessionBrowserData& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; Channel: %s"), *SessionName.ToString(), *Result.Match.Channel);

	Channel = Result.Match.Channel;

	// Channel gets returned from the backend with the format "{namespace}:{channel}", need to substring this to only get channel
	int32 SeparatorIndex;
	if (Channel.FindChar(':', SeparatorIndex))
	{
		Channel = Channel.Mid(SeparatorIndex + 1);
	}

	// Check whether the current session is marked joinable or not, affects player registration
	bIsSessionJoinable = Result.Joinable;

	// Get all initial users that are in this session query so that we can add them to the session
	int32 TeamIndex = 0;
	for (const FAccelByteModelsMatchingAlly& Ally : Result.Match.Matching_allies)
	{
		for (const FAccelByteModelsMatchingParty& Party : Ally.Matching_parties)
		{
			TPartyMemberArray UsersInParty;
			for (const FAccelByteModelsUser& User : Party.Party_members)
			{
				FAccelByteUniqueIdComposite CompositeId;
				CompositeId.Id = User.User_id;
				TSharedRef<const FUniqueNetIdAccelByteUser> MatchedUserId = FUniqueNetIdAccelByteUser::Create(CompositeId);
				CurrentPlayers.AddUnique(MatchedUserId);

				Teams.Add(MatchedUserId, TeamIndex);
				UsersInParty.AddUnique(MatchedUserId);
			}
			Parties.Add(UsersInParty);
		}
		TeamIndex++;
	}

	SessionResult = Result.Match;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully queried match session information for session '%s'!"), *SessionId);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnQueryMatchSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get match session from backend for session '%s'! Error code: %d; Error message: %s"), *SessionId, ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; SessionId: %s"), *SessionName.ToString(), *Result.Session_id);

	SessionId = Result.Session_id;

	if (SessionId.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get session ID from backend for session '%s'! SessionId is empty"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD();

	if (!bIsCustomGame) 
	{
		THandler<FAccelByteModelsMatchmakingResult> OnQuerySessionStatusSuccess = TDelegateUtils<THandler<FAccelByteModelsMatchmakingResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnQueryMatchSessionSuccess);
		FErrorHandler OnQuerySessionStatusError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnQueryMatchSessionError);
		ServerApiClient->ServerMatchmaking.QuerySessionStatus(SessionId, OnQuerySessionStatusSuccess, OnQuerySessionStatusError);
	}
	else
	{
		// We also want to query the session information itself to get matchmaking channel, so that we can perform operations
		// that require this channel to be passed in
		THandler<FAccelByteModelsSessionBrowserData> OnQuerySessionStatusSuccess = TDelegateUtils<THandler<FAccelByteModelsSessionBrowserData>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnQueryCustomMatchSessionSuccess);
		FErrorHandler OnQuerySessionStatusError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnQueryMatchSessionError);
		ServerApiClient->ServerSessionBrowser.GetGameSessionBySessionId(SessionId, OnQuerySessionStatusSuccess, OnQuerySessionStatusError);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Found session ID '%s'! Querying status of this session from backend!"), *SessionId);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo::OnGetSessionIdError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get session ID from backend for session '%s'! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
}
