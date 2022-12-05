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
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Models/AccelByteSessionModels.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "Models/AccelByteDSHubModels.h"

class FInternetAddr;
class FNamedOnlineSession;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionInfoAccelByteV2 : public FOnlineSessionInfo
{
public:
	FOnlineSessionInfoAccelByteV2(const FString& SessionIdStr);

	//~ Begin FOnlineSessionInfo overrides
	const FUniqueNetId& GetSessionId() const override;
	const uint8* GetBytes() const override;
	int32 GetSize() const override;
	bool IsValid() const override;
	FString ToString() const override;
	FString ToDebugString() const override;
	//~ End FOnlineSessionInfo overrides

	/** Set the host address for this session if one is available, used for game sessions */
	void SetHostAddress(const TSharedRef<FInternetAddr>& InHostAddress);

	/** Get the host address for this session, may be nullptr */
	TSharedPtr<FInternetAddr> GetHostAddress() const;

	/** Set the current backend session data associated with this session */
	void SetBackendSessionData(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBackendSessionData);

	/** Set the current backend session data associated with this session */
	void SetBackendSessionData(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBackendSessionData, bool& bWasInvitedPlayersChanged);

	/** Get the locally stored backend session data for this session */
	TSharedPtr<FAccelByteModelsV2BaseSession> GetBackendSessionData() const;

	/** Get the locally stored backend session data for this session as game session*/
	TSharedPtr<FAccelByteModelsV2GameSession> GetBackendSessionDataAsGameSession() const;

	/** Get the locally stored team assignments for this session */
	TArray<FAccelByteModelsV2GameSessionTeam> GetTeamAssignments() const;

	/** Set the locally stored team assignments for this session, can be updated on backend with UpdateSession */
	void SetTeamAssignments(const TArray<FAccelByteModelsV2GameSessionTeam>& InTeams);

	/** Whether or not this session info structure has information to allow the player to connect to it remotely */
	bool HasConnectionInfo() const;

	/** Return connection info that this session has as a string, or blank if none is found */
	FString GetConnectionString() const;

	/** Returns the ID of the leader of this session. Only valid for party sessions, will be nullptr otherwise. */
	FUniqueNetIdPtr GetLeaderId() const;

	/** Get ID of the peer we would be joining in a P2P session, will be blank if server type is not P2P */
	FString GetPeerId() const;

	/** Get the type of server that this session is using */
	EAccelByteV2SessionConfigurationServerType GetServerType() const;

	/** Get an array of user IDs representing players that are marked as joined to this session */
	TArray<FUniqueNetIdRef> GetJoinedMembers() const;

	/** Get an array of user IDs representing players that are marked as invited to this session*/
	TArray<FUniqueNetIdRef> GetInvitedPlayers() const;

PACKAGE_SCOPE:
	/**
	 * Update the list of invited players on this session from the backend session data.
	 * 
	 * @param bOutJoinedMembersChanged Boolean denoting whether the array of joined members has changed
	 * @param bOutInvitedPlayersChanged Boolean denoting whether the array of invited players has changed
	 */
	void UpdatePlayerLists(bool& bOutJoinedMembersChanged, bool& bOutInvitedPlayersChanged);

	/**
	 * Update the stored leader ID for this session. Intended only for use with party sessions.
	 */
	void UpdateLeaderId();

	/**
	 * Update the stored connection information for this server.
	 */
	void UpdateConnectionInfo();

	/** Set the most recent locally stored backend session data update for this session */
	void SetLatestBackendSessionDataUpdate(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBackendSessionData);

	/** Get the most recent locally stored backend session data update for this session */
	TSharedPtr<FAccelByteModelsV2BaseSession> GetLatestBackendSessionDataUpdate() const;

	/** Get the most recent locally stored backend session data update for this session as game session */
	TSharedPtr<FAccelByteModelsV2GameSession> GetLatestBackendSessionDataUpdateAsGameSession() const;

	/** Set the value for the "is latest update a DS ready update" flag */
	void SetDSReadyUpdateReceived(const bool& bInDSReadyUpdateReceived);

	/** Get the value for the "is latest update a DS ready update" flag */
	bool GetDSReadyUpdateReceived() const;

	/**
	 * Attempt to find a member of this session by their ID
	 */
	bool FindMember(const FUniqueNetId& MemberId, FAccelByteModelsV2SessionUser*& OutMember);

	/**
	 * Check if this session contains a member with the provided ID at all
	 */
	bool ContainsMember(const FUniqueNetId& MemberId);

private:
	TSharedPtr<FAccelByteModelsV2GameSession> StaticCastAsGameSession(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBaseSession) const;

	/**
	 * Structure representing the session data on the backend, used for updating session data.
	 */
	TSharedPtr<FAccelByteModelsV2BaseSession> BackendSessionData{nullptr};

	/**
	 * Structure representing the session data update from the backend
	 */
	TSharedPtr<FAccelByteModelsV2BaseSession> LatestBackendSessionDataUpdate{nullptr};

	/**
	 * Flag indicating whether the session has recieved a DS update and should trigger the
	 * server update delegate
	 */
	bool bDSReadyUpdateReceived{false};

