// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "Misc/ScopeLock.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemAccelBytePackage.h"
#include "Core/AccelByteError.h"
#include "Models/AccelByteSessionBrowserModels.h"
#include "Models/AccelByteLobbyModels.h"
#include "LANBeacon.h"
#include "FNboSerializeToBufferAccelByte.h"
#include "OnlineDelegateMacros.h"

class FOnlineSubsystemAccelByte;
class FOnlineSessionV1AccelByte;

struct FMatchSummaryInfo;
class FOnlineSessionSearchResult;

struct FErrorInfo;
/**
 * Session setting describing the matchmaking channel that is used for a matchmaking ticket.
 */
#define SETTING_CHANNELNAME FName(TEXT("ABCHANNEL"))

/**
 * Matchmaking query setting describing whether the matchmaking call was made directly by the user or if it was made by
 * a remote player (ex. party leader).
 */
#define SETTING_IS_REMOTE_MATCHMAKING FName(TEXT("ABREMOTEMATCHMAKING"))

 /**
  * # Matchmaking query setting representing the sub game mode that we want to match with.
  */
#define SETTING_SUBGAMEMODE FName(TEXT("ABSUBGAMEMODE"))

/**
 * Internal structure for tracking matches that we are still getting information for from the matchmaker and other queries.
 */
struct FAccelBytePendingMatchInfo
{
	FString MatchId;
	FString ServerName;
	FString Ip;
	FString GameMode;
	int32 Port;
	TArray<FString> MatchedPlayers;
	double ReceivedTimeSeconds = 0.0;
	bool bIsLocalServer = false;
	bool bIsJoinable = false;
	bool bIsWaitingArea = false;

	/**
	 * Optional string containing information from the DS notification that created this pending match, used for error logging
	 */
	FString NotificationMessage;

	bool IsValid() const
	{
		// NOTE(Damar): removing GameMode and MatchedPlayers checking. Joinable session cannot have that data, unless gather it from SessionBrowser.
		return !MatchId.IsEmpty() && !ServerName.IsEmpty() && !Ip.IsEmpty() && Port > 0;
	}
};

/**
 * Delegate fired when deregistering a server from Armada finishes
 */
DECLARE_DELEGATE_OneParam(FOnDeregisterSessionComplete, bool /*bWasSuccessful*/)

/**
 * Delegate fired when a request to send a ready consent to the matchmaker finishes
 */
DECLARE_DELEGATE_OneParam(FOnSendReadyConsentComplete, bool /*bWasSuccessful*/)

/**
 * Delegate fired when a request to enqueue a joinable session completes
 */
DECLARE_DELEGATE_OneParam(FOnEnqueueJoinableSessionComplete, bool /*bWasSuccessful*/)

/**
 * Delegate fired when a request to dequeue a joinable session completes
 */
DECLARE_DELEGATE_OneParam(FOnDequeueJoinableSessionComplete, bool /*bWasSuccessful*/)

/**
 * Delegate fired when a request to query information on a dedicated matchmaking session completes
 */
DECLARE_DELEGATE_ThreeParams(FOnQueryDedicatedSessionInfoComplete, bool /*bWasSuccessful*/, FName SessionName, TSharedPtr<FOnlineSessionInfoAccelByteV1> /*SessionInfo*/)

/**
 * Delegate fired when a request to post event to session browser that a match has started completes
 */
DECLARE_DELEGATE_OneParam(FOnSendStartMatchEventComplete, bool /*bWasSuccessful*/)

/**
 * #SG Delegate fired when a request to drop a player from a session completes
 */
DECLARE_DELEGATE_OneParam(FOnDropPlayerComplete, bool /*bWasSuccessful*/)

/**
 * Delegate fired when we need to send a ready consent message to the matchmaker to join a match.
 * 
 * Normally, you'll either want to just call SessionInt->SendReady(MatchId) if you don't care if players ready up, or
 * have UI to send an explicit ready.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyConsentRequested, FString /*MatchId*/)
typedef FOnReadyConsentRequested::FDelegate FOnReadyConsentRequestedDelegate;

/**
 * Delegate fired when matchmaking has started.
 */
DECLARE_MULTICAST_DELEGATE(FOnMatchmakingStarted)
typedef FOnMatchmakingStarted::FDelegate FOnMatchmakingStartedDelegate;

/**
 * Delegate fired when a DS notification has been received from the Lobby websocket.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDedicatedServerNotification, FString /*Ip*/, int32 /*Port*/)
