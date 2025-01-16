// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSessionInterfaceV1AccelByte.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#if ENGINE_MINOR_VERSION >= 1
#include "Online/OnlineBase.h"
#endif // ENGINE_MINOR_VERSION >= 1
#include "Online/OnlineSessionNames.h"
#endif // ENGINE_MAJOR_VERSION == 5
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "SocketSubsystem.h"
#include "OnlineAsyncTaskManager.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteSessionBrowserApi.h"
#include "GameServerApi/AccelByteServerDSMApi.h"
#include "GameServerApi/AccelByteServerOauth2Api.h"
#include "Models/AccelByteSessionBrowserModels.h"
#include "Misc/DefaultValueHelper.h" 
#include "Api/AccelByteLobbyApi.h"
#include "AccelByteNetworkUtilities.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineVoiceInterfaceAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Core/AccelByteError.h"
#include "AsyncTasks/SessionV1/OnlineAsyncTaskAccelByteStartV1Matchmaking.h"
#include "AsyncTasks/SessionV1/OnlineAsyncTaskAccelByteRegisterPlayerV1.h"
#include "AsyncTasks/SessionV1/OnlineAsyncTaskAccelByteUnregisterPlayerV1.h"
#include "AsyncTasks/SessionV1/OnlineAsyncTaskAccelByteFindV1Sessions.h"
#include "AsyncTasks/SessionV1/OnlineAsyncTaskAccelByteUpdateV1GameSession.h"
#include "AsyncTasks/SessionV1//OnlineAsyncTaskAccelByteUpdateV1GameSettings.h"
#include "AsyncTasks/SessionV1/Server/OnlineAsyncTaskAccelByteRegisterDedicatedV1Session.h"
#include "AsyncTasks/SessionV1/Server/OnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo.h"
#include "AsyncTasks/SessionV1/Server/OnlineAsyncTaskAccelByteGetDedicatedV1SessionId.h"
#include "AsyncTasks/SessionV1/Server/OnlineAsyncTaskAccelByteEnqueueJoinableV1Session.h"
#include "AsyncTasks/SessionV1/Server/OnlineAsyncTaskAccelByteDequeueJoinableV1Session.h"
#include "AsyncTasks/SessionV1/Server/OnlineAsyncTaskAccelByteRemoveUserFromV1Session.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteBanUser.h"
#include "AsyncTasks/SessionV1/OnlineAsyncTaskAccelByteFindV1GameSessionById.h"

using namespace AccelByte;

bool GetConnectionStringFromSessionInfo(TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo, FString& ConnectInfo, int32 PortOverride = 0)
{
	// Check whether we have a Remote P2P relay ID in the session info. If we do not, then we just want to return the connection string as an
	// ip:port string. Otherwise, we want to return the P2P relay URL for the session.
	if (SessionInfo->GetRemoteId().IsEmpty())
	{
		int32 HostPort = SessionInfo->GetHostAddr()->GetPort();
		if (PortOverride > 0)
		{
			HostPort = PortOverride;
		}
		ConnectInfo = FString::Printf(TEXT("%s:%d"), *SessionInfo->GetHostAddr()->ToString(false), HostPort);
		return true;
	}
	else 
	{
		ConnectInfo = FString::Printf(ACCELBYTE_URL_PREFIX TEXT("%s:%d"), *SessionInfo->GetRemoteId(), SessionInfo->GetP2PChannel());
		return true;
	}
}

FOnlineSessionV1AccelByte::FOnlineSessionV1AccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem->AsShared())
	, SessionSearchHandle(nullptr)
{
}

bool FOnlineSessionV1AccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineSessionV1AccelBytePtr& OutInterfaceInstance)
{
#if !AB_USE_V2_SESSIONS
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(Subsystem->GetSessionInterface());
	return OutInterfaceInstance.IsValid();
#else
	OutInterfaceInstance = nullptr;
	return false;
#endif
}

bool FOnlineSessionV1AccelByte::GetFromWorld(const UWorld* World, FOnlineSessionV1AccelBytePtr& OutInterfaceInstance)
{
#if !AB_USE_V2_SESSIONS
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
#else
	OutInterfaceInstance = nullptr;
	return false;
#endif
}

void FOnlineSessionV1AccelByte::OnMatchmakingNotificationReceived(const FAccelByteModelsMatchmakingNotice& Notification)
{
	const UEnum* StatusEnum = StaticEnum<EAccelByteMatchmakingStatus>();
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Matchmaking Status: %s"), *StatusEnum->GetNameStringByValue(static_cast<int64>(Notification.Status)));

	ensureMsgf(IsInGameThread(), TEXT("OnMatchmakingNotificationReceived is not in game thread!"));

	if (Notification.Status == EAccelByteMatchmakingStatus::Start)
	{
		// If we don't already have a session handle for this matchmaking query or the state of the search handle is
		// one of completion (either we failed or we finished), create a new one
		if (!SessionSearchHandle.IsValid() || (SessionSearchHandle->SearchState == EOnlineAsyncTaskState::Failed || SessionSearchHandle->SearchState == EOnlineAsyncTaskState::Done))
		{
			SessionSearchHandle = MakeShared<FOnlineSessionSearch>();
			SessionSearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;
			SessionSearchHandle->QuerySettings.Set(SETTING_IS_REMOTE_MATCHMAKING, true, EOnlineComparisonOp::Equals);
			SessionSearchHandle->QuerySettings.Set<FString>(SETTING_GAMEMODE, Notification.GameMode, EOnlineComparisonOp::Equals);

			UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Matchmaking started by party leader!"));
		}
		
		TriggerOnMatchmakingStartedDelegates();	
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Received a notification that we have started matchmaking!"));
	}
	else if (Notification.Status == EAccelByteMatchmakingStatus::Cancel)
	{
		CancelMatchmakingNotification();
		
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Received a notification that matchmaking has been cancelled!"));
	}
	else if (Notification.Status == EAccelByteMatchmakingStatus::Done && !Notification.MatchId.IsEmpty())
	{
		TriggerOnReadyConsentRequestedDelegates(Notification.MatchId);

		TArray<FString> Allies; 
		for(const FAccelByteModelsMatchingAlly& Ally : Notification.MatchingAllies.Data)
		{
			for(const FAccelByteModelsMatchingParty& Party : Ally.Matching_parties)
			{
				for(const FAccelByteModelsUser& User : Party.Party_members)
				{
					Allies.Add(User.User_id);
				}
			}
		}

		FAccelBytePendingMatchInfo* FoundMatchInfo = PendingMatchesUnderConstruction.Find(Notification.MatchId);
		if(FoundMatchInfo)
		{
			FoundMatchInfo->MatchId = Notification.MatchId;
			FoundMatchInfo->GameMode = Notification.GameMode;
			FoundMatchInfo->bIsJoinable = Notification.Joinable;
			FoundMatchInfo->MatchedPlayers = Allies;
		}
		else
		{
			FAccelBytePendingMatchInfo ConstructMatchInfo = {};
			ConstructMatchInfo.MatchId = Notification.MatchId;
			ConstructMatchInfo.GameMode = Notification.GameMode;
			ConstructMatchInfo.bIsJoinable = Notification.Joinable;
			ConstructMatchInfo.MatchedPlayers = Allies;
			PendingMatchesUnderConstruction.Add(Notification.MatchId, ConstructMatchInfo);
		}
		
		bool bAutoSendReadyConsent = false;
		GConfig->GetBool(TEXT("OnlineSubsystemAccelByte"), TEXT("bAutoSendReadyConsent"), bAutoSendReadyConsent, GEngineIni);
		if(bAutoSendReadyConsent)
		{
			// TODO : @damar how to get the player index?
			SendReady(0, Notification.MatchId);
		}

		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Received a notification that we have found a match with ID '%s'! Waiting for ready consent..."), *Notification.MatchId);
	}
	else if (Notification.Status == EAccelByteMatchmakingStatus::Timeout)
	{
		FErrorInfo Error;
		Error.Message = TEXT("matchmaking-timeout");
		Error.ErrorMessage = TEXT("matchmaking-timeout");
		TriggerOnMatchmakingFailedDelegates(Error);
		
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Received a notification that matchmaking with matchId '%s' has reached a timeout state, aborting!"), *Notification.MatchId);
	}
	else
	{
		FErrorInfo Error;
		Error.Message = TEXT("game-server-failed-to-start");
		Error.ErrorMessage = TEXT("game-server-failed-to-start");
		TriggerOnMatchmakingFailedDelegates(Error);

		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Received a notification that matchmaking with matchId '%s' has reached a failure state, aborting!"), *Notification.MatchId);
	}
}

void FOnlineSessionV1AccelByte::CreatePendingMatchFromDSNotif(const FAccelByteModelsDsNotice& Notification)
{
	FAccelBytePendingMatchInfo MatchInfo;
	
	FAccelBytePendingMatchInfo* FoundMatchInfo = PendingMatchesUnderConstruction.Find(Notification.MatchId);
	if(FoundMatchInfo)
	{
		MatchInfo = *FoundMatchInfo;
	}
	
	MatchInfo.MatchId = Notification.MatchId;
	MatchInfo.ServerName = Notification.PodName;
	MatchInfo.Ip = Notification.Ip;
	MatchInfo.Port = Notification.Port;
	MatchInfo.bIsLocalServer = Notification.Region.IsEmpty();
	MatchInfo.ReceivedTimeSeconds = FPlatformTime::Seconds();
	MatchInfo.NotificationMessage = Notification.Message;
	
	ConstructSessionResultForMatch(MatchInfo);
	
	// We also want to query the session information itself to get the current players that are in our session
	// to fill on our OSS side, we do this after we get a DS for this session, as we probably will have the
	// session populated to the session browser API by this point
	// NOTE @damar : Commented this out, the code is not complete yet and above is the fix for matchmaking
	// Get a note from Play Team, we should minimize call from SessionBrowser. Use it only if needed. For Matchmaking, we should skip using SessionBrowser
	// GetSessionInformation(NewPendingMatch.MatchId);
}