	/**
	 * ID of the session that this information is for
	 */
	TSharedRef<const FUniqueNetIdAccelByteResource> SessionId;

	/**
	 * IP address for this session, if session type is not P2P
	 */
	TSharedPtr<FInternetAddr> HostAddress{nullptr};

	/**
	 * Remote ID of the P2P player hosting this session
	 */
	FString PeerId{};

	/**
	 * Team assignments for this session, only used for game session
	 */
	TArray<FAccelByteModelsV2GameSessionTeam> Teams{};

	/**
	 * Array of user IDs corresponding to players that are joined to this session
	 */
	TArray<FUniqueNetIdRef> JoinedMembers{};

	/**
	 * Players that have been invited to this session
	 */
	TArray<FUniqueNetIdRef> InvitedPlayers{};

	/**
	 * ID of the leader of this session. Only will be valid for party sessions.
	 */
	FUniqueNetIdPtr LeaderId{};

};

/**
 * Enum used to tell the P2P connection finished delegate what action to trigger
 */
UENUM(BlueprintType)
enum class EOnlineSessionP2PConnectedAction : uint8
{
	Join,
	Update
};

/**
 * Structure representing a session that was restored through Session::GetMyGameSessions or Session::GetMyParties
 */
struct ONLINESUBSYSTEMACCELBYTE_API FOnlineRestoredSessionAccelByte
{
	FOnlineRestoredSessionAccelByte()
	{
	}

	FOnlineRestoredSessionAccelByte(const EAccelByteV2SessionType& InSessionType, const FOnlineSessionSearchResult& InSession)
		: SessionType(InSessionType)
		, Session(InSession)
	{
	}

	/** Session type of the restored session */
	EAccelByteV2SessionType SessionType{EAccelByteV2SessionType::Unknown};

	/** Search result of the restored session */
	FOnlineSessionSearchResult Session{};
};

/**
 * Structure representing a session invite
 */
struct ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionInviteAccelByte
{
	/** Type of session that this invite is for */
	EAccelByteV2SessionType SessionType{EAccelByteV2SessionType::Unknown};

	/** ID of the user that sent this invite, could be nullptr */
	FUniqueNetIdPtr SenderId{nullptr};

	/** Search result containing this session for join */
	FOnlineSessionSearchResult Session{};
};

/**
 * AccelByte specific subclass for an online session search handle. Stores ticket ID and matchmaking user ID for retrieval later.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionSearchAccelByte : public FOnlineSessionSearch
{
public:
	FOnlineSessionSearchAccelByte() = default;
	FOnlineSessionSearchAccelByte(const TSharedRef<FOnlineSessionSearch>& InBaseSearch);

	FUniqueNetIdPtr GetSearchingPlayerId() const;
	FString GetTicketId() const;
	FName GetSearchingSessionName() const;

PACKAGE_SCOPE:
	/**
	 * ID of the player that is currently searching for a match with this handle.
	 */
	FUniqueNetIdPtr SearchingPlayerId{nullptr};

	/**
	 * ID of the ticket a player is apart of for the search
	 */
	FString TicketId{};

	/**
	 * Name of the local session that we intend to matchmake for
	 */
	FName SearchingSessionName{};

};

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
/**
 * Delegate from UE 4.26+ that is used to signal if the members in a session change. Bringing back to 4.25 or lower
 * for back compat reasons. Callers will have to instead register this delegate in our session interface rather than
 * the base session interface.
 *
 * @param SessionName The name of the session that changed
 * @param UniqueId The ID of the user whose join state has changed
 * @param bJoined If true this is a join event, (if false it is a leave event)
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSessionParticipantsChange, FName, const FUniqueNetId&, bool);
typedef FOnSessionParticipantsChange::FDelegate FOnSessionParticipantsChangeDelegate;

/**
 * Structure from UE 4.26+ for matchmaking results on start matchmaking. Just a stub for back compat with lower versions.
 */
struct FSessionMatchmakingResults
{
};

/**
 * Delegate from UE 4.26+ that is used to signal that a call to start matchmaking has completed. Bringing back to 4.25
 * or lower for back compat reasons.
 *
 * @param SessionName the name of the session that was passed to StartMatchmaking
 * @param ErrorDetails extended details of the failure (if failed)
 * @param Results results of matchmaking (if succeeded)
 */
DECLARE_DELEGATE_ThreeParams(FOnStartMatchmakingComplete, FName /*SessionName*/, const struct FOnlineError& /*ErrorDetails*/, const struct FSessionMatchmakingResults& /*Results*/);

/**
 * Structure for a matchmaking user from UE 4.26+. Defined here for back compat reasons with 4.25 or lower.
 */
struct FSessionMatchmakingUser
{
	/** Id of the user */
	TSharedRef<const FUniqueNetId> UserId;
	/** Attributes for the user */
	FOnlineKeyValuePairs<FString, FVariantData> Attributes;
};
#endif

/**
 * Convenience method to convert a single user ID for a start matchmaking request into a array of matchmaking users,
 * containing the single user.
 */