typedef FOnDedicatedServerNotification::FDelegate FOnDedicatedServerNotificationDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingFailed, const FErrorInfo& /*Error*/);
typedef FOnMatchmakingFailed::FDelegate FOnMatchmakingFailedDelegate;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionV1AccelByte : public IOnlineSession, public TSharedFromThis<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe>
{

private:

	/** Reference to the main AccelByte subsystem */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;
	
	/** Current search start time. */
	double LANPingStartSeconds = 0.0;
	FAccelByteModelsSessionBrowserData SessionBrowserData;

	/** Handles advertising sessions over LAN and client searches */
	FLANSession LANSessionManager;

	/**
	 * Session search handle that is currently being used, this will be set either explicitly by the user when
	 * StartMatchmaking is called, or when a start matchmaking notification is received and we need a handle.
	 * 
	 * In any case, the handle can be grabbed with SessionInt->GetSessionSearch()
	 */
	TSharedPtr<FOnlineSessionSearch> SessionSearchHandle;

	/**
	 * Default session settings for a session, set these with SessionInt->SetDefaultSessionSettings. These settings will be
	 * used if we get match results but haven't explicitly started a matchmaking session.
	 */
	FOnlineSessionSettings DefaultSessionSettings;

	/**
	 * Session settings for a specific matchmaking session, these take precedent over the default and are cleared after a
	 * session search result is constructed.
	 */
	TSharedPtr<FOnlineSessionSettings> ExplicitSessionSettings = nullptr;

	/**
	 * #SG Map of session ID to pending matches that we are still in the process of constructing
	 */
	TMap<FString, FAccelBytePendingMatchInfo> PendingMatchesUnderConstruction;

	/**
	 * #SG Array of pending matches used to filter for the best one to attempt to create a session for
	 */
	TArray<FAccelBytePendingMatchInfo> PendingMatchesToFilter;

	/**
	 * #SG Amount of time in seconds that we wish to increase the pending match wait time by between receiving sessions
	 */
	const double PendingMatchWaitTimeIncreaseSeconds = 1.0;

	/**
	 * #SG Amount of time in seconds that we wish to wait between pending matches before filtering and creating a session
	 */
	double PendingMatchWaitTimeSeconds = 0.0;

	/**
	 * #SG Time in seconds elapsed between multiple ticks, used for tracking how long we wish to wait before filtering and
	 * constructing a session from pending matches
	 * 
	 * #NOTE (Maxwell): This is a _terrible_ name for this member but I cannot think of a better one for the life of me
	 */
	double PendingMatchFilterTimerSeconds = 0.0;

	/**
	 * Whether or not we are actively filtering our pending match array
	 */
	bool bIsFilteringPendingMatches = false;

	/**
	 * Cache match ticket id from backend, will be used on handling possible race condition during matchmaking queue
	 */
	FString MatchmakingTicketId;

	/** Hidden on purpose */
	FOnlineSessionV1AccelByte() :
		AccelByteSubsystem(nullptr),
		SessionSearchHandle(nullptr)
	{}

	/**
	 * Delegate handler for when we get a notification back from the matchmaker that we have found a match.
	 */
	void OnMatchmakingNotificationReceived(const FAccelByteModelsMatchmakingNotice& Notification);
	
	/**
	 * Checks if the server for a notification is marked as a waiting area server
	 */
	bool IsDSNotifForWaitingArea(const FString& CustomAttribute);

	/**
	 * Create session information based on a DS notification
	 */
	void CreatePendingMatchFromDSNotif(const FAccelByteModelsDsNotice& Notification);

	/**
	 * Delegate handler for when we get a notification regarding a dedicated server from the AccelByte lobby.
	 */
	void OnDedicatedServerNotificationReceived(const FAccelByteModelsDsNotice& Notification);

	/**
	 * Send a request to get information about a session by its string ID.
	 * 
	 * Usage of this requires the NAMESPACE:{namespace}:SESSION permission to be set to READ on the OAuth Client
	 */
	void GetSessionInformation(const FString& SessionId);
	
	/**
	 * Handler for when we get a failed response back regarding session information.
	 */
	void OnGetMatchSessionError(int32 ErrorCode, const FString& ErrorMessage);
	
	/**
	 * Handler for when we get a failed response back regarding eject match session.
	 */
	void OnRejectMatchError(int32 ErrorCode, const FString& ErrorMessage);
	
	// #TICKETID end

	/**
	 * Construct a FOnlineSessionSearchResult object from the PendingMatchData (if valid) and add it to the
	 * SessionSearchHandle (also if valid)
	 */
	FOnlineSessionSearchResult ConstructSessionResultForMatch(const FAccelBytePendingMatchInfo& PendingMatch);

	void CreateP2PSession(FNamedOnlineSession* Session);
	void OnRTCConnected(const FString& NetId, bool bWasSuccessful, FName SessionName);
	void OnSessionCreateSuccess(const FAccelByteModelsSessionBrowserData& Data, FName SessionName);
	
PACKAGE_SCOPE:

	/**
	 * Delegate broadcast when we get a notification for a dedicated server from the Lobby websocket
	 * #SG Splitgate specific for DS creation in custom games
	 */
	FOnDedicatedServerNotification OnDedicatedServerNotificationDelegate;

	/**
	 * Flag describing whether we should continue to get session information as well as create a session object for a match upon receiving a dedicated server notification.
	 * #SG Splitgate specific for DS creation in custom games
	 */
	bool bSkipSessionWorkOnDSNotif = false;

	/** Current session settings */
	TArray<FNamedOnlineSession> Sessions;
	/** Critical sections for thread safe operation of session lists */
	mutable FCriticalSection SessionLock;
	
	FOnlineSessionV1AccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	// IOnlineSession
	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings) override
	{
		FScopeLock ScopeLock(&SessionLock);
		return new (Sessions) FNamedOnlineSession(SessionName, SessionSettings);
	}

	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSession& Session) override
	{
		FScopeLock ScopeLock(&SessionLock);
		return new (Sessions) FNamedOnlineSession(SessionName, Session);
	}

	/**
	 * Registers real time delegates for notifications such as matchmaking for the specified user
	 */
	void RegisterRealTimeLobbyDelegates(int32 LocalUserNum);

	/**
	 * Method fired when a session result is created for either matchmaking or a waiting area. Will notify proper delegates.
	 */
	void OnSessionResultCreateSuccess(const FOnlineSessionSearchResult& Result);

	/**
	 * Session tick for various background tasks
	 */
	void Tick(float DeltaTime);

	bool NeedsAdvertising();
	bool NeedsAdvertising( const FNamedOnlineSession& Session );
	bool IsSessionJoinable( const FNamedOnlineSession& Session) const;
	uint32 UpdateLANStatus();
	uint32 JoinLANSession(int32 PlayerNum, class FNamedOnlineSession* Session, const class FOnlineSession* SearchSession);
	uint32 FindLANSession();
	uint32 FinalizeLANSearch();	
	void AppendSessionToPacket(class FNboSerializeToBufferAccelByte& Packet, class FOnlineSession* Session);
	void AppendSessionSettingsToPacket(class FNboSerializeToBufferAccelByte& Packet, FOnlineSessionSettings* SessionSettings);	
	void ReadSessionFromPacket(class FNboSerializeFromBufferAccelByte& Packet, class FOnlineSession* Session);
	void ReadSettingsFromPacket(class FNboSerializeFromBufferAccelByte& Packet, FOnlineSessionSettings& SessionSettings);
	void OnValidQueryPacketReceived(uint8* PacketData, int32 PacketLength, uint64 ClientNonce);
	void OnValidResponsePacketReceived(uint8* PacketData, int32 PacketLength);
	void OnLANSearchTimeout();
	bool IsHost(const FNamedOnlineSession& Session) const;
	static void SetPortFromNetDriver(const FOnlineSubsystemAccelByte& Subsystem, const TSharedPtr<FOnlineSessionInfo>& SessionInfo);