void FOnlineSessionV1AccelByte::ContinueCreatePendingMatchFromDSNotif(const FString& MatchId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *MatchId);

	FAccelBytePendingMatchInfo* FoundMatchInfo = PendingMatchesUnderConstruction.Find(MatchId);
	if (FoundMatchInfo == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to continue creating a pending match from a DS notification as there is no pending match with ID '%s'!"), *MatchId);
		return;
	}

	// #TICKETID party member continue to create pending match data
	// From here, even if the match is invalid we want to add it to the array of match data structures, as it will be filtered
	PendingMatchWaitTimeSeconds += PendingMatchWaitTimeIncreaseSeconds;
	PendingMatchesToFilter.Add(*FoundMatchInfo);

	// Finally, remove this particular match from the matches under construction list so that we don't have any false duplicates
	PendingMatchesUnderConstruction.Remove(MatchId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Pushing data for match '%s' to array for filtering!"), *MatchId);
}

void FOnlineSessionV1AccelByte::OnDedicatedServerNotificationReceived(const FAccelByteModelsDsNotice& Notification)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Dedicated Server Status: %s"), *Notification.Status);

	// A status of READY or BUSY means we have a server that we can join, so fill out the session host addr with the data
	// from the dedicated server notification in these cases
	if (Notification.Status == TEXT("READY") || Notification.Status == TEXT("BUSY"))
	{
		if (bSkipSessionWorkOnDSNotif)
		{
			OnDedicatedServerNotificationDelegate.Broadcast(Notification.Ip, Notification.Port);
			bSkipSessionWorkOnDSNotif = false;

			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Got dedicated server back after request for custom game, notifiying async task!"));
		}
		else
		{
			CreatePendingMatchFromDSNotif(Notification);

			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found suitable server from matchmaker, getting information about session!"));
		}
	}
	else if(Notification.IsOK.Equals(TEXT("false"), ESearchCase::IgnoreCase))
	{
		FErrorInfo Error;
		Error.Message = Notification.Message;
		Error.Error = Notification.Message;

		SessionSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
		TriggerOnMatchmakingFailedDelegates(Error);
		TriggerOnMatchmakingCompleteDelegates(NAME_GameSession, false);
	}
}

void FOnlineSessionV1AccelByte::GetSessionInformation(const FString& SessionId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session information as we could not get an AccelByteSubsystem instance!"));
		return;
	}

	const int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();
	const AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session information as we could not get an API client instance!"));
		return;
	}
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent request to get session information for '%s'"), *SessionId);

	if(PendingMatchesUnderConstruction.Num() > 0)
	{
		ConstructSessionResultForMatch(PendingMatchesUnderConstruction[SessionId]);
	}
}

FOnlineSessionSearchResult FOnlineSessionV1AccelByte::ConstructSessionResultForMatch(const FAccelBytePendingMatchInfo& PendingMatch)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to construct session search result for matchmaking as we could not get an AccelByteSubsystem instance!"));
		return FOnlineSessionSearchResult();
	}

	if (!SessionSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to construct session search result for matchmaking as the search handle was invalid!"));
		return FOnlineSessionSearchResult();
	}

	if (!PendingMatch.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to construct session search result for matchmaking as we did not have valid match data!"));
		return FOnlineSessionSearchResult();
	}

	FOnlineSessionSearchResult Result;
	FOnlineSession Session;

	// Grab the proper session settings object between our explicit and default settings
	FOnlineSessionSettings SessionSettings;
	if (ExplicitSessionSettings.IsValid())
	{
		SessionSettings = *ExplicitSessionSettings.Get();
		ExplicitSessionSettings = nullptr;
	}
	else
	{
		SessionSettings = DefaultSessionSettings;
	}

	// Set the number of open connections to the number of available connections
	// Note that we probably want to set this to the max player counts based on game mode, but I don't think we have a way to query this yet
	Session.NumOpenPrivateConnections = SessionSettings.NumPrivateConnections;
	Session.NumOpenPublicConnections = SessionSettings.NumPublicConnections;

	// Grab the identity interface from the parent subsystem and use the nickname from that as the owning user name
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();

		Session.OwningUserName = IdentityInterface->GetPlayerNickname(LocalUserNum);
		Session.OwningUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	}

	Session.SessionSettings = SessionSettings;

	// Set up session info for the matchmaking session, fills out the host address and the session ID from the backend
	TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV1>();
	SessionInfo->SetSessionId(PendingMatch.MatchId);

	// HostAddr should always be instantiated to a proper default FInternetAddr instance, but just in case we will check if this is valid
	bool bIsIpValid = false;
	if (SessionInfo->GetHostAddr() != nullptr)
	{
		SessionInfo->GetHostAddr()->SetIp(*PendingMatch.Ip, bIsIpValid);
		SessionInfo->GetHostAddr()->SetPort(PendingMatch.Port);
	}

	if (!bIsIpValid)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session from matchmaking result as the IP address from the session was invalid! IP: %s"), *PendingMatch.Ip);
		return FOnlineSessionSearchResult();
	}

	Session.SessionInfo = SessionInfo;

	// Set up tracking settings for the session, such as game mode, whether the server is local, etc.
	Session.SessionSettings.Set(SETTING_GAMEMODE, PendingMatch.GameMode, EOnlineDataAdvertisementType::ViaOnlineService);
	Session.SessionSettings.Set(SETTING_SESSION_LOCAL, PendingMatch.bIsLocalServer, EOnlineDataAdvertisementType::ViaOnlineService);
	Session.SessionSettings.Set(SETTING_SESSION_SERVER_NAME, PendingMatch.ServerName, EOnlineDataAdvertisementType::ViaOnlineService);
	Session.SessionSettings.bAllowJoinInProgress = PendingMatch.bIsJoinable;
	Session.SessionSettings.bIsDedicated = true;

	Result.Session = Session;

	// Execute on the game thread so that all delegates also execute on that thread
	AsyncTask(ENamedThreads::GameThread, [this, Result]() {
		OnSessionResultCreateSuccess(Result);
	});

	return Result;
}

FUniqueNetIdPtr FOnlineSessionV1AccelByte::CreateSessionIdFromString(const FString& SessionIdStr)
{
	if (!SessionIdStr.IsEmpty())
	{
		return FUniqueNetIdAccelByteResource::Create(SessionIdStr);
	}
	return nullptr;
}

FNamedOnlineSession* FOnlineSessionV1AccelByte::GetNamedSession(FName SessionName)
{
	FScopeLock ScopeLock(&SessionLock);
	for (int32 SearchIndex = 0; SearchIndex < Sessions.Num(); SearchIndex++)
	{
		if (Sessions[SearchIndex].SessionName == SessionName)
		{
			return &Sessions[SearchIndex];
		}
	}
	
	return nullptr;
}

void FOnlineSessionV1AccelByte::RemoveNamedSession(FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	bool bHasRemovedSession = false;
	FScopeLock ScopeLock(&SessionLock);
	for (int32 SearchIndex = 0; SearchIndex < Sessions.Num(); SearchIndex++)
	{
		if (Sessions[SearchIndex].SessionName == SessionName)
		{
			Sessions.RemoveAtSwap(SearchIndex);
			bHasRemovedSession = true;
			break;
		}
	}

	if (bHasRemovedSession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Successfully removed session '%s'!"), *SessionName.ToString());
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to remove session '%s' as there was no session that matched that name!"), *SessionName.ToString());
	}
}

EOnlineSessionState::Type FOnlineSessionV1AccelByte::GetSessionState(FName SessionName) const
{
	FScopeLock ScopeLock(&SessionLock);
	for (int32 SearchIndex = 0; SearchIndex < Sessions.Num(); SearchIndex++)
	{
		if (Sessions[SearchIndex].SessionName == SessionName)
		{
			return Sessions[SearchIndex].SessionState;
		}
	}

	return EOnlineSessionState::NoSession;
}

bool FOnlineSessionV1AccelByte::HasPresenceSession()
{
	FScopeLock ScopeLock(&SessionLock);
	for (int32 SearchIndex = 0; SearchIndex < Sessions.Num(); SearchIndex++)
	{
		if (Sessions[SearchIndex].SessionSettings.bUsesPresence)
		{
			return true;
		}
	}

	return false;
}

bool FOnlineSessionV1AccelByte::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	return CreateSession(0, SessionName, NewSessionSettings);
}