#define USER_ID_TO_MATCHMAKING_USER_ARRAY(UserId) TArray<FSessionMatchmakingUser>{{UserId}}

// Begin custom delegates
DECLARE_DELEGATE_TwoParams(FOnRestorePartySessionsComplete, const FUniqueNetId& /*LocalUserId*/, const FOnlineError& /*Result*/);
DECLARE_DELEGATE_TwoParams(FOnRestoreActiveSessionsComplete, const FUniqueNetId& /*LocalUserId*/, const FOnlineError& /*Result*/);
DECLARE_DELEGATE_TwoParams(FOnPromotePartySessionLeaderComplete, const FUniqueNetId& /*PromotedUserId*/, const FOnlineError& /*Result*/);
DECLARE_DELEGATE_OneParam(FOnRegisterServerComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnUnregisterServerComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_TwoParams(FOnLeaveSessionComplete, bool /*bWasSuccessful*/, FString /*SessionId*/);
DECLARE_DELEGATE_OneParam(FOnRefreshSessionComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnRejectSessionInviteComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnAcceptBackfillProposalComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnRejectBackfillProposalComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnSessionMemberStatusUpdateComplete, bool /*bWasSuccessful*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerReceivedSession, FName /*SessionName*/);
typedef FOnServerReceivedSession::FDelegate FOnServerReceivedSessionDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQueryAllInvitesComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*PlayerId*/);
typedef FOnQueryAllInvitesComplete::FDelegate FOnQueryAllInvitesCompleteDelegate;

DECLARE_MULTICAST_DELEGATE(FOnInviteListUpdated);
typedef FOnInviteListUpdated::FDelegate FOnInviteListUpdatedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnV2SessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FOnlineSessionInviteAccelByte& /*Invite*/);
typedef FOnV2SessionInviteReceived::FDelegate FOnV2SessionInviteReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionServerUpdate, FName /*SessionName*/);
typedef FOnSessionServerUpdate::FDelegate FOnSessionServerUpdateDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionServerError, FName /*SessionName*/, const FString& /*ErrorMessage*/);
typedef FOnSessionServerError::FDelegate FOnSessionServerErrorDelegate;

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingStarted)
typedef FOnMatchmakingStarted::FDelegate FOnMatchmakingStartedDelegate;

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingExpired)
typedef FOnMatchmakingExpired::FDelegate FOnMatchmakingExpiredDelegate;

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingCanceled)
typedef FOnMatchmakingCanceled::FDelegate FOnMatchmakingCanceledDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBackfillProposalReceived, FAccelByteModelsV2MatchmakingBackfillProposalNotif /*Proposal*/);
typedef FOnBackfillProposalReceived::FDelegate FOnBackfillProposalReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionInvitesChanged, FName /*SessionName*/);
typedef FOnSessionInvitesChanged::FDelegate FOnSessionInvitesChangedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnKickedFromSession, FName /*SessionName*/);
typedef FOnKickedFromSession::FDelegate FOnKickedFromSessionDelegate;
//~ End custom delegates

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionV2AccelByte : public IOnlineSession, public TSharedFromThis<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe>
{
public:
	virtual ~FOnlineSessionV2AccelByte() {}

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	// Begin IOnlineSession overrides
	TSharedPtr<const FUniqueNetId> CreateSessionIdFromString(const FString& SessionIdStr) override;
	class FNamedOnlineSession* GetNamedSession(FName SessionName) override;
	void RemoveNamedSession(FName SessionName) override;
	bool HasPresenceSession() override;
	EOnlineSessionState::Type GetSessionState(FName SessionName) const override;
	bool CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	bool CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	bool StartSession(FName SessionName) override;
	bool UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData = true) override;
	bool EndSession(FName SessionName) override;
	bool DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate = FOnDestroySessionCompleteDelegate()) override;
	bool IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId) override;
	bool StartMatchmaking(const TArray<TSharedRef<const FUniqueNetId>>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 25)
	bool StartMatchmaking(const TArray<FSessionMatchmakingUser>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnStartMatchmakingComplete& CompletionDelegate);
#else
	bool StartMatchmaking(const TArray<FSessionMatchmakingUser>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnStartMatchmakingComplete& CompletionDelegate) override;
#endif
	bool CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName) override;
	bool CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName) override;
	bool FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	bool FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	bool FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate) override;
	bool CancelFindSessions() override;
	bool PingSearchResults(const FOnlineSessionSearchResult& SearchResult) override;
	bool JoinSession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	bool JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	bool FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend) override;
	bool FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend) override;
	bool FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& FriendList) override;
	bool SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend) override;
	bool SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend) override;
	bool SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends) override;
	bool SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends) override;
	bool GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType = NAME_GamePort) override;
	bool GetResolvedConnectString(const class FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo) override;
	FOnlineSessionSettings* GetSessionSettings(FName SessionName) override;
	bool RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited) override;
	bool RegisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players, bool bWasInvited = false) override;
	bool UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId) override;
	bool UnregisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players) override;
	void RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate) override;
	void UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate) override;
	int32 GetNumSessions() override;
	void DumpSessionState() override;
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27)
	void RemovePlayerFromSession(int32 LocalUserNum, FName SessionName, const FUniqueNetId& TargetPlayerId) override;