public:

	/**
	 * Delegate fired when we need to send a ready consent message to the matchmaker to join a match.
	 *
	 * Normally, you'll either want to just call SessionInt->SendReady(MatchId) if you don't care if players ready up, or
	 * have UI to send an explicit ready.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnReadyConsentRequested, FString);

	/**
	 * Delegate fired when matchmaking has started.
	 */
	DEFINE_ONLINE_DELEGATE(OnMatchmakingStarted);

	virtual ~FOnlineSessionV1AccelByte() {}

	//~ Begin IOnlineSession Interface
	virtual TSharedPtr<const FUniqueNetId> CreateSessionIdFromString(const FString& SessionIdStr) override;
	virtual FNamedOnlineSession* GetNamedSession(FName SessionName) override;
	virtual void RemoveNamedSession(FName SessionName) override;
	virtual EOnlineSessionState::Type GetSessionState(FName SessionName) const override;
	virtual bool HasPresenceSession() override;
	virtual bool CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	virtual bool CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	virtual bool StartSession(FName SessionName) override;
	virtual bool UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData = true) override;
	virtual bool EndSession(FName SessionName) override;
	virtual bool DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate = FOnDestroySessionCompleteDelegate()) override;
	virtual bool IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId) override;
	virtual bool StartMatchmaking(const TArray< TSharedRef<const FUniqueNetId> >& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	virtual bool CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName) override;
	virtual bool CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName) override;
	virtual bool FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	virtual bool FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	virtual bool FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate) override;
	virtual bool CancelFindSessions() override;
	virtual bool PingSearchResults(const FOnlineSessionSearchResult& SearchResult) override;
	virtual bool JoinSession(int32 PlayerNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	virtual bool JoinSession(const FUniqueNetId& PlayerId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	virtual bool FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend) override;
	virtual bool FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend) override;
	virtual bool FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& FriendList) override;
	virtual bool SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend) override;
	virtual bool SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend) override;
	virtual bool SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& Friends) override;
	virtual bool SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& Friends) override;
	virtual bool GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType) override;
	virtual bool GetResolvedConnectString(const FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo) override;
	virtual FOnlineSessionSettings* GetSessionSettings(FName SessionName) override;
	virtual bool RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited) override;
	virtual bool RegisterPlayers(FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& Players, bool bWasInvited = false) override;
	virtual bool UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId) override;
	virtual bool UnregisterPlayers(FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& Players) override;
	virtual void RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate) override;
	virtual void UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate) override;
	virtual int32 GetNumSessions() override;
	virtual void DumpSessionState() override;
	//~ End IOnlineSession Interface
	
	/**
	* Used to handle notification regarding matchmaking being canceled by the party leader. Can also be used by non-party leaders to back out of matchmaking.
	*/
	void CancelMatchmakingNotification();

	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnMatchmakingFailed, const FErrorInfo& /* Error */);
	
	/**
	 * Method to deregister a local or remote server from Armada by its session.
	 */
	void DeregisterSession(FName SessionName, const FOnDeregisterSessionComplete& Delegate);

	/**
	 * Sends off a ready consent request for the specified match ID.
	 */
	void SendReady(int32 LocalUserNum, const FString& MatchId, const FOnSendReadyConsentComplete& Delegate=FOnSendReadyConsentComplete());

	/**
	 * Sets the default settings for a session to be created with. Used for matchmaking sessions when we did not explicitly
	 * start matchmaking ourselves (ex. if a party leader starts matchmaking).
	 */
	void SetDefaultSessionSettings(const FOnlineSessionSettings& DefaultSettings);

	/**
	 * Grabs the current search handle for matchmaking, should be used to get matchmaking status if we did not explicitly
	 * start matchmaking.
	 */
	TSharedPtr<FOnlineSessionSearch> GetSessionSearch();

	/**
	 * Enqueues a session to be joinable on the backend, allows for backfill with matchmaking.
	 */
	void EnqueueJoinableSession(FName SessionName, const FOnEnqueueJoinableSessionComplete& Delegate=FOnEnqueueJoinableSessionComplete());

	/**
	 * Dequeues a joinable session on the backend, disabling backfill through matchmaker.
	 */
	void DequeueJoinableSession(FName SessionName, const FOnDequeueJoinableSessionComplete& Delegate=FOnDequeueJoinableSessionComplete());

	/**
	 * Query information about a dedicated session from the backend, used to get team and party associations from backfill
	 */
	bool QueryDedicatedSessionInfo(FName SessionName, const FOnQueryDedicatedSessionInfoComplete& Delegate=FOnQueryDedicatedSessionInfoComplete());

	/**
	 * #SG Send event to session browser service that we have started a match for this session
	 */
	void SendMatchStartEvent(FName SessionName, const FString& Playlist, const FOnSendStartMatchEventComplete& Delegate=FOnSendStartMatchEventComplete());

	// #TICKETID start
	/**
	 * Get matchmaking ticket id
	 */
	FString GetMatchmakingTicketId();

	/**
	 * Set matchmaking ticket ids locally
	 */
	void SetMatchmakingTicketId(const FString& TicketId);

	/**
	 * Empty matchmaking ticket id locally
	 */
	void ClearMatchmakingTicketId();

	/**
	 * Continue create pending match data
	 */
	void ContinueCreatePendingMatchFromDSNotif(const FString& MatchId);

	virtual void SendBanUser(FName SessionName, const FUniqueNetId& PlayerId, int32 InActionID, const FString& InMessage);

	void TriggerOnDedicatedServerNotificationReceived(const FAccelByteModelsDsNotice& Notification);
};

typedef TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> FOnlineSessionV1AccelBytePtr;