bool FOnlineSessionV1AccelByte::CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerNum: %d; SessionName: %s"), HostingPlayerNum, *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Could not create session as a session with the name '%s' already exists!"), *SessionName.ToString());
		TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		return false;
	}
	
	Session = AddNamedSession(SessionName, NewSessionSettings);
	Session->SessionState = EOnlineSessionState::Creating;
	Session->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
	Session->NumOpenPublicConnections = NewSessionSettings.NumPublicConnections;
	Session->HostingPlayerNum = HostingPlayerNum;
	Session->bHosting = true;

	// Give a owning user ID and owning user name only if we are not running a dedicated server
	//
	// @todo I think this has implications if we want dedicated servers hosted by users, though I'm not too sure how that
	// authentication would be handled.
	if (!IsRunningDedicatedServer())
	{
		IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			Session->OwningUserId = IdentityInterface->GetUniquePlayerId(HostingPlayerNum);
			Session->OwningUserName = IdentityInterface->GetPlayerNickname(HostingPlayerNum);
		}

		// If the ID of the user that owns this session is not valid, then we cannot continue
		if (!Session->OwningUserId.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Could not create session as the ID of host player '%d' was not valid! SessionName: %s"), HostingPlayerNum, *SessionName.ToString());
			TriggerOnCreateSessionCompleteDelegates(SessionName, false);
			return false;
		}
	}

	// Unique identifier of this build for compatibility
	Session->SessionSettings.BuildUniqueId = GetBuildUniqueId();

	// Setup the session info and set up P2P relay info if we are not running a dedicated or LAN match
	TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV1>();

	bool bIceEnabled = false;
	Session->SessionSettings.Get(SETTING_ACCELBYTE_ICE_ENABLED, bIceEnabled);
	
	if (!Session->SessionSettings.bIsDedicated && !Session->SessionSettings.bIsLANMatch && bIceEnabled)
	{
		// Register the Socket Subsystem AccelByte when the session is P2P Relay (ICE connection)
		FAccelByteNetworkUtilitiesModule::Get().RegisterDefaultSocketSubsystem();		
		SessionInfo->SetupP2PRelaySessionInfo(*AccelByteSubsystemPtr);
		// Setup the network manager for using the logged player api client
		FOnlineIdentityAccelBytePtr IdentityInterface =  StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
		const AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(HostingPlayerNum); 
		FAccelByteNetworkUtilitiesModule::Get().Setup(ApiClient);
		FAccelByteNetworkUtilitiesModule::Get().EnableHosting();
	}

	// Try and grab session ID from the environment or from the command line if we have it in advance
	FString SessionId = FPlatformMisc::GetEnvironmentVariable(TEXT("NOMAD_META_session_id"));
	if (!SessionId.IsEmpty()) {
		SessionInfo->SetSessionId(SessionId);
	}
	else if (FParse::Value(FCommandLine::Get(), TEXT("SessionId="), SessionId)) {
		SessionInfo->SetSessionId(SessionId);
	}
	
	Session->SessionInfo = SessionInfo;
	Session->SessionState = EOnlineSessionState::Pending;

	if (NewSessionSettings.bIsLANMatch)
	{
		int32 Result = UpdateLANStatus();
		if (Result != ONLINE_SUCCESS)
		{
			RemoveNamedSession(SessionName);
		}
		return Result != ONLINE_IO_PENDING;
	}
	else if (NewSessionSettings.bIsDedicated)
	{
		bool bRegisterDSSession = false;
		Session->SessionSettings.Get(SETTING_SERVER_DEDICATED_REGISTER_SESSION, bRegisterDSSession);
		
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Spawning async task to register server to DSM."));
		
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session>(
			AccelByteSubsystemPtr.Get(), HostingPlayerNum, SessionName, NewSessionSettings, bRegisterDSSession);
		
		return true;
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Creating relay session browser instance on backend."));
		CreateP2PSession(Session);
		return true;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("No task spawned to create session."));
	return false;
}

void FOnlineSessionV1AccelByte::CreateP2PSession(FNamedOnlineSession* Session)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session browser as we could not get an AccelByteSubsystem instance!"));
		return;
	}
	
	// Retrieve the API client for the user hosting this session
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session browser as an identity interface instance could not be retrieved!"));
		return;
	}

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(Session->HostingPlayerNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session browser as an API client could not be retrieved for user num %d!"), Session->HostingPlayerNum);
		return;
	}

	const auto Setting = Session->SessionSettings;
	FString GameMode;
	FString GameMapName;
	const FString GameVersion = FString::Printf(TEXT("%d"), GetBuildUniqueId());
	int GameNumBot;
	Setting.Get(SETTING_GAMEMODE, GameMode);
	Setting.Get(SETTING_MAPNAME, GameMapName);
	Setting.Get(SETTING_NUMBOTS, GameNumBot);

	auto SettingJson = MakeShared<FJsonObject>();    	
	for(const auto &Set : Setting.Settings)
	{
		auto const &Data = Set.Value.Data;
		auto JsonShared = Set.Value.Data.ToJson();
		SettingJson->SetField(Set.Key.ToString(), JsonShared->TryGetField(TEXT("value")));
	}
		
	THandler<FAccelByteModelsSessionBrowserData> OnSessionCreateSuccessDelegate =
		THandler<FAccelByteModelsSessionBrowserData>::CreateThreadSafeSP(
			AsShared(), &FOnlineSessionV1AccelByte::OnSessionCreateSuccess, Session->SessionName
			);
	
	FErrorHandler OnSessionCreateFailedDelegate = FErrorHandler::CreateLambda([this, Session](int32 ErrorCode, const FString& ErrorMessage) {
		UE_LOG_AB(Warning, TEXT("ERROR %s"), *ErrorMessage);
		TriggerOnCreateSessionCompleteDelegates(Session->SessionName, false);
	});

	ApiClient->SessionBrowser.CreateGameSession(SETTING_SEARCH_TYPE_PEER_TO_PEER_RELAY, GameMode, GameMapName, GameVersion, GameNumBot, Setting.NumPublicConnections, Setting.NumPrivateConnections, TEXT(""), SettingJson, OnSessionCreateSuccessDelegate, OnSessionCreateFailedDelegate);
}

void FOnlineSessionV1AccelByte::OnSessionCreateSuccess(const FAccelByteModelsSessionBrowserData& Data, FName SessionName)
{
	this->SessionBrowserData = Data;
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		UE_LOG_AB(Error, TEXT("unable create session: Session is null"));
		TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV1> AccelByteSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
	if (!AccelByteSessionInfo.IsValid())
	{
		UE_LOG_AB(Error, TEXT("unable create session: AccelByteSessionInfo not valid"));
		TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		return;
	}

	AccelByteSessionInfo->SetSessionId(SessionBrowserData.Session_id);
	TriggerOnCreateSessionCompleteDelegates(SessionName, true);
}

bool FOnlineSessionV1AccelByte::StartSession(FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to start session '%s' as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to start session '%s' as it does not exist!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, false);
		});
		return false;
	}

	if (Session->SessionState == EOnlineSessionState::Pending || Session->SessionState == EOnlineSessionState::Ended)
	{
		// Mark the session as in progress and save the time that we kicked this session off
		Session->SessionState = EOnlineSessionState::InProgress;
		Session->SessionSettings.Set(SETTING_SESSION_START_TIME, FDateTime::Now().ToIso8601());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, true);
		});
		return true;
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start session '%s' in while in state '%s'!"), *SessionName.ToString(), EOnlineSessionState::ToString(Session->SessionState));
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, false);
		});
		return false;
	}
}

bool FOnlineSessionV1AccelByte::UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update session '%s' as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}
	
	// Currently we do support updating a session through the backend, only for current player, max player and session settings
	UE_LOG_AB(Warning, TEXT("FOnlineSessionAccelByte::UpdateSession is currently only support changing current player, max player and session settings!"));

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Could not update session as a session with the name '%s' does not exists!"), *SessionName.ToString());
		TriggerOnUpdateSessionCompleteDelegates(SessionName, false);
		return false;
	}

	auto CurrentPlayer = SessionBrowserData.Game_session_setting.Current_player > Session->RegisteredPlayers.Num() ?
		SessionBrowserData.Game_session_setting.Current_player > Session->RegisteredPlayers.Num() : Session->RegisteredPlayers.Num();

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateV1GameSession>(AccelByteSubsystemPtr.Get(), SessionName, UpdatedSessionSettings, CurrentPlayer, bShouldRefreshOnlineData);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateV1GameSettings>(AccelByteSubsystemPtr.Get(), SessionName, UpdatedSessionSettings, bShouldRefreshOnlineData);
	return true;
}

bool FOnlineSessionV1AccelByte::EndSession(FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to end session '%s' as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Failed to end session '%s' as the session does not exist!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, false);
		});
		return false;
	}
	Session->SessionSettings.Set(SETTING_SESSION_END_TIME, FDateTime::Now().ToIso8601());

	if (Session->SessionState != EOnlineSessionState::InProgress)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Cannot end session '%s' as the session is not in progress! Session state: %s"), *SessionName.ToString(), EOnlineSessionState::ToString(Session->SessionState));
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, false);
		});
		return false;
	}

	// Retrieve the API client for the user hosting this session
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to end session as an identity interface instance could not be retrieved!"));
		return false;
	}

	// First set to ending to notify that we are ending this session
	Session->SessionState = EOnlineSessionState::Ending;

	if (Session->SessionSettings.bIsDedicated)
	{
		// For dedicated, just set to ended and move on, deregistration of the server from Armada should be separate.
		Session->SessionState = EOnlineSessionState::Ended;
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Ended dedicated session! SessionName: %s"), *SessionName.ToString());
		TriggerOnEndSessionCompleteDelegates(SessionName, true);
		return true;
	}
	else
	{
		bool bIceEnabled = false;
		Session->SessionSettings.Get(SETTING_ACCELBYTE_ICE_ENABLED, bIceEnabled);

		// Unregister the Socket Subsystem AccelByte when the session is P2P Relay and when the session settings has specified
		// that we want to use the relay socket subsystem
		if (bIceEnabled)
		{
			FAccelByteNetworkUtilitiesModule::Get().UnregisterDefaultSocketSubsystem();
			FAccelByteNetworkUtilitiesModule::Get().DisableHosting();
		}

		if(Session->bHosting)
		{
			AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(Session->HostingPlayerNum);
			if (!ApiClient.IsValid())
			{
				AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to end session as an API client could not be retrieved for user num %d!"), Session->HostingPlayerNum);
				return false;
			}

			THandler<FAccelByteModelsSessionBrowserData> OnRemoveSessionSuccess = THandler<FAccelByteModelsSessionBrowserData>::CreateLambda([SessionInterface = AsShared(), Session, SessionName](const FAccelByteModelsSessionBrowserData& Result) {
				UE_LOG_AB(Warning, TEXT("Remove session browser success"));
				Session->SessionState = EOnlineSessionState::Ended;
				SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, true);
			});

			FErrorHandler OnRemoveSessionError = FErrorHandler::CreateLambda([SessionInterface = AsShared(), SessionName](int32 ErrorCode, const FString& ErrorMessage) {
				UE_LOG_AB(Error, TEXT("Failed to remove P2P relay game session '%s' from backend! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
				SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, false);
			});
			
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Spawning task to remove game session from session browser! SessionName: %s"), *SessionName.ToString());
			ApiClient->SessionBrowser.RemoveGameSession(SessionBrowserData.Session_id, OnRemoveSessionSuccess, OnRemoveSessionError);
			return true;
		}
		
		Session->SessionState = EOnlineSessionState::Ended;
		TriggerOnEndSessionCompleteDelegates(SessionName, true);
		return true;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to spawn a task to end a session on the backend! SessionName: %s"), *SessionName.ToString());
	return false;
}