#endif
	//~ End IOnlineSession overrides

	/**
	 * Query for any pending invites that the player passed in has yet to respond to.
	 */
	bool QueryAllInvites(const FUniqueNetId& PlayerId);

	/**
	 * Get every pending invite that has not yet been acted on
	 */
	TArray<FOnlineSessionInviteAccelByte> GetAllInvites() const;

	/**
	 * Get every pending game session invite that has not yet been acted on
	 */
	TArray<FOnlineSessionInviteAccelByte> GetAllGameInvites() const;

	/**
	 * Get every pending party invite that has not yet been acted on
	 */
	TArray<FOnlineSessionInviteAccelByte> GetAllPartyInvites() const;

	/**
	 * Rejects any invite that was sent to the player passed in.
	 */
	bool RejectInvite(const FUniqueNetId& PlayerId, const FOnlineSessionInviteAccelByte& InvitedSession, const FOnRejectSessionInviteComplete& Delegate= FOnRejectSessionInviteComplete());

	/**
	 * Query the backend for any session that the player is marked as active in. This is intended to be used to reconcile
	 * state between backend and client. Typically, this means that at login we want to grab all of the sessions where the
	 * player is marked as active to figure out if they are still considered to be in any sessions. From here, it is up
	 * to you if you want to rejoin the session through JoinSession, or leave the session entirely through the custom
	 * LeaveSession call.
	 * 
	 * @param LocalUserId ID of the user that we are restoring sessions for
	 * @param Delegate Handler fired after the restore call either succeeds or fails
	 */
	bool RestoreActiveSessions(const FUniqueNetId& LocalUserId, const FOnRestoreActiveSessionsComplete& Delegate = FOnRestoreActiveSessionsComplete());

	/**
	 * Grab every restored session from our cache
	 */
	TArray<FOnlineRestoredSessionAccelByte> GetAllRestoredSessions() const;

	/**
	 * Grab every restored party session from our cache
	 */
	TArray<FOnlineRestoredSessionAccelByte> GetAllRestoredPartySessions() const;

	/**
	 * Grab every restored game session from our cache
	 */
	TArray<FOnlineRestoredSessionAccelByte> GetAllRestoredGameSessions() const;

	/**
	 * Leave a restored session on the backend. For party sessions, this will allow you to create a new party instance
	 * once this call is finished. Though, this should also be done for game sessions that will not be rejoined.
	 */
	bool LeaveRestoredSession(const FUniqueNetId& LocalUserId, const FOnlineRestoredSessionAccelByte& SessionToLeave, const FOnLeaveSessionComplete& Delegate);

	/**
	 * Returns whether the local user is in a party session or not
	 */
	bool IsInPartySession() const;

	/**
	 * Get the party session instance that the player is in, or nullptr if they are not in one
	 */
	FNamedOnlineSession* GetPartySession() const;

	/*
	 * Attempt to get a named session instance by it's backend ID
	 */
	FNamedOnlineSession* GetNamedSessionById(const FString& SessionIdString);

	/**
	 * Register a dedicated server to Armada, either as a local server for testing, or as a managed Armada pod.
	 * This will also establish a connection with the DSHub service for session updates.
	 */
	void RegisterServer(FName SessionName, const FOnRegisterServerComplete& Delegate=FOnRegisterServerComplete());

	/**
	 * Unregister a dedicated server from Armada
	 */
	void UnregisterServer(FName SessionName, const FOnUnregisterServerComplete& Delegate = FOnUnregisterServerComplete());

	/**
	 * Get session type from session by its name
	 */
	EAccelByteV2SessionType GetSessionTypeByName(const FName& SessionName);

	/**
	 * Get an AccelByte session type from a session settings object
	 */
	EAccelByteV2SessionType GetSessionTypeFromSettings(const FOnlineSessionSettings& Settings) const;

	/**
	 * Try and get the ID of the user that is leader of this particular session. Only works with party sessions.
	 */
	FUniqueNetIdPtr GetSessionLeaderId(const FNamedOnlineSession* Session) const;

	/**
	 * Kick a member of the session out of the session.
	 */
	bool KickPlayer(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToKick);

	/**
	 * Promote a member of the party session to leader
	 * 
	 * @param LocalUserId ID of the user that we are restoring sessions for
	 * @param Delegate Handler fired after the restore call either succeeds or fails
	 */
	bool PromotePartySessionLeader(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToPromote, const FOnPromotePartySessionLeaderComplete& Delegate = FOnPromotePartySessionLeaderComplete());

	/**
	 * Get the list of regions that you are able to spin up a server in, sorted by latency to the player.
	 */
	TArray<FString> GetRegionList(const FUniqueNetId& LocalPlayerId) const;

	/**
	* Get the current session search handle that we are using for matchmaking.
	*/
	TSharedPtr<FOnlineSessionSearchAccelByte> GetCurrentMatchmakingSearchHandle() const;

	/**
	 * Get the current amount of players in the session specified.
	 */
	int32 GetSessionPlayerCount(const FName& SessionName) const;

	/**
	 * Get the current amount of players in the session specified.
	 */
	int32 GetSessionPlayerCount(const FOnlineSession& Session) const;

	/**
	 * Get the maximum amount of players allowed in the session specified.
	 */
	int32 GetSessionMaxPlayerCount(const FName& SessionName) const;

	/**
	 * Get the maximum amount of players allowed in the session specified.
	 */
	int32 GetSessionMaxPlayerCount(const FOnlineSession& Session) const;

	/**
	 * Set the maximum amount of players allowed in the session specified. After calling this, UpdateSession must be called
	 * for this to take effect on backend.
	 */
	bool SetSessionMaxPlayerCount(const FName& SessionName, int32 NewMaxPlayerCount) const;

	/**
	 * Set the maximum amount of players allowed in the session specified. After calling this, UpdateSession must be called
	 * for this to take effect on backend.
	 */
	bool SetSessionMaxPlayerCount(FOnlineSession* Session, int32 NewMaxPlayerCount) const;

	/**
	 * Method to manually refresh a session's data from the backend. Use to reconcile state between client and backend if
	 * notifications are missed or other issues arise.
	 * 
	 * @param SessionName Name of the session that we want to refresh data for
	 * @param Delegate Delegate fired when we finish refreshing data for this session
	 * @returns true if call was made to refresh session, false otherwise
	 */
	bool RefreshSession(const FName& SessionName, const FOnRefreshSessionComplete& Delegate);

	/**
	 * Accept a backfill proposal from matchmaking. Which will then invite the players backfilled to your session.
	 */
	bool AcceptBackfillProposal(const FName& SessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Proposal, bool bStopBackfilling, const FOnAcceptBackfillProposalComplete& Delegate);

	/**
	 * Reject a backfill proposal from received from matchmaker. Players included in the proposal will not be added to the session.
	 */
	bool RejectBackfillProposal(const FName& SessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Proposal, bool bStopBackfilling, const FOnRejectBackfillProposalComplete& Delegate);

	/**
	 * Update the status of a member in a session. Intended to be used by the server to mark a player as connected or left.
	 * 
	 * Requires permission 'ADMIN:NAMESPACE:{namespace}:SESSION:GAME' to be set with action 'UPDATE'.
	 */
	bool UpdateMemberStatus(FName SessionName, const FUniqueNetId& PlayerId, const EAccelByteV2SessionMemberStatus& Status, const FOnSessionMemberStatusUpdateComplete& Delegate=FOnSessionMemberStatusUpdateComplete());

	/**
	 * Delegate fired when we have retrieved information on the session that our server is claimed by on the backend.
	 *
	 * @param SessionName the name that our server session is stored under
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnServerReceivedSession, FName /*SessionName*/);

	/**
	 * Delegate fired when we have finished restoring all invites from the backend to the local interface.
	 *
	 * @param bWasSuccessful whether the call to get all invites was a success
	 * @param PlayerId ID of the player who had their invites restored
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnQueryAllInvitesComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*PlayerId*/);

	/**
	 * Delegate fired when we get a session invite from another player. Includes an AccelByte invite structure to allow for rejecting the invite.
	 * 
	 * @param UserId ID of the local user that received the invite
	 * @param FromId ID of the remote user that sent the invite
	 * @param Invite Invite structure that can be used to either accept or reject the invite
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnV2SessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FOnlineSessionInviteAccelByte& /*Invite*/);

	/**
	 * Delegate fired when our local list of invites has been updated
	 */
	DEFINE_ONLINE_DELEGATE(OnInviteListUpdated);

	/**
	 * Delegate fired when the server info for the session referenced changes. Basically is a signal that the server is
	 * ready to be traveled to.
	 * 
	 * @param SessionName Name of the session that has had server information change.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionServerUpdate, FName /*SessionName*/);

	/**
	 * Delegate fired when session failed to get a server.
	 * 
	 * @param SessionName Name of the session.
	 * @param ErrorMessage Message of the error
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnSessionServerError, FName /*SessionName*/, const FString& /*ErrorMessage*/);

	/**
	 * Delegate fired when matchmaking has started
	 */
	DEFINE_ONLINE_DELEGATE(OnMatchmakingStarted);

	/**
	 * Delegate fired when matchmaking ticket has expired.
	 */
	DEFINE_ONLINE_DELEGATE(OnMatchmakingExpired);

	/**
	 * Delegate fired when matchmaking ticket has been canceled.
	 */
	DEFINE_ONLINE_DELEGATE(OnMatchmakingCanceled);

	/**
	 * Delegate fired when the game server receives a backfill proposal from matchmaking. Use AcceptBackfillProposal and
	 * RejectBackfillProposal to act on the proposal.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnBackfillProposalReceived, FAccelByteModelsV2MatchmakingBackfillProposalNotif /*Proposal*/);

	/**
	 * Delegate fired when a session's invited members list changes.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionInvitesChanged, FName /*SessionName*/);

	/**
	 * Delegate fired when a local player has been kicked from a session. Fired before session is destroyed in case there
	 * is a need to do any extra clean up.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnKickedFromSession, FName /*SessionName*/);

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 25)
	/**
	 * Delegate fired when the members in a session have changed. From the UE 4.26+ base session interface delegates.
	 * Brought back to 4.25 or lower for back compat reasons.
	 * 
	 * @param SessionName The name of the session that changed
	 * @param UniqueId The ID of the user whose join state has changed
	 * @param bJoined if true this is a join event, (if false it is a leave event)
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnSessionParticipantsChange, FName, const FUniqueNetId&, bool);
#endif

PACKAGE_SCOPE:
	/** Restored sessions stored in this interface */
	TArray<FOnlineRestoredSessionAccelByte> RestoredSessions;

	/** Session invites stored in this interface */
	TArray<FOnlineSessionInviteAccelByte> SessionInvites;

	/**
	 * Current session search handle that we are using for matchmaking.
	 * 
	 * #NOTE (Maxwell): Since we only track one handle at a time, we don't support matchmaking for two different session names at the same time.
	 * This might be something useful down the line, think about if this is possible to support.
	 */
	TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle{nullptr};

	/**
	 * Session settings that we will create the matchmaking session result with.
	 */
	FOnlineSessionSettings CurrentMatchmakingSessionSettings{};

	/**
	 * Global string for the environment variable to get session ID for a spawned server.
	 */
	static const FString ServerSessionIdEnvironmentVariable;

	FOnlineSessionV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/**
	 * Initializer method for this interface. Used to define internal delegate handlers that don't require user identifiers.
	 */
	void Init();

	/**
	 * Session tick for various background tasks
	 */
	void Tick(float DeltaTime);

	/**
	 * Register notification delegates for the player specified. Will also fire off associated OSS delegates once received.
	 */
	void RegisterSessionNotificationDelegates(const FUniqueNetId& PlayerId);

	/**
	 * Get an AccelByte joinability enum from a string value
	 */
	EAccelByteV2SessionJoinability GetJoinabilityFromString(const FString& JoinabilityStr) const;

	/**
	 * Get an AccelByte session type from a string value
	 */
	EAccelByteV2SessionType GetSessionTypeFromString(const FString& SessionTypeStr) const;

	/**
	 * Figure out the AccelByte joinability status of a session from its settings
	 */
	EAccelByteV2SessionJoinability GetJoinabiltyFromSessionSettings(const FOnlineSessionSettings& Settings) const;

	/**
	 * Get a string representation of the joinability enum passed in
	 */
	FString GetJoinabilityAsString(const EAccelByteV2SessionJoinability& Joinability);
	
	/**
	 * Get an AccelByte joinability enum from a string value
	 */
	FString GetServerTypeAsString(const EAccelByteV2SessionConfigurationServerType& ServerType) const;

	/**
	 * Get an AccelByte joinability enum from a string value
	 */
	EAccelByteV2SessionConfigurationServerType GetServerTypeFromString(const FString& ServerType) const;

	/**
	 * Create a new named game session instance based on the passed in settings and backend session structure
	 */
	void FinalizeCreateGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& BackendSessionInfo);

	/**
	* Create a new named party session instance based on the passed in settings and backend session structure
	*/
	void FinalizeCreatePartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& BackendSessionInfo);

	/**
	 * Construct a new session search result instance from a backend representation of a game session
	 */
	bool ConstructGameSessionFromBackendSessionModel(const FAccelByteModelsV2GameSession& BackendSession, FOnlineSession& OutResult);

	/**
	 * Construct a new session search result instance from a backend representation of a party session
	 */
	bool ConstructPartySessionFromBackendSessionModel(const FAccelByteModelsV2PartySession& BackendSession, FOnlineSession& OutResult);

	/**
	 * Checks whether or not the attribute name passed in is intended to be ignored for session attributes on backend.
	 */
	bool ShouldSkipAddingFieldToSessionAttributes(const FName& FieldName) const;

	/**
	 * Attempt to get the currently bound local address for a dedicated server.
	 */
	bool GetServerLocalIp(FString& OutIp) const;

	/**
	 * Attempt to get the currently bound port for a dedicated server.
	 */
	bool GetServerPort(int32& OutPort) const;

	/**
	 * Attempt to get the name of the local server to register with
	 */
	bool GetLocalServerName(FString& OutServerName) const;

	/**
	 * Attempt to get the session that is currently associated with this dedicated server. Will create a new game session
	 * under NAME_GameSession for the developer to interface with.
	 */
	bool GetServerClaimedSession(FName SessionName, const FString& SessionId);

	/**
	 * Remove a restored session instance by it's ID, if an instance exists
	 * 
	 * @param SessionIdStr Session ID that we are trying to find and remove a restore session instance for
	 * @return bool true if found and removed, false otherwise
	 */
	bool RemoveRestoreSessionById(const FString& SessionIdStr);

	/**
	 * Remove an invite from our local list by it's ID. Used when we accept or reject an invite.
	 * 
	 * @param SessionIdStr Session ID that we are trying to find and remove an invite instance for
	 * @return bool true if found and removed, false otherwise
	 */
	bool RemoveInviteById(const FString& SessionIdStr);

	/**
	 * Send a request on the backend to leave a session by its ID. Used internally by LeaveRestoredSession and DestroySession
	 * to notify backend that we want to leave the session referenced.
	 *
	 * @param LocalUserId ID of the user that is leaving the session provided
	 * @param SessionId ID of the session that we want to leave
	 * @param Delegate Delegate fired once the leave session call completes
	 * @return bool true if leave session task was spawned, false otherwise
	 */
	bool LeaveSession(const FUniqueNetId& LocalUserId, const EAccelByteV2SessionType& SessionType, const FString& SessionId, const FOnLeaveSessionComplete& Delegate=FOnLeaveSessionComplete());

	/**
	 * Convert an array of doubles to an array of FJsonValues
	 *
	 * @param Array The source array to convert
	 * @return An array of shared pointers to FJsonValues
	 */
	TArray<TSharedPtr<FJsonValue>> ConvertSessionSettingArrayToJson(const TArray<double>& Array) const;

	/**
	 * Convert an array of FStrings to an array of FJsonValues
	 *
	 * @param Array The source array to convert
	 * @return An array of shared pointers to FJsonValues
	 */
	TArray<TSharedPtr<FJsonValue>> ConvertSessionSettingArrayToJson(const TArray<FString>& Array) const;

	/**
	 * Convert a JSON array into a primitive array of FStrings
	 *
	 * @param JsonArray The array to convert, pulled out of a JSON object
	 * @param OutArray The array of strings to write to
	 * @return A boolean indicating whether the conversion was successful
	 */
	bool ConvertSessionSettingJsonToArray(const TArray<TSharedPtr<FJsonValue>>* JsonArray, TArray<FString>& OutArray) const;

	/**
	 * Convert a JSON array into a primitive array of doubles
	 *
	 * @param JsonArray The array to convert, pulled out of a JSON object
	 * @param OutArray The array of doubles to write to
	 * @return A boolean indicating whether the conversion was successful
	 */
	bool ConvertSessionSettingJsonToArray(const TArray<TSharedPtr<FJsonValue>>* JsonArray, TArray<double>& OutArray) const;

	/**
	 * Convert a session settings object into a JSON object that can be used with create or update requests for sessions.
	 */
	TSharedRef<FJsonObject> ConvertSessionSettingsToJsonObject(const FOnlineSessionSettings& Settings) const;

	/**
	 * Read a JSON object into a session settings instance
	 */
	FOnlineSessionSettings ReadSessionSettingsFromJsonObject(const TSharedRef<FJsonObject>& Object) const;
	
	/**
	 * Convert a session search parameters into a json object that can be used to fill match ticket attributes
	 */
	TSharedRef<FJsonObject> ConvertSearchParamsToJsonObject(const FSearchParams& Params) const;

	/**
	 * Attempt to connect to a P2P session
	 */
	void ConnectToJoinedP2PSession(FName SessionName, EOnlineSessionP2PConnectedAction Action);

	/**
	 * Update game session data from a backend model. Used for update notifications and refreshing a game session manually.
	 */
	void UpdateInternalGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& UpdatedGameSession, bool& bIsConnectingToP2P);

	/**
	 * Update party session data from a backend model. Used for update notifications and refreshing a game session manually.
	 */
	void UpdateInternalPartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& UpdatedPartySession);

	/**
	 * Connect a server to the DS hub, as well as register delegates internally for session management.
	 * 
	 * @param ServerName Name of the server that is connecting to the DS hub
	 */
	void ConnectToDSHub(const FString& ServerName);

	/**
	 * Disconnect a server from the DS hub, unregistering any delegates bound.
	 */
	void DisconnectFromDSHub();

