// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Models/AccelByteDSMModels.h"

FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
{
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
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

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
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
		const FOnAuthenticateServerComplete OnAuthenticateServerCompleteDelegate = FOnAuthenticateServerComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnAuthenticateServerComplete);
		IdentityInterface->AuthenticateAccelByteServer(OnAuthenticateServerCompleteDelegate);

		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Authenticating server with client credentials to get session information"), *SessionName.ToString());
		return;
	}

	TryQueryDedicatedSessionInfo();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s; Channel: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *Channel);

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		Session->SessionSettings.Set(SETTING_CHANNELNAME, Channel, EOnlineDataAdvertisementType::ViaOnlineService);
		Session->SessionSettings.bAllowJoinInProgress = bIsSessionJoinable;
		
		TSharedPtr<FOnlineSessionInfoAccelByte> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByte>(Session->SessionInfo);
		if (SessionInfo.IsValid())
		{
			SessionInfo->SetTeams(Teams);
			SessionInfo->SetParties(Parties);
		}

		Session->RegisteredPlayers.Append(CurrentPlayers);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	TSharedPtr<FOnlineSessionInfoAccelByte> SessionInfo = nullptr;
	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByte>(Session->SessionInfo);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnAuthenticateServerComplete(bool bAuthenticationSuccessful)
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

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::TryQueryDedicatedSessionInfo()
{
	// Query session status for this dedicated server to get information such as parties and teams, as well as joinable status
	// #SG We only can query session status for non custom games, so check for that before making a query
	if (!bIsCustomGame)
	{
		THandler<FAccelByteModelsServerSessionResponse> OnGetSessionIdSuccess = THandler<FAccelByteModelsServerSessionResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnGetSessionIdSuccess);
		FErrorHandler OnGetSessionIdError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnGetSessionIdError);
		FRegistry::ServerDSM.GetSessionId(OnGetSessionIdSuccess, OnGetSessionIdError);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnQueryMatchSessionSuccess(const FAccelByteModelsSessionBrowserData& Result)
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
				TSharedRef<const FUniqueNetIdAccelByteUser> MatchedUserId = FUniqueNetIdAccelByteUser::Create(CompositeId).ToSharedRef();
				CurrentPlayers.AddUnique(MatchedUserId);

				Teams.Add(MatchedUserId, TeamIndex);
				UsersInParty.AddUnique(MatchedUserId);
			}
			Parties.Add(UsersInParty);
		}
		TeamIndex++;
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully queried match session information for session '%s'!"), *SessionId);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnQueryMatchSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get match session from backend for session '%s'! Error code: %d; Error message: %s"), *SessionId, ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s; SessionId: %s"), *SessionName.ToString(), *Result.Session_id);

	SessionId = Result.Session_id;

	// We also want to query the session information itself to get matchmaking channel, so that we can perform operations
	// that require this channel to be passed in
	THandler<FAccelByteModelsSessionBrowserData> OnQuerySessionStatusSuccess = THandler<FAccelByteModelsSessionBrowserData>::CreateRaw(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnQueryMatchSessionSuccess);
	FErrorHandler OnQuerySessionStatusError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnQueryMatchSessionError);
	FRegistry::ServerSessionBrowser.GetGameSessionBySessionId(SessionId, OnQuerySessionStatusSuccess, OnQuerySessionStatusError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Found session ID '%s'! Querying status of this session from backend!"), *SessionId);
}

void FOnlineAsyncTaskAccelByteRetrieveDedicatedSessionInfo::OnGetSessionIdError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get session ID from backend for session '%s'! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
}