bool FOnlineSessionV1AccelByte::DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate) 
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session '%s' as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Could not destroy session '%s' as it does not exist!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), CompletionDelegate, SessionName]() {
			CompletionDelegate.ExecuteIfBound(SessionName, false);
			SessionInterface->TriggerOnDestroySessionCompleteDelegates(SessionName, false);
		});
		return false;
	}

	RemoveNamedSession(Session->SessionName);
	CompletionDelegate.ExecuteIfBound(SessionName, true);
	TriggerOnDestroySessionCompleteDelegates(SessionName, true);

	FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystemPtr->GetVoiceInterface());
	if (VoiceInterface.IsValid())
	{
		VoiceInterface->RemoveAllTalkers();
	}

	return true;
}

bool FOnlineSessionV1AccelByte::IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId)
{
	return IsPlayerInSessionImpl(this, SessionName, UniqueId);
}

bool FOnlineSessionV1AccelByte::StartMatchmaking(const TArray<TSharedRef<const FUniqueNetId>>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to start matchmaking '%s' as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}

	// Set our explicit session settings as well as our search handle to those passed in
	SessionSearchHandle = SearchSettings;
	ExplicitSessionSettings = MakeShared<FOnlineSessionSettings>(NewSessionSettings);
	
	// Set that this matchmaking session was initiated by the player and thus not remote
	SessionSearchHandle->QuerySettings.Set(SETTING_IS_REMOTE_MATCHMAKING, false, EOnlineComparisonOp::Equals);

	// We should already know the game mode our match will have, so set that here as it's required for cancellation
	//CurrentPendingMatchData = FAccelBytePendingMatchInfo();
	//SearchSettings->QuerySettings.Get(SETTING_GAMEMODE, CurrentPendingMatchData.GameMode);

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteStartV1Matchmaking>(AccelByteSubsystemPtr.Get(), LocalPlayers, SessionName, NewSessionSettings, SearchSettings);
	
	return true;
}

bool FOnlineSessionV1AccelByte::CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerNum: %d; SessionName: %s"), SearchingPlayerNum, *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking '%s' as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);

	if (!PlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Failed to find player at index %d!"), SearchingPlayerNum);
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName] {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
		});
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Passing to cancel matchmaking with player '%s'!"), *PlayerId->ToDebugString());
	return CancelMatchmaking(PlayerId.ToSharedRef().Get(), SessionName);
}

bool FOnlineSessionV1AccelByte::CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; SessionName: %s"), *SearchingPlayerId.ToDebugString(), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cacncel matchmaking '%s' as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}

	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		return false;
	}

	// Get the player's API client
	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(SearchingPlayerId);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking as an API client could not be retrieved for user '%s'!"), *(SearchingPlayerId.ToDebugString()));
		return false;
	}

	if (!SessionSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking as we are not currently matchmaking!"));
		return false;
	}

	TSharedRef<const FUniqueNetIdAccelByteUser> ABUserId = FUniqueNetIdAccelByteUser::CastChecked(SearchingPlayerId);
	FString PartyLeaderIdString = ABUserId.Get().GetAccelByteId();

	// Note that here we are not registering any delegates for a cancel response, as we already will get a notification
	// when the matchmaking request is canceled
	AccelByte::Api::Lobby::FMatchmakingResponse OnCancelMatchmakingSuccess = AccelByte::Api::Lobby::FMatchmakingResponse::CreateLambda([this, ApiClient, PartyLeaderIdString, SessionName](const FAccelByteModelsMatchmakingResponse& Result) {
		if (Result.Code != TEXT("0"))
		{
			UE_LOG_AB(Warning, TEXT("Failed to cancel matchmaking! Error code: %s"), *Result.Code);
			AsyncTask(ENamedThreads::GameThread, [this, SessionName]() {
				TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
			});
		}

		if (ApiClient.IsValid())
		{
			ApiClient->Lobby.SetCancelMatchmakingResponseDelegate(AccelByte::Api::Lobby::FMatchmakingResponse());
		}

		// We will then want to send a party notification to tell other party members that we've canceled matchmaking
		FString Topic = PARTYNOTIF_PARTY_LEADER_CANCEL_MATCHMAKING;		// Customization changed notification
		FString Payload = PartyLeaderIdString;							// UserId of the party leader just in case the receiving party members need it 
	});
	ApiClient->Lobby.SetCancelMatchmakingResponseDelegate(OnCancelMatchmakingSuccess);

	FString GameModeToCancel;
	if (!SessionSearchHandle.IsValid() || !SessionSearchHandle->QuerySettings.Get(SETTING_GAMEMODE, GameModeToCancel))
	{
		UE_LOG_AB(Warning, TEXT("Failed to get matchmaking game mode from session search handle, defaulting to current pending match game mode!"));
	}

	ApiClient->Lobby.SendCancelMatchmaking(GameModeToCancel);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to cancel matchmaking for game mode '%s'!"), *GameModeToCancel);
	return true;
}

bool FOnlineSessionV1AccelByte::FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerNum: %d"), SearchingPlayerNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);

	if (!PlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Failed to find player at index %d!"), SearchingPlayerNum);
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared()]{
			SessionInterface->TriggerOnFindSessionsCompleteDelegates(false);
		});
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Passing to FindSessions with player '%s'!"), *PlayerId->ToDebugString());
	return FindSessions(PlayerId.ToSharedRef().Get(), SearchSettings);
}

bool FOnlineSessionV1AccelByte::FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV1Sessions>(AccelByteSubsystemPtr.Get(), SearchingPlayerId, SearchSettings);
	return true;
}

bool FOnlineSessionV1AccelByte::FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate)
{
	// @todo not supported by SDK yet, in subsequent version that we will upgrade to as soon as released
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; SessionId: %s"), *SearchingUserId.ToDebugString(), *SessionId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to end session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV1GameSessionById>(AccelByteSubsystemPtr.Get(), SearchingUserId, SessionId, CompletionDelegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off async task to find session with ID '%s'!"), *SessionId.ToDebugString());
	return false;
}

bool FOnlineSessionV1AccelByte::CancelFindSessions()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel find session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}
	
	// Currently unsupported
	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared()]() {
		SessionInterface->TriggerOnCancelFindSessionsCompleteDelegates(true);
	});
	return false;
}

bool FOnlineSessionV1AccelByte::PingSearchResults(const FOnlineSessionSearchResult& SearchResult)
{
	// @todo Going to need a way to ping for dedicated servers as well as P2P.
	return false;
}

bool FOnlineSessionV1AccelByte::JoinSession(const FUniqueNetId& PlayerId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s; SessionName: %s"), *PlayerId.ToDebugString(), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel join session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session as we could not get our identity interface instance!"));
		return false;
	}

	int32 LocalUserNum;
	if (!IdentityInterface->GetLocalUserNum(PlayerId, LocalUserNum))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session as we could not get a user index for the player ID specified!"));
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Moving to JoinSession call with LocalUserNum %d!"), LocalUserNum);
	return JoinSession(LocalUserNum, SessionName, DesiredSession);
}

bool FOnlineSessionV1AccelByte::JoinSession(int32 PlayerNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("PlayerNum: %d; SessionName: %s"), PlayerNum, *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session %s as we could not get an AccelByteSubsystem instance!"), *SessionName.ToString());
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session as we are already in a session named %s!"), *SessionName.ToString());
		TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
		return false;
	}

	// Retrieve the API client for this user
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session as an identity interface instance could not be retrieved!"));
		return false;
	}

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(PlayerNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session as an API client could not be retrieved for user num %d!"), PlayerNum);
		return false;
	}

	Session = AddNamedSession(SessionName, DesiredSession.Session);
	Session->SessionState = EOnlineSessionState::InProgress;

	bool bIsIceEnabled = false;
	Session->SessionSettings.Get(SETTING_ACCELBYTE_ICE_ENABLED, bIsIceEnabled);

	if (DesiredSession.Session.SessionSettings.bIsDedicated && !bIsIceEnabled)
	{
		// Currently, no extra work is needed to join a dedicated session besides triggering delegates
		TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::Success);
		return true;
	}
	else if (DesiredSession.Session.SessionSettings.bIsLANMatch)
	{
		Session->SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV1>();
		Session->SessionSettings.bShouldAdvertise = false;
		return (JoinLANSession(PlayerNum, Session, &DesiredSession.Session) == ONLINE_SUCCESS);
	}
	// If we are neither a dedicated or LAN match, then we are going to treat this as a P2P match and try and join as that
	else
	{
		// First check if we have valid session info, as we will require this for connecting to the remote player's session
		const TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(DesiredSession.Session.SessionInfo);
		if (!SessionInfo.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join P2P relay session as the session info for %s is not a valid instance!"), *SessionName.ToString());
			TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::CouldNotRetrieveAddress);
			return false;
		}

		// Next check if we are connected to the AccelByte lobby socket, which is required to connect to a P2P relay session
		if (ApiClient->Lobby.IsConnected())
		{
			// Register the Socket Subsystem AccelByte when the session is P2P Relay
			FAccelByteNetworkUtilitiesModule::Get().RegisterDefaultSocketSubsystem();

			// Setup the network manager for using the logged player api client 
			FAccelByteNetworkUtilitiesModule::Get().Setup(ApiClient);

			// Now we can request connection to the remote ID for the session owner
			const int32 P2PChannel = FMath::RandRange(1024, 4096);
			FAccelByteNetworkUtilitiesModule::Get().RequestConnect(FString::Printf (TEXT("%s:%d"), *SessionInfo->GetRemoteId(), P2PChannel));

			// Register a delegate that will notify that we've joined the session once we've connected to the WebRTC socket
			const FAccelByteNetworkUtilitiesModule::OnICERequestConnectFinished OnICEConnectionCompleteDelegate = FAccelByteNetworkUtilitiesModule::OnICERequestConnectFinished::CreateRaw(this, &FOnlineSessionV1AccelByte::OnRTCConnected, SessionName);
			FAccelByteNetworkUtilitiesModule::Get().RegisterICERequestConnectFinishedDelegate(OnICEConnectionCompleteDelegate);

			// Register a delegate that will notify if WebRTC socket closed
			const TDelegate<void(const FString&)> OnICEConnectionCloseDelegate = TDelegate<void(const FString&)>::CreateRaw(this, &FOnlineSessionV1AccelByte::OnRTCClosed, SessionName);
			FAccelByteNetworkUtilitiesModule::Get().RegisterICEClosedDelegate(OnICEConnectionCloseDelegate);
			return true;
		}
		else
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join P2P relay session as the AccelByte Lobby websocket is not connected!"));
			TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::CouldNotRetrieveAddress);
			return false;
		}
	}
}