private:
	/** Parent subsystem of this interface instance */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Critical section to lock sessions map while accessing */
	mutable FCriticalSection SessionLock;

	/** Sessions stored in this interface, associated by session name */
	TMap<FName, TSharedPtr<FNamedOnlineSession>> Sessions;

	/** Flag denoting whether there is already a task in progress to get a session associated with a server */
	bool bIsGettingServerClaimedSession{ false };

	/** Flag denoting whether there has been an update to any of the sessions in the interface */
	bool bReceivedSessionUpdate{ false };

	/** Array of delegates that are awaiting server session retrieval before executing */
	TArray<TFunction<void()>> SessionCallsAwaitingServerSession;

	/**
	 * Pointer to SessionSearch.
	 * will be set when matchmaking is in progress 
	 */
	TSharedPtr<FOnlineSessionSearch> SessionSearchPtr;

	/** Default session setting to use when no CustomSessionSetting is provided */
	FOnlineSessionSettings DefaultSessionSettings;

	/** 
	 * Session setting provided by user when matchmaking,
	 * if provided, CustomSessionSetting will be prioritized then DefaultSessionSetting.
	 * These will be cleared when matchmaking is complete and session search result is constructed
	 */
	TSharedPtr<FOnlineSessionSettings> ExplicitSessionSettings;

	/** Cache matchmaking ticket after started matchmaking. to be used to cancel matchmaking process */
	FString MatchmakingTicketId;

	/** Hidden on purpose */
	FOnlineSessionV2AccelByte() :
		AccelByteSubsystem(nullptr)
	{
	}

	bool CreatePartySession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings);
	bool CreateGameSession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings, bool bSendCreateRequest=true);

	//~ Begin Game Session Notification Handlers
	void OnInvitedToGameSessionNotification(FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent, int32 LocalUserNum);
	void OnFindGameSessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent);
	void OnGameSessionMembersChangedNotification(FAccelByteModelsV2GameSessionMembersChangedEvent MembersChangedEvent, int32 LocalUserNum);
	void OnGameSessionUpdatedNotification(FAccelByteModelsV2GameSession UpdatedGameSession, int32 LocalUserNum);
	void OnKickedFromGameSessionNotification(FAccelByteModelsV2GameSessionUserKickedEvent KickedEvent, int32 LocalUserNum);
	void OnDsStatusChangedNotification(FAccelByteModelsV2DSStatusChangedNotif DsStatusChangeEvent, int32 LocalUserNum);
	//~ End Game Session Notification Handlers

	//~ Begin Party Session Notification Handlers
	void OnInvitedToPartySessionNotification(FAccelByteModelsV2PartyInvitedEvent InviteEvent, int32 LocalUserNum);
	void OnKickedFromPartySessionNotification(FAccelByteModelsV2PartyUserKickedEvent KickedEvent, int32 LocalUserNum);
	void OnFindPartySessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2PartyInvitedEvent InviteEvent);
	void OnPartySessionMembersChangedNotification(FAccelByteModelsV2PartyMembersChangedEvent MemberChangeEvent, int32 LocalUserNum);
	void OnPartySessionUpdatedNotification(FAccelByteModelsV2PartySession UpdatedPartySession, int32 LocalUserNum);
	void OnPartySessionInviteRejectedNotification(FAccelByteModelsV2PartyUserRejectedEvent RejectEvent, int32 LocalUserNum);
	//~ End Party Session Notification Handlers

	//~ Begin Matchmaking Notification Handlers
	void OnMatchmakingStartedNotification(FAccelByteModelsV2StartMatchmakingNotif MatchmakingStartedNotif, int32 LocalUserNum);
	void OnMatchmakingMatchFoundNotification(FAccelByteModelsV2MatchFoundNotif MatchFoundEvent, int32 LocalUserNum);
	void OnMatchmakingExpiredNotification(FAccelByteModelsV2MatchmakingExpiredNotif MatchmakingExpiredNotif, int32 LocalUserNum);
	void OnMatchmakingCanceledNotification(FAccelByteModelsV2MatchmakingCanceledNotif MatchmakingCanceledNotif, int32 LocalUserNum);
	void OnFindMatchmakingGameSessionByIdComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result);
	//~ End Matchmaking Notification Handlers

	//~ Begin Server Notification Handlers
	void OnServerClaimedNotification(const FAccelByteModelsServerClaimedNotification& Notification);
	void OnV2BackfillProposalNotification(const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Notification);
	//~ End Server Notification Handlers

	void UpdateSessionMembers(FNamedOnlineSession* Session, const TArray<FAccelByteModelsV2SessionUser>& PreviousMembers, const bool bHasInvitedPlayersChanged);

	void RegisterJoinedSessionMember(FNamedOnlineSession* Session, const FAccelByteModelsV2SessionUser& JoinedMember);
	void UnregisterLeftSessionMember(FNamedOnlineSession* Session, const FAccelByteModelsV2SessionUser& LeftMember);

	void OnServerReceivedSessionComplete_Internal(FName SessionName);

	/**
	 * Sets up socket subsystem for a P2P connection and runs set up on the subsystem level
	 */
	void SetupAccelByteP2PConnection(const FUniqueNetId& LocalPlayerId);

	/**
	 * Tears down the socket subsystem for a P2P connection, called on destroy session
	 */
	void TeardownAccelByteP2PConnection();

	/**
	 * Delegate handler when we finish connecting to a P2P ICE session
	 */
	void OnICEConnectionComplete(const FString& PeerId, bool bStatus, FName SessionName, EOnlineSessionP2PConnectedAction Action);

	/**
	 * Internal method to get a named session as a const pointer.
	 */
	FNamedOnlineSession* GetNamedSession(FName SessionName) const;

	/**
	 * Enqueue a session data update for the next tick if its version is more up-to-date than existing data
	 */
	void EnqueueBackendDataUpdate(const FName& SessionName, const TSharedPtr<FAccelByteModelsV2BaseSession>& SessionData, const bool bIsDSReadyUpdate=false);

protected:
	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings) override;
	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSession& Session) override;

	// Making this async task a friend so that it can add new named sessions
	friend class FOnlineAsyncTaskAccelByteGetServerClaimedV2Session;
};

typedef TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> FOnlineSessionV2AccelBytePtr;