void FOnlineSessionV1AccelByte::OnRTCConnected(const FString& NetId, const NetworkUtilities::EAccelByteP2PConnectionStatus& Status, FName SessionName)
{
	EOnJoinSessionCompleteResult::Type Result;
	
	switch (Status)
	{
	case NetworkUtilities::EAccelByteP2PConnectionStatus::Success:
		Result = EOnJoinSessionCompleteResult::Success;
		break;

	case NetworkUtilities::EAccelByteP2PConnectionStatus::SignalingServerDisconnected:
	case NetworkUtilities::EAccelByteP2PConnectionStatus::HostResponseTimeout:
	case NetworkUtilities::EAccelByteP2PConnectionStatus::PeerIsNotHosting:
		Result = EOnJoinSessionCompleteResult::SessionDoesNotExist;
		break;

	case NetworkUtilities::EAccelByteP2PConnectionStatus::JuiceGatherFailed:
	case NetworkUtilities::EAccelByteP2PConnectionStatus::JuiceGetLocalDescriptionFailed:
	case NetworkUtilities::EAccelByteP2PConnectionStatus::JuiceConnectionFailed:
	case NetworkUtilities::EAccelByteP2PConnectionStatus::FailedGettingTurnServer:
	case NetworkUtilities::EAccelByteP2PConnectionStatus::FailedGettingTurnServerCredential:
		Result = EOnJoinSessionCompleteResult::CouldNotRetrieveAddress;
		break;

	default:
		Result = EOnJoinSessionCompleteResult::UnknownError;
	}

	FAccelByteNetworkUtilitiesModule::Get().RegisterICERequestConnectFinishedDelegate(nullptr);
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	check(Session != nullptr);

	TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
	check(SessionInfo.IsValid());
	const TTuple<FString, int32> PeerChannel = FAccelByteNetworkUtilitiesModule::ExtractPeerAndChannel(NetId);
	SessionInfo->SetP2PChannel(PeerChannel.Value);

	TriggerOnJoinSessionCompleteDelegates(SessionName, Result);
}

void FOnlineSessionV1AccelByte::OnRTCClosed(const FString& NetId, FName SessionName)
{
	DestroySession(SessionName);
}

bool FOnlineSessionV1AccelByte::FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend)
{
	//@todo - not implemented yet
	return false;
}

bool FOnlineSessionV1AccelByte::FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend)
{
	//@todo - not implemented yet
	return false;
}

bool FOnlineSessionV1AccelByte::FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& FriendList)
{
	//@todo - not implemented yet
	return false;
}

bool FOnlineSessionV1AccelByte::SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend)
{
	//@todo - not implemented yet
	return false;
}

bool FOnlineSessionV1AccelByte::SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend)
{
	//@todo - not implemented yet
	return false;
}

bool FOnlineSessionV1AccelByte::SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& Friends)
{
	//@todo - not implemented yet
	return false;
}

bool FOnlineSessionV1AccelByte::SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& Friends)
{
	//@todo - not implemented yet
	return false;
}

bool FOnlineSessionV1AccelByte::GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType)
{
	bool bSuccess = false;
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
	if (PortType == NAME_BeaconPort)
	{
		int32 BeaconListenPort = GetBeaconPortFromSessionSettings(Session->SessionSettings);
		bSuccess = GetConnectionStringFromSessionInfo(SessionInfo, ConnectInfo, BeaconListenPort);
	}
	else if (PortType == NAME_GamePort)
	{
		bSuccess = GetConnectionStringFromSessionInfo(SessionInfo, ConnectInfo);
	}

	if (!bSuccess || ConnectInfo.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("Invalid session info in search result to GetResolvedConnectString()"));
	}

	return bSuccess;
}

bool FOnlineSessionV1AccelByte::GetResolvedConnectString(const FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo)
{
	bool bSuccess = false;
	if (SearchResult.Session.SessionInfo.IsValid())
	{
		TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(SearchResult.Session.SessionInfo);

		if (PortType == NAME_BeaconPort)
		{
			int32 BeaconListenPort = GetBeaconPortFromSessionSettings(SearchResult.Session.SessionSettings);
			bSuccess = GetConnectionStringFromSessionInfo(SessionInfo, ConnectInfo, BeaconListenPort);
		}
		else if (PortType == NAME_GamePort)
		{
			bSuccess = GetConnectionStringFromSessionInfo(SessionInfo, ConnectInfo);
		}
	}

	if (!bSuccess || ConnectInfo.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("Invalid session info in search result to GetResolvedConnectString()"));
	}

	return bSuccess;
}

FOnlineSessionSettings* FOnlineSessionV1AccelByte::GetSessionSettings(FName SessionName) 
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		return &Session->SessionSettings;
	}
	return nullptr;
}

bool FOnlineSessionV1AccelByte::RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited)
{
	TArray<TSharedRef<const FUniqueNetId>> Players;
	Players.Add(PlayerId.AsShared());
	return RegisterPlayers(SessionName, Players, bWasInvited);
}

bool FOnlineSessionV1AccelByte::RegisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players, bool bWasInvited)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; Players Count: %d; bWasInvited: %s"), *SessionName.ToString(), Players.Num(), LOG_BOOL_FORMAT(bWasInvited));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register player to a session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Could not register player as we failed to get session with name '%s'!"), *SessionName.ToString());
		TriggerOnRegisterPlayersCompleteDelegates(SessionName, Players, false);
		return false;
	}

	// If we do not have a session ID yet for this session and we are the host, then we want to query for the session ID
	// We add both the session ID query and the register player task as serial so that we get the ID first, and then register a player
	if (Session->bHosting && Session->SessionSettings.bIsDedicated && !Session->SessionInfo->GetSessionId().IsValid())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteGetDedicatedV1SessionId>(AccelByteSubsystemPtr.Get(), SessionName);
	}

	// TODO(damar): Delete check bHosting after there is fix in the BE
#if !UE_SERVER
	// Register player always called from all client, we should only allow to call this once
	if(Session->bHosting)
#endif
	{
		TArray<TSharedRef<const FUniqueNetId>> NewPlayers;
		for (TSharedRef<const FUniqueNetId> Player : Players)
		{
			if (!Session->RegisteredPlayers.Contains(Player))
			{
				NewPlayers.Add(Player);
			}
		}

		if (NewPlayers.Num() > 0)
		{
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteRegisterPlayersV1>(AccelByteSubsystemPtr.Get(), SessionName, NewPlayers, bWasInvited, false);
		}
		else
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("There is no new player, skip!"));
			return true;
		}
	}

	FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystemPtr->GetVoiceInterface());
	if (VoiceInterface.IsValid())
	{
		VoiceInterface->RegisterTalkers(Players);
	}

	if (Session->bHosting && Session->SessionSettings.bIsDedicated) 
	{
		// Get information about the session after the player has joined, used to get the latest session status for sending to the server
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo>(AccelByteSubsystemPtr.Get(), SessionName);
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Spawned async tasks for registering %d players to '%s' session!"), Players.Num(), *SessionName.ToString());
	return true;
}

bool FOnlineSessionV1AccelByte::UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId)
{
	TArray<TSharedRef<const FUniqueNetId>> Players;
	Players.Add(PlayerId.AsShared());
	return UnregisterPlayers(SessionName, Players);
}

bool FOnlineSessionV1AccelByte::UnregisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; Players Count: %d"), *SessionName.ToString(), Players.Num());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister player from a session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("Could not unregister players as we failed to get session with name '%s'!"), *SessionName.ToString());
		TriggerOnUnregisterPlayersCompleteDelegates(SessionName, Players, false);
		return false;
	}

	// If we do not have a session ID yet for this session and we are the host, then we want to query for the session ID
	// We add both the session ID query and the unregister player task as serial so that we get the ID first, and then unregister a player
	if (Session->bHosting && Session->SessionSettings.bIsDedicated && !Session->SessionInfo->GetSessionId().IsValid())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteGetDedicatedV1SessionId>(AccelByteSubsystemPtr.Get(), SessionName);
	}

	// TODO(damar): Delete check bHosting after there is fix in the BE
#if !UE_SERVER
	// Register player always called from all client, we should only allow to call this once
	if(Session->bHosting)
#endif
	{
		TArray<TSharedRef<const FUniqueNetId>> QuitPlayers;
		for(TSharedRef<const FUniqueNetId> Player : Players)
		{
			if(Session->RegisteredPlayers.Contains(Player))
			{
				QuitPlayers.Add(Player);
			}
		}
		if(QuitPlayers.Num() > 0)
		{
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteUnregisterPlayersV1>(AccelByteSubsystemPtr.Get(), SessionName, QuitPlayers);
		}
		else
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Player not in the current session, skip!"));
		}
	}
	if (Session->bHosting && Session->SessionSettings.bIsDedicated)
	{
		// Queue task to get updated session information after we removed a player from the queue
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo>(AccelByteSubsystemPtr.Get(), SessionName);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Spawned async tasks for unregistering %d players from '%s' session!"), Players.Num(), *SessionName.ToString());
	return true;
}

void FOnlineSessionV1AccelByte::RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate)
{
	Delegate.ExecuteIfBound(PlayerId, EOnJoinSessionCompleteResult::Success);
}

void FOnlineSessionV1AccelByte::UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate)
{
	Delegate.ExecuteIfBound(PlayerId, true);
}

int32 FOnlineSessionV1AccelByte::GetNumSessions()
{
	FScopeLock ScopeLock(&SessionLock);
	return Sessions.Num();
}

void FOnlineSessionV1AccelByte::DumpSessionState()
{
	FScopeLock ScopeLock(&SessionLock);

	for (int32 SessionIdx = 0; SessionIdx < Sessions.Num(); SessionIdx++)
	{
		DumpNamedSession(&Sessions[SessionIdx]);
	}
}

void FOnlineSessionV1AccelByte::RegisterRealTimeLobbyDelegates(int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register real-time lobby as we could not get an AccelByteSubsystem instance!"));
		return;
	}
	
	// Retrieve the API client for this user
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register real-time lobby as an identity interface instance could not be retrieved!"));
		return;
	}

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register real-time lobby as an API client could not be retrieved for user num %d!"), LocalUserNum);
		return;
	}

	// NOTE(Maxwell): We are globally setting matchmaking notifications here, as we want to be notified of matchmaking
	// status when we are in a party and the leader kicks off a matchmaking request. This requires a bit more work to
	// keep a search handle around so that the client can act on matchmaking results.
	const AccelByte::Api::Lobby::FMatchmakingNotif OnMatchmakingNotificationReceivedDelegate = AccelByte::Api::Lobby::FMatchmakingNotif::CreateRaw(this, &FOnlineSessionV1AccelByte::OnMatchmakingNotificationReceived);
	const AccelByte::Api::Lobby::FDsNotif OnDedicatedServerNotificationReceivedDelegate = AccelByte::Api::Lobby::FDsNotif::CreateRaw(this, &FOnlineSessionV1AccelByte::OnDedicatedServerNotificationReceived);
	ApiClient->Lobby.SetMatchmakingNotifDelegate(OnMatchmakingNotificationReceivedDelegate);
	ApiClient->Lobby.SetDsNotifDelegate(OnDedicatedServerNotificationReceivedDelegate);
}

void FOnlineSessionV1AccelByte::OnSessionResultCreateSuccess(const FOnlineSessionSearchResult& Result)
{
	SessionSearchHandle->SearchResults.Add(Result);
	SessionSearchHandle->SearchState = EOnlineAsyncTaskState::Done;
	
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Session Successfully created! SessionID = %s!"), *Result.Session.SessionInfo->GetSessionId().ToString());

	TriggerOnMatchmakingCompleteDelegates(NAME_GameSession, true);
}

void FOnlineSessionV1AccelByte::Tick(float DeltaTime)
{
	LANSessionManager.Tick(DeltaTime);

	// If we have some pending matches to filter, start timer and attempt filtering once timer has been reached
	if (PendingMatchesToFilter.Num() > 0 && !bIsFilteringPendingMatches)
	{
		PendingMatchFilterTimerSeconds += DeltaTime;
		if (PendingMatchFilterTimerSeconds >= PendingMatchWaitTimeSeconds)
		{
			PendingMatchFilterTimerSeconds = 0.0;
		}
	}
	// If we have no pending matches to filter but we still have timer values, then we want to reset these
	else if (PendingMatchFilterTimerSeconds > 0.0 && !bIsFilteringPendingMatches)
	{
		PendingMatchFilterTimerSeconds = 0.0;
		PendingMatchWaitTimeSeconds = 0.0;
	}
}

bool FOnlineSessionV1AccelByte::NeedsAdvertising()
{
	FScopeLock ScopeLock(&SessionLock);
	for (int32 i=0; i < Sessions.Num(); i++)
	{
		FNamedOnlineSession& Session = Sessions[i];
		if (NeedsAdvertising(Session))
		{
			return true;
		}
	}
	return false;
}

bool FOnlineSessionV1AccelByte::NeedsAdvertising(const FNamedOnlineSession& Session)
{
	return Session.SessionSettings.bIsLANMatch && Session.SessionSettings.bShouldAdvertise && IsHost(Session);
}

bool FOnlineSessionV1AccelByte::IsSessionJoinable(const FNamedOnlineSession& Session) const
{
	const auto& Settings = Session.SessionSettings;
	const bool bIsAdvertising = Settings.bShouldAdvertise || Settings.bIsLANMatch;
	const bool bJoinableFromProgress = Session.SessionState != EOnlineSessionState::InProgress || Settings.bAllowJoinInProgress;
	return bIsAdvertising && bJoinableFromProgress && Session.NumOpenPublicConnections > 0;
}

uint32 FOnlineSessionV1AccelByte::UpdateLANStatus()
{
	uint32 Result = ONLINE_SUCCESS;
	if (NeedsAdvertising())
	{
		if (LANSessionManager.GetBeaconState() == ELanBeaconState::NotUsingLanBeacon)
		{
			FOnValidQueryPacketDelegate QueryPacketDelegate = FOnValidQueryPacketDelegate::CreateRaw(this, &FOnlineSessionV1AccelByte::OnValidQueryPacketReceived);
			if (!LANSessionManager.Host(QueryPacketDelegate))
			{
				Result = ONLINE_FAIL;
				LANSessionManager.StopLANSession();
			}
		}
	}
	else
	{
		if (LANSessionManager.GetBeaconState() != ELanBeaconState::Searching)
		{
			LANSessionManager.StopLANSession();
		}
	}
	return Result;
}

uint32 FOnlineSessionV1AccelByte::JoinLANSession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession)
{
	uint32 Result = ONLINE_FAIL;
	Session->SessionState = EOnlineSessionState::Pending;

	if (Session->SessionInfo.IsValid() && SearchSession != nullptr && SearchSession->SessionInfo.IsValid())
	{
		const FOnlineSessionInfoAccelByteV1* SearchSessionInfo = static_cast<const FOnlineSessionInfoAccelByteV1*>(SearchSession->SessionInfo.Get());
		FOnlineSessionInfoAccelByteV1* SessionInfo = static_cast<FOnlineSessionInfoAccelByteV1*>(Session->SessionInfo.Get());
		SessionInfo->SetSessionId(SearchSessionInfo->GetSessionId().ToString());
		SessionInfo->SetHostAddr(SearchSessionInfo->GetHostAddr()->Clone());
		Result = ONLINE_SUCCESS;
	}
	return Result;
}

uint32 FOnlineSessionV1AccelByte::FindLANSession()
{
	uint32 Return = ONLINE_IO_PENDING;
	GenerateNonce(reinterpret_cast<uint8*>(&LANSessionManager.LanNonce), 8);
	auto ResponseDelegate = FOnValidResponsePacketDelegate::CreateRaw(this, &FOnlineSessionV1AccelByte::OnValidResponsePacketReceived);
	auto TimeoutDelegate = FOnSearchingTimeoutDelegate::CreateRaw(this, &FOnlineSessionV1AccelByte::OnLANSearchTimeout);
	FNboSerializeToBufferAccelByte Packet(LAN_BEACON_MAX_PACKET_SIZE);
	LANSessionManager.CreateClientQueryPacket(Packet, LANSessionManager.LanNonce);
	LANPingStartSeconds = FPlatformTime::Seconds();
	if (!LANSessionManager.Search(Packet, ResponseDelegate, TimeoutDelegate))
	{
		Return = ONLINE_FAIL;
		FinalizeLANSearch();
		SessionSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
		TriggerOnFindSessionsCompleteDelegates(false);
	}
	return Return;
}

uint32 FOnlineSessionV1AccelByte::FinalizeLANSearch()
{
	if (LANSessionManager.GetBeaconState() == ELanBeaconState::Searching)
	{
		LANSessionManager.StopLANSession();
	}
	return UpdateLANStatus();
}

void FOnlineSessionV1AccelByte::AppendSessionToPacket(FNboSerializeToBufferAccelByte& Packet, FOnlineSession* Session)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to append session to packet as we could not get an AccelByteSubsystem instance!"));
		return;
	}
	
	Packet << *FUniqueNetIdAccelByteUser::CastChecked(Session->OwningUserId.ToSharedRef());
	((FNboSerializeToBuffer&)Packet) << Session->OwningUserName;
	((FNboSerializeToBuffer&)Packet) << Session->NumOpenPrivateConnections;
	((FNboSerializeToBuffer&)Packet) << Session->NumOpenPublicConnections;
	SetPortFromNetDriver(*AccelByteSubsystemPtr.Get(), Session->SessionInfo);
	Packet << *StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
	AppendSessionSettingsToPacket(Packet, &Session->SessionSettings);
}

void FOnlineSessionV1AccelByte::AppendSessionSettingsToPacket(FNboSerializeToBufferAccelByte& Packet,
	FOnlineSessionSettings* SessionSettings)
{
	((FNboSerializeToBuffer&)Packet) << SessionSettings->NumPublicConnections
		<< SessionSettings->NumPrivateConnections
		<< static_cast<uint8>(SessionSettings->bShouldAdvertise)
		<< static_cast<uint8>(SessionSettings->bIsLANMatch)
		<< static_cast<uint8>(SessionSettings->bIsDedicated)
		<< static_cast<uint8>(SessionSettings->bUsesStats)
		<< static_cast<uint8>(SessionSettings->bAllowJoinInProgress)
		<< static_cast<uint8>(SessionSettings->bAllowInvites)
		<< static_cast<uint8>(SessionSettings->bUsesPresence)
		<< static_cast<uint8>(SessionSettings->bAllowJoinViaPresence)
		<< static_cast<uint8>(SessionSettings->bAllowJoinViaPresenceFriendsOnly)
		<< static_cast<uint8>(SessionSettings->bAntiCheatProtected)
		<< SessionSettings->BuildUniqueId;

	int32 Num = 0;
	for (FSessionSettings::TConstIterator It(SessionSettings->Settings); It; ++It)
	{	
		const auto& Setting = It.Value();
		if (Setting.AdvertisementType >= EOnlineDataAdvertisementType::ViaOnlineService)
		{
			Num++;
		}
	}

	((FNboSerializeToBuffer&)Packet) << Num;
	for (FSessionSettings::TConstIterator It(SessionSettings->Settings); It; ++It)
	{
		const auto& Setting = It.Value();
		if (Setting.AdvertisementType >= EOnlineDataAdvertisementType::ViaOnlineService)
		{
			((FNboSerializeToBuffer&)Packet) << It.Key();
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
			((FNboSerializeToBuffer&)Packet) << Setting;
#else
			((FNboSerializeToBufferOSS&)Packet) << Setting;
#endif
		}
	}
}

void FOnlineSessionV1AccelByte::ReadSessionFromPacket(FNboSerializeFromBufferAccelByte& Packet, FOnlineSession* Session)
{
	TSharedRef<FUniqueNetIdAccelByteUser> UniqueId = ConstCastSharedRef<FUniqueNetIdAccelByteUser>(FUniqueNetIdAccelByteUser::Invalid());
	Packet >> *UniqueId
		>> Session->OwningUserName
		>> Session->NumOpenPrivateConnections
		>> Session->NumOpenPublicConnections;

	Session->OwningUserId = UniqueId;
	TSharedRef<FOnlineSessionInfoAccelByteV1> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV1>();
	SessionInfo->SetHostAddr(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr());
	Packet >> *SessionInfo;
	Session->SessionInfo = SessionInfo;

	ReadSettingsFromPacket(Packet, Session->SessionSettings);
}

void FOnlineSessionV1AccelByte::ReadSettingsFromPacket(FNboSerializeFromBufferAccelByte& Packet, FOnlineSessionSettings& SessionSettings)
{
	SessionSettings.Settings.Empty();
	Packet >> SessionSettings.NumPublicConnections >> SessionSettings.NumPrivateConnections;
	uint8 Read = 0;
	Packet >> Read;
	SessionSettings.bShouldAdvertise = !!Read;
	Packet >> Read;
	SessionSettings.bIsLANMatch = !!Read;
	Packet >> Read;
	SessionSettings.bIsDedicated = !!Read;
	Packet >> Read;
	SessionSettings.bUsesStats = !!Read;
	Packet >> Read;
	SessionSettings.bAllowJoinInProgress = !!Read;
	Packet >> Read;
	SessionSettings.bAllowInvites = !!Read;
	Packet >> Read;
	SessionSettings.bUsesPresence = !!Read;
	Packet >> Read;
	SessionSettings.bAllowJoinViaPresence = !!Read;
	Packet >> Read;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = !!Read;
	Packet >> Read;
	SessionSettings.bAntiCheatProtected = !!Read;
	Packet >> SessionSettings.BuildUniqueId;
	int32 Num = 0;
	Packet >> Num;
	if (!Packet.HasOverflow())
	{
		FName Key;
		for (int32 i = 0; i < Num && !Packet.HasOverflow(); i++)
		{
			FOnlineSessionSetting Setting;
			Packet >> Key;
			Packet >> Setting;
			SessionSettings.Set(Key, Setting);
		}
	}
	
	if (Packet.HasOverflow())
	{
		SessionSettings.Settings.Empty();
		UE_LOG_AB(Verbose, TEXT("Packet overflow detected"));
	}
}

void FOnlineSessionV1AccelByte::OnValidQueryPacketReceived(uint8* PacketData, int32 PacketLength, uint64 ClientNonce)
{
	FScopeLock ScopeLock(&SessionLock);
	for (int32 i = 0; i < Sessions.Num(); i++)
	{
		FNamedOnlineSession* Session = &Sessions[i];
		if (Session && IsSessionJoinable(*Session))
		{
			FNboSerializeToBufferAccelByte Packet(LAN_BEACON_MAX_PACKET_SIZE);
			LANSessionManager.CreateHostResponsePacket(Packet, ClientNonce);
			AppendSessionToPacket(Packet, Session);
			if (!Packet.HasOverflow())
			{
				LANSessionManager.BroadcastPacket(Packet, Packet.GetByteCount());
			}
			else
			{
				UE_LOG_AB(Warning, TEXT("LAN broadcast packet overflow, cannot broadcast on LAN"));
			}
		}
	}
}

void FOnlineSessionV1AccelByte::OnValidResponsePacketReceived(uint8* PacketData, int32 PacketLength)
{
	FOnlineSessionSettings NewServer;
	if (SessionSearchHandle.IsValid())
	{
		// NOTE(Maxwell, 6/17/2021): For anyone else that might stumble upon this. This line creates the new search
		// result, but also calls an `operator new` that's set up in the TArray header (Array.h, around line 3062),
		// which will also allocate one uninitialized space in the array.
		FOnlineSessionSearchResult* NewResult = new (SessionSearchHandle->SearchResults) FOnlineSessionSearchResult();
		
		NewResult->PingInMs = static_cast<int32>((FPlatformTime::Seconds() - LANPingStartSeconds) * 1000);
		FOnlineSession* NewSession = &NewResult->Session;
		FNboSerializeFromBufferAccelByte Packet(PacketData, PacketLength);		
		ReadSessionFromPacket(Packet, NewSession);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to create new online game setting"));
	}
}

void FOnlineSessionV1AccelByte::OnLANSearchTimeout()
{
	FinalizeLANSearch();
	if (SessionSearchHandle.IsValid())
	{
		SessionSearchHandle->SearchState = EOnlineAsyncTaskState::Done;
		SessionSearchHandle = nullptr;
	}
	TriggerOnFindSessionsCompleteDelegates(true);
}

bool FOnlineSessionV1AccelByte::IsHost(const FNamedOnlineSession& Session) const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to check host as we could not get an AccelByteSubsystem instance!"));
		return false;
	}
	
	if (AccelByteSubsystemPtr->IsDedicated())
	{
		return true;
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface(); 
	if (!IdentityInterface.IsValid())
	{
		return false;
	}

	FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(Session.HostingPlayerNum);
	return (UserId.IsValid() && (*UserId == *Session.OwningUserId));
}

void FOnlineSessionV1AccelByte::SetPortFromNetDriver(const FOnlineSubsystemAccelByte& Subsystem, const TSharedPtr<FOnlineSessionInfo>& SessionInfo)
{
	int32 Port = GetPortFromNetDriver(Subsystem.GetInstanceName());
	TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfoAccelByte = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(SessionInfo);
	if (SessionInfoAccelByte.IsValid() && SessionInfoAccelByte->GetHostAddr().IsValid())
	{
		SessionInfoAccelByte->GetHostAddr()->SetPort(Port);
	}
}

void FOnlineSessionV1AccelByte::CancelMatchmakingNotification()
{
	// Set the search handle to reflect that the request failed as it was canceled
	if (SessionSearchHandle.IsValid())
	{
		SessionSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
	}

	// Trigger delegates that we canceled matchmaking
	// #NOTE(Maxwell): Using NAME_GameSession here, but that doesn't particularly matter, as long as the developer gets
	// the notification that we sent for matchmaking
	AsyncTask(ENamedThreads::GameThread, [this]() {
		TriggerOnCancelMatchmakingCompleteDelegates(NAME_GameSession, true);
	});
}

void FOnlineSessionV1AccelByte::DeregisterSession(FName SessionName, const FOnDeregisterSessionComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to deregister session from backend as the session '%s' does not exist!"), *SessionName.ToString());
		Delegate.ExecuteIfBound(false);
		return;
	}
	
	// Define simple lambda delegates for deregistering a server from Armada
	FVoidHandler OnShutdownSentSuccessful = FVoidHandler::CreateLambda([Delegate]() {
		UE_LOG_AB(Log, TEXT("AccelByte server successfully shut down!"));
		Delegate.ExecuteIfBound(true);
	});

	FErrorHandler OnShutdownSentError = FErrorHandler::CreateLambda([Delegate](int32 ErrorCode, const FString& ErrorMessage) {
		UE_LOG_AB(Warning, TEXT("Failed to send shutdown request to Armada server! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
		Delegate.ExecuteIfBound(false);
	});

	// Check if we are in a local server, as the calls are slightly different for deregistration
	bool bIsLocalSession = false;
	FAccelByteModelsDSUnregisteredPayload DSUnregisteredPayload{};

	TSharedPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
		{
			UE_LOG_AB(Warning, TEXT("Server AccelByteSubsystem is invalid, skipping call!"));
			Delegate.ExecuteIfBound(false);
		});
	}

	FAccelByteInstancePtr AccelByteInstancePtr = AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
	if(!AccelByteInstancePtr.IsValid())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
		{
			UE_LOG_AB(Warning, TEXT("Server AccelByteInstance is invalid, skipping call!"));
			Delegate.ExecuteIfBound(false);
		});
	}

	FServerApiClientPtr ServerApiClientPtr = AccelByteInstancePtr->GetServerApiClient();
	if(!ServerApiClientPtr.IsValid())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]()
		{
			UE_LOG_AB(Warning, TEXT("Server ServerApiClient is invalid, skipping call!"));
			Delegate.ExecuteIfBound(false);
		});
	}
	
	if (Session->SessionSettings.Get(SETTING_SESSION_LOCAL, bIsLocalSession) && bIsLocalSession)
	{
		// First get the name of the server we wish to unregister
		FString ServerName;
		Session->SessionSettings.Get(SETTING_SESSION_SERVER_NAME, ServerName);

		// Then send the request to unregister the local server
		ServerApiClientPtr->ServerDSM.DeregisterLocalServerFromDSM(ServerName, OnShutdownSentSuccessful, OnShutdownSentError);
		DSUnregisteredPayload.PodName = ServerName;
	}
	else
	{
		// Next, we want to send a shutdown request to Armada to deregister the server if we are not spawned from the DS launcher
		ServerApiClientPtr->ServerDSM.SendShutdownToDSM(false, Session->GetSessionIdStr(), OnShutdownSentSuccessful, OnShutdownSentError);
		DSUnregisteredPayload.PodName = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME"));
	}

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSUnregisteredPayload>(DSUnregisteredPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent request to deregister session '%s' from Armada."), *SessionName.ToString());
}

void FOnlineSessionV1AccelByte::SendReady(int32 LocalUserNum, const FString& MatchId, const FOnSendReadyConsentComplete& Delegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send ready as we could not get an AccelByteSubsystem instance!"));
		return;
	}
	
	// Retrieve the API client for this user
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send ready consent as an identity interface instance could not be retrieved!"));
		return;
	}

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send ready consent as an API client could not be retrieved for user num %d!"), LocalUserNum);
		return;
	}

	AccelByte::Api::Lobby::FReadyConsentResponse OnReadyConsentSent = AccelByte::Api::Lobby::FReadyConsentResponse::CreateLambda([Delegate](const FAccelByteModelsReadyConsentRequest& Result) {
		// Ready consent sending was successful if the code returned for the response is 0
		Delegate.ExecuteIfBound(Result.Code == TEXT("0"));
	});
	ApiClient->Lobby.SetReadyConsentResponseDelegate(OnReadyConsentSent);
	ApiClient->Lobby.SendReadyConsentRequest(MatchId);
}

void FOnlineSessionV1AccelByte::SetDefaultSessionSettings(const FOnlineSessionSettings& DefaultSettings)
{
	DefaultSessionSettings = DefaultSettings;
}

TSharedPtr<FOnlineSessionSearch> FOnlineSessionV1AccelByte::GetSessionSearch()
{
	return SessionSearchHandle;
}

void FOnlineSessionV1AccelByte::EnqueueJoinableSession(FName SessionName, const FOnEnqueueJoinableSessionComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to enqueue joinable session as we could not get an AccelByteSubsystem instance!"));
		return;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to enqueue joinable session on backend as the session '%s' does not exist!"), *SessionName.ToString());
		return;
	}

	if (!Session->bHosting)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to enqueue joinable session '%s' as we are not the host of the session!"), *SessionName.ToString());
		return;
	}

	// If we do not have the ID of this session yet, then we want to queue a task serially to get the ID of the session before any other calls.
	if (!Session->SessionInfo->GetSessionId().IsValid())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteGetDedicatedV1SessionId>(AccelByteSubsystemPtr.Get(), SessionName);
	}

	// Queue two tasks to first get the information about the session from the backend, and then to enqueue the session as joinable
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo>(AccelByteSubsystemPtr.Get(), SessionName);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session>(AccelByteSubsystemPtr.Get(), SessionName, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off tasks to enqueue joinable session on backend!"));
}

void FOnlineSessionV1AccelByte::DequeueJoinableSession(FName SessionName, const FOnDequeueJoinableSessionComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to dequeue joinable session as we could not get an AccelByteSubsystem instance!"));
		return;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to dequeue joinable session on backend as the session '%s' does not exist!"), *SessionName.ToString());
		return;
	}

	if (!Session->bHosting)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to dequeue joinable session '%s' as we are not the host of the session!"), *SessionName.ToString());
		return;
	}

	// If we do not have the ID of this session yet, then we want to queue a task serially to get the ID of the session.
	if (!Session->SessionInfo->GetSessionId().IsValid())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteGetDedicatedV1SessionId>(AccelByteSubsystemPtr.Get(), SessionName);
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteDequeueJoinableV1Session>(AccelByteSubsystemPtr.Get(), SessionName, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off tasks to dequeue joinable session from backend!"));
}

bool FOnlineSessionV1AccelByte::QueryDedicatedSessionInfo(FName SessionName, const FOnQueryDedicatedSessionInfoComplete& Delegate/*=FOnQueryDedicatedSessionInfoComplete()*/)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query dedicated session info as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRetrieveDedicatedV1SessionInfo>(AccelByteSubsystemPtr.Get(), SessionName, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV1AccelByte::RemoveUserFromSession(const FUniqueNetId& LocalUserId, const FString& ChannelName, const FString& MatchId, const FOnRemoveUserFromSessionComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("ChannelName: %s"), *ChannelName);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove user from session as we could not get an AccelByteSubsystem instance!"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRemoveUserFromV1Session>(AccelByteSubsystemPtr.Get(), LocalUserId, ChannelName, MatchId, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;

}

void FOnlineSessionV1AccelByte::SendBanUser(FName SessionName, const FUniqueNetId& PlayerId, int32 InActionID, const FString& InMessage)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send ready as we could not get an AccelByteSubsystem instance!"));
		return;
	}
	
	TSharedRef<const FUniqueNetIdAccelByteUser> PlayerIdComposite = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBanUser>(AccelByteSubsystemPtr.Get(), PlayerIdComposite->GetAccelByteId(), InActionID, InMessage);
}

void FOnlineSessionV1AccelByte::TriggerOnDedicatedServerNotificationReceived(const FAccelByteModelsDsNotice& Notification)
{
	OnDedicatedServerNotificationReceived(Notification);
}

bool FOnlineSessionV1AccelByte::ConstructGameSessionFromBackendSessionModel(
	const FAccelByteModelsSessionBrowserData& FoundSession, FOnlineSession& Session)
{
	// This block contains settings for features that we do not currently support, such as join via presence, invites, etc
	Session.NumOpenPrivateConnections = FoundSession.Game_session_setting.Current_internal_player;
	Session.SessionSettings.NumPrivateConnections = FoundSession.Game_session_setting.Max_internal_player;
	Session.SessionSettings.bAllowJoinViaPresence = false;
	Session.SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	Session.SessionSettings.bAllowInvites = false;
	Session.SessionSettings.bUsesPresence = true;
	Session.SessionSettings.bAntiCheatProtected = false;
	// End unsupported feature block

	// # AB (Apin) splitgate do differently about player count calculation
	Session.NumOpenPublicConnections = FoundSession.Game_session_setting.Max_player - FoundSession.Game_session_setting.Current_player;
	FDefaultValueHelper::ParseInt(FoundSession.Game_version, Session.SessionSettings.BuildUniqueId);
	Session.SessionSettings.NumPublicConnections = FoundSession.Game_session_setting.Max_player;
	Session.SessionSettings.bAllowJoinInProgress = FoundSession.Game_session_setting.Allow_join_in_progress;
	// Differentiating between dedicated and p2p sessions through the session browser is done through a string that will either be
	// 'p2p' or 'dedicated'. With that in mind, just check in a case insensitive manner if the session is marked as 'dedicated'.
	Session.SessionSettings.bIsDedicated = (FoundSession.Session_type.Compare(TEXT("dedicated"), ESearchCase::IgnoreCase) == 0);
	Session.SessionSettings.bIsLANMatch = false;
	Session.SessionSettings.bShouldAdvertise = true;

	auto Setting = FoundSession.Game_session_setting.Settings;
	if(Setting.JsonObject.IsValid())
	{
		for(const auto &JValue : FoundSession.Game_session_setting.Settings.JsonObject->Values)
		{
			switch (JValue.Value->Type)
			{
			case EJson::String:						
				Session.SessionSettings.Set(FName(JValue.Key), JValue.Value->AsString());
				break;
			case EJson::Boolean:						
				Session.SessionSettings.Set(FName(JValue.Key), JValue.Value->AsBool());
				break;
			case EJson::Number:						
				Session.SessionSettings.Set(FName(JValue.Key), JValue.Value->AsNumber());
				break;
			default:
				break;
			}
		}
	}		

	TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV1>();
	SessionInfo->SetSessionId(FoundSession.Session_id);

	if(!FoundSession.Server.Ip.IsEmpty())
	{
		// HostAddr should always be instantiated to a proper default FInternetAddr instance, but just in case we will check if this is valid
		bool bIsIpValid = false;
		if (SessionInfo->GetHostAddr() != nullptr)
		{
			SessionInfo->GetHostAddr()->SetIp(*FoundSession.Server.Ip, bIsIpValid);
			SessionInfo->GetHostAddr()->SetPort(FoundSession.Server.Port);
		}
	}

	// For sessions that are P2P, there will be a user ID associated with the session for the user who owns this session
	// If this is set, then we should set the owner ID and username to that contained in the result, and also set the
	// remote ID for the session info to the user ID.
	//
	// @sessions There may be a case where dedicated servers could have "owning" users, but for now that isn't supported
	if (!FoundSession.User_id.IsEmpty())
	{
		// Create composite ID for the owning user
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = FoundSession.User_id;

		// Create a shared ref for the user composite ID and set the session info
		TSharedRef<const FUniqueNetIdAccelByteUser> OwnerId = FUniqueNetIdAccelByteUser::Create(CompositeId);
		Session.OwningUserId = OwnerId;
		
		Session.OwningUserName = FoundSession.Username;
		if(FoundSession.Session_type == SETTING_SEARCH_TYPE_PEER_TO_PEER_RELAY)
		{
			SessionInfo->SetRemoteId(FoundSession.User_id);
		}			
	}

	Session.SessionInfo = SessionInfo;
	return true;
}
