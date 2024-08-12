// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
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
#include "OnlineErrorAccelByte.h"
#include "Models/AccelByteSessionModels.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "Models/AccelByteDSHubModels.h"
#include "AccelByteNetworkingStatus.h"
#include "Core/StatsD/IAccelByteStatsDMetricCollector.h"
#include "GameServerApi/AccelByteServerMetricExporterApi.h"
#include "OnlineSubsystemAccelBytePackage.h"

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

	/** Get the locally stored backend session data for this session as game session */
	TSharedPtr<FAccelByteModelsV2GameSession> GetBackendSessionDataAsGameSession() const;

	/** Get the locally stored backend session data for this session as a party session */
	TSharedPtr<FAccelByteModelsV2PartySession> GetBackendSessionDataAsPartySession() const;

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

	/** Get PartyID a sesion member belong to, returns empty string if not found */
	FString GetMemberPartyId(const FUniqueNetIdRef& UserId) const;

	/**
	 * Attempt to find a member of this session by their ID
	 */
	bool FindMember(const FUniqueNetId& MemberId, FAccelByteModelsV2SessionUser*& OutMember);

	/**
	 * Check if this session contains a member with the provided ID at all
	 */
	bool ContainsMember(const FUniqueNetId& MemberId);

	/** Set session leader storage. */
	void SetSessionLeaderStorage(const FJsonObjectWrapper& Data);

	/** Set session member storage. */
	void SetSessionMemberStorage(const FUniqueNetIdRef& UserId, const FJsonObjectWrapper& Data);

	/** Get session leader storage. */
	bool GetSessionLeaderStorage(FJsonObjectWrapper& OutStorage) const;

	/** Get session member storage. */
	bool GetSessionMemberStorage(const FUniqueNetIdRef& UserId, FJsonObjectWrapper& OutStorage) const;

	/** Get all session member storage. */
	bool GetAllSessionMemberStorage(TUniqueNetIdMap<FJsonObjectWrapper>& OutStorage) const;

PACKAGE_SCOPE:
	/**
	 * Update the list of invited players on this session from the backend session data.
	 *
	 * @param bOutJoinedMembersChanged Boolean denoting whether the array of joined members has changed
	 * @param bOutInvitedPlayersChanged Boolean denoting whether the array of invited players has changed
	 */
	void UpdatePlayerLists(bool& bOutJoinedMembersChanged, bool& bOutInvitedPlayersChanged);

	/**
	 * Update the stored leader ID for this session.
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
	 * Set the p2p channel for the connection
	 */
	void SetP2PChannel(int32 InChannel);

	/**
	 * Check if the session is P2P matchmaking
	 */
	bool IsP2PMatchmaking();

private:
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
	 * ID of the leader of this session.
	 */
	FUniqueNetIdPtr LeaderId{};

	/** Map of Members belonging to which party, key is User ID and value is Party ID **/
	TMap<FString, FString> MemberParties;

	int32 P2PChannel{0};

	/**
	 * Static cast the given base session pointer to a game session pointer if type is valid
	 */
	TSharedPtr<FAccelByteModelsV2GameSession> StaticCastAsGameSession(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBaseSession) const;

	/**
	 * Static cast the given base session pointer to a party session pointer if type is valid
	 */
	TSharedPtr<FAccelByteModelsV2PartySession> StaticCastAsPartySession(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBaseSession) const;

	/** Session leader storage. */
	FJsonObjectWrapper SessionLeaderStorage;

	/** Session members storage. */
	TUniqueNetIdMap<FJsonObjectWrapper> SessionMembersStorages;
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

	/** ID of the player that restored this session */
	FUniqueNetIdPtr LocalOwnerId{nullptr};
};

/**
 * Structure representing a session invite
 */
struct ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionInviteAccelByte
{
	/** Type of session that this invite is for */
	EAccelByteV2SessionType SessionType{EAccelByteV2SessionType::Unknown};

	/** ID of the player that received this session invite */
	FUniqueNetIdPtr RecipientId{nullptr};

	/** ID of the user that sent this invite, could be nullptr */
	FUniqueNetIdPtr SenderId{nullptr};

	/** Search result containing this session for join */
	FOnlineSessionSearchResult Session{};

	/** The invitation expiration */
	FDateTime ExpiredAt{0};

	/** Check whether the invitation is already expired or not */
	bool IsExpired();
};

struct ONLINESUBSYSTEMACCELBYTE_API FSessionServerCheckPollItem
{
	/** User ID of the player currently waiting for DS */
	FUniqueNetIdPtr SearchingPlayerId;

	/** Named session waiting to get DS */
	FName SessionName;

	/** Time in UTC the next session server check will be executed */
	FDateTime NextPollTime;
};

struct ONLINESUBSYSTEMACCELBYTE_API FSessionInviteCheckPollItem
{
	/** User ID of the player currently waiting for DS */
	FUniqueNetIdPtr SearchingPlayerId;

	/** Named session waiting to get DS */
	FString SessionId;

	/** Time in UTC the next session server check will be executed */
	FDateTime NextPollTime;
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
	TSharedPtr<FJsonObject>& GetSearchStorage();
	bool GetIsP2PMatchmaking() const;
	void SetIsP2PMatchmaking(const bool IsP2PMatchmaking);
	void SetSearchStorage(TSharedPtr<FJsonObject> const& JsonObject);

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

	/**
	 * Name of the match pool used in current matchmaking
	 */
	FString MatchPool{};

	/**
	 * The list of available platforms for cross platform matchmaking
	 */
	TArray<FString> CrossPlatforms{};

	/**
	 * Additional shared data not considered for matchmaking
	 */
	TSharedPtr<FJsonObject> SearchStorage{};
	
	/**
	 * Flag to check if the current matchmaking is for P2P and getting the latencies from turn server list.
	 */
	bool bIsP2PMatchmaking{false};
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

struct FOnlineSessionV2AccelBytePlayerAttributes
{
	/**
	 * Whether or not crossplay should be enabled for a player. If disabled by the system, this will always be false.
	 */
	bool bEnableCrossplay{false};

	/**
	 * Custom data that should be associated with this player between multiple sessions.
	 */
	TSharedPtr<FJsonObject> Data{nullptr};

	/**
	 * Preferred roles that associated with this player between multiple sessions.
	 */
	TArray<FString> Roles{};

	/**
	 * Preferred roles that associated with this player between multiple sessions.
	 */
	TArray<FAccelByteModelsV2PlayerAttributesPlatform> Platforms{};
};

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
DECLARE_DELEGATE_TwoParams(FOnGenerateNewPartyCodeComplete, bool /*bWasSuccessful*/, FString /*NewPartyCode*/);
DECLARE_DELEGATE_OneParam(FOnRevokePartyCodeComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_TwoParams(FOnGenerateNewGameCodeComplete, bool /*bWasSuccessful*/, FString /*NewGameCode*/);
DECLARE_DELEGATE_OneParam(FOnRevokeGameCodeComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_TwoParams(FOnKickPlayerComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*KickedPlayerId*/);
DECLARE_DELEGATE_OneParam(FOnCreateBackfillTicketComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnDeleteBackfillTicketComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_TwoParams(FOnUpdatePlayerAttributesComplete, const FUniqueNetId& /*LocalPlayerId*/, bool /*bWasSuccessful*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerReceivedSession, FName /*SessionName*/);
typedef FOnServerReceivedSession::FDelegate FOnServerReceivedSessionDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQueryAllInvitesComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*PlayerId*/);
typedef FOnQueryAllInvitesComplete::FDelegate FOnQueryAllInvitesCompleteDelegate;

DECLARE_MULTICAST_DELEGATE(FOnInviteListUpdated);
typedef FOnInviteListUpdated::FDelegate FOnInviteListUpdatedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnV2SessionInviteTimeoutReceived, const FUniqueNetId& /*UserId*/, const FOnlineSessionInviteAccelByte& /*Invite*/, EAccelByteV2SessionType /*SessionType*/);
typedef FOnV2SessionInviteTimeoutReceived::FDelegate FOnV2SessionInviteTimeoutReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnV2SessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FOnlineSessionInviteAccelByte& /*Invite*/);
typedef FOnV2SessionInviteReceived::FDelegate FOnV2SessionInviteReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionServerUpdate, FName /*SessionName*/);
typedef FOnSessionServerUpdate::FDelegate FOnSessionServerUpdateDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionServerError, FName /*SessionName*/, const FString& /*ErrorMessage*/);
typedef FOnSessionServerError::FDelegate FOnSessionServerErrorDelegate;

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingStarted)
typedef FOnMatchmakingStarted::FDelegate FOnMatchmakingStartedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingExpired, TSharedPtr<FOnlineSessionSearchAccelByte> /*SearchHandler*/)
typedef FOnMatchmakingExpired::FDelegate FOnMatchmakingExpiredDelegate;

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingCanceled)
typedef FOnMatchmakingCanceled::FDelegate FOnMatchmakingCanceledDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchmakingCanceledReason, const FOnlineErrorAccelByte& /*Error*/)
typedef FOnMatchmakingCanceledReason::FDelegate FOnMatchmakingCanceledReasonDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBackfillProposalReceived, FAccelByteModelsV2MatchmakingBackfillProposalNotif /*Proposal*/);
typedef FOnBackfillProposalReceived::FDelegate FOnBackfillProposalReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBackfillTicketExpiredReceived, FName /*SessionName*/, FAccelByteModelsV2MatchmakingBackfillTicketExpireNotif /*BackfillExpiredTicket*/);
typedef FOnBackfillTicketExpiredReceived::FDelegate FOnBackfillTicketExpiredReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionInvitesChanged, FName /*SessionName*/);
typedef FOnSessionInvitesChanged::FDelegate FOnSessionInvitesChangedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnKickedFromSession, FName /*SessionName*/);
typedef FOnKickedFromSession::FDelegate FOnKickedFromSessionDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionUpdateReceived, FName /*SessionName*/);
typedef FOnSessionUpdateReceived::FDelegate FOnSessionUpdateReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionLeaderStorageUpdateReceived, FName /*SessionName*/);
typedef FOnSessionLeaderStorageUpdateReceived::FDelegate FOnSessionLeaderStorageUpdateReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionMemberStorageUpdateReceived, FName /*SessionName*/, const FUniqueNetId& /*UpdatedMemberId*/);
typedef FOnSessionMemberStorageUpdateReceived::FDelegate FOnSessionMemberStorageUpdateReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionUpdateRequestComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
typedef FOnSessionUpdateRequestComplete::FDelegate FOnSessionUpdateRequestCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionUpdateConflictError, FName /*SessionName*/, FOnlineSessionSettings /*FailedSessionSettings*/);
typedef FOnSessionUpdateConflictError::FDelegate FOnSessionUpdateConflictErrorDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnSendSessionInviteComplete, const FUniqueNetId& /*LocalSenderId*/, FName /*SessionName*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*InviteeId*/);
typedef FOnSendSessionInviteComplete::FDelegate FOnSendSessionInviteCompleteDelegate;

DECLARE_MULTICAST_DELEGATE(FOnAMSDrainReceived);
typedef FOnAMSDrainReceived::FDelegate FOnAMSDrainReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionInviteRejected, FName /*SessionName*/, const FUniqueNetId& /*RejecterId*/);
typedef FOnSessionInviteRejected::FDelegate FOnSessionInviteRejectedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerAttributesInitialized, const FUniqueNetId& /*LocalUserId*/, bool /*bWasSuccessful*/);
typedef FOnPlayerAttributesInitialized::FDelegate FOnPlayerAttributesInitializedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnServerQueryGameSessionsComplete, const FAccelByteModelsV2PaginatedGameSessionQueryResult& /*GameSessionsQueryResult*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnServerQueryGameSessionsComplete::FDelegate FOnServerQueryGameSessionsCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnServerQueryPartySessionsComplete, const FAccelByteModelsV2PaginatedPartyQueryResult& /*PartySessionsQueryResult*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnServerQueryPartySessionsComplete::FDelegate FOnServerQueryPartySessionsCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPromoteGameSessionLeaderComplete, const FUniqueNetId& /*PromotedUserId*/, const FOnlineErrorAccelByte& /*Result*/);
typedef FOnPromoteGameSessionLeaderComplete::FDelegate FOnPromoteGameSessionLeaderCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetMyActiveMatchTicketComplete, bool /*bWasSuccessful*/, FName /*SessionName*/, TSharedPtr<FOnlineSessionSearch>& /*SearchHandler*/);
typedef FOnGetMyActiveMatchTicketComplete::FDelegate FOnGetMyActiveMatchTicketCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGetMatchTicketDetailsComplete, const FAccelByteModelsV2MatchmakingGetTicketDetailsResponse& /*TicketDetailsResult*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnGetMatchTicketDetailsComplete::FDelegate FOnGetMatchTicketDetailsCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateSessionLeaderStorageComplete, FName /*SessionName*/, const FOnlineError& /*ErrorInfo*/);
typedef FOnUpdateSessionLeaderStorageComplete::FDelegate FOnUpdateSessionLeaderStorageCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnUpdateSessionMemberStorageComplete, FName /*SessionName*/, const FUniqueNetId& /*UpdatedUserId*/, const FOnlineError& /*ErrorInfo*/);
typedef FOnUpdateSessionMemberStorageComplete::FDelegate FOnUpdateSessionMemberStorageCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSendDSSessionReadyComplete, const FOnlineError& /*ErrorInfo*/)
typedef FOnSendDSSessionReadyComplete::FDelegate FOnSendDSSessionReadyCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnV2SessionEnded, FName /*SessionName*/);
typedef FOnV2SessionEnded::FDelegate FOnV2SessionEndedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnCancelSessionInviteComplete, const FUniqueNetId& /*LocalUserId*/, FName /*SessionName*/, const FUniqueNetId& /*Invitee*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnCancelSessionInviteComplete::FDelegate FOnCancelSessionInviteCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionInviteCanceled, const FString& /*SessionID*/)
typedef FOnSessionInviteCanceled::FDelegate FOnSessionInviteCanceledDelegate;

/**
 * Delegate broadcast when a session that the player is in locally has been removed on the backend. Gives the game an
 * opportunity to clean up state.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionRemoved, FName /*SessionName*/);
typedef FOnSessionRemoved::FDelegate FOnSessionRemovedDelegate;
//~ End custom delegates

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionV2AccelByte : public IOnlineSession, public TSharedFromThis<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe>
{
public:
	virtual ~FOnlineSessionV2AccelByte() override;

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
	FUniqueNetIdPtr CreateSessionIdFromString(const FString& SessionIdStr) override;
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
	* @brief Notify the backend service that the server is already complete the prerequisite action(s) and ready to accept player.
	* Either automatically called from RegisterServer if condition met (bRegisterServerOnAutoLogin is true)
	* else call it manually after RegisterServer.
	*/
	void SendServerReady(FName SessionName, const FOnRegisterServerComplete& Delegate=FOnRegisterServerComplete());

	/**
	 * Unregister a dedicated server from Armada
	 */
	void UnregisterServer(FName SessionName, const FOnUnregisterServerComplete& Delegate = FOnUnregisterServerComplete());

	/**
	 * Set Timeout for the dedicated server.
	 */
	void SetServerTimeout(int32 NewTimeout);

	/**
	 * Reset the dedicated server's timeout
	 */
	void ResetServerTimeout();

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
	bool KickPlayer(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToKick, const FOnKickPlayerComplete& Delegate=FOnKickPlayerComplete());

	/**
	 * Promote a member of the game session to leader
	 *
	 * @param LocalUserId ID of the user that we are restoring sessions for
	 * @param SessionName Name of the game session
	 * @param PlayerIdToPromote ID of the player to be promoted as game session leader
	 */
	bool PromoteGameSessionLeader(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToPromote);

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
	 * Create a backfill ticket for the session provided. This will queue your session to be backfilled with users from
	 * matchmaking.
	 */
	bool CreateBackfillTicket(const FName& SessionName, const FOnCreateBackfillTicketComplete& Delegate);

	/**
	 * Create a backfill ticket for the session provided. This will queue your session to be backfilled with users from
	 * matchmaking in the match pool specified.
	 */
	bool CreateBackfillTicket(const FName& SessionName, const FString& MatchPool, const FOnCreateBackfillTicketComplete& Delegate);

	/**
	 * Delete a backfill ticket associated with the session provided. This will stop backfilling players from
	 * matchmaking for the session.
	 */
	bool DeleteBackfillTicket(const FName& SessionName, const FOnDeleteBackfillTicketComplete& Delegate);

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
	 * Join a session using a session code. This code is intended to be generated either by the leader or DSes, on
	 * session create, and passed to players to join the session without invites. To access an already generated
	 * code, grab the SETTING_PARTYSESSION_CODE setting out of a party session's settings.
	 */
	bool JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FString& Code, EAccelByteV2SessionType SessionType = EAccelByteV2SessionType::PartySession);

	/**
	 * Generate a new party code for this party session. Once updated, this new generated code will be reflected in the
	 * SETTING_PARTYSESSION_CODE session setting.
	 */
	bool GenerateNewPartyCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnGenerateNewPartyCodeComplete& Delegate);

	/**
	 * Revoke the code stored on a party for external joins. Once revoked, the SETTING_PARTYSESSION_CODE session setting
	 * will be blanked out and joining this party via code will not be possible.
	 */
	bool RevokePartyCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnRevokePartyCodeComplete& Delegate);

	/**
	 * Generate a new Game code for this game session. Once updated, this new generated code will be reflected in the
	 * SETTING_PARTYSESSION_CODE session setting.
	 */
	bool GenerateNewGameCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnGenerateNewGameCodeComplete& Delegate);

	/**
	 * Revoke the code stored on a game for external joins. Once revoked, the SETTING_PARTYSESSION_CODE session setting
	 * will be blanked out and joining this party via code will not be possible.
	 */
	bool RevokeGameCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnRevokeGameCodeComplete& Delegate);

	/**
	 * Set an override for a local server name to register with. Only intended for use with local dedicated servers.
	 */
	void SetLocalServerNameOverride(const FString& InLocalServerNameOverride);

	/**
	 * Set an override for a local server IP to register with. Only intended for use with local dedicated servers.
	 */
	void SetLocalServerIpOverride(const FString& InLocalServerIpOverride);

	/**
	 * Set an override for a local server name to register with. Only intended for use with local dedicated servers.
	 */
	void SetLocalServerPortOverride(int32 InLocalServerPortOverride);

	/**
	 * Get attributes for the given player. To update the stored values in the session backend service, call UpdatePlayerAttributes.
	 *
	 * @param LocalPlayerId ID of the local player that attributes should be returned for
	 */
	FOnlineSessionV2AccelBytePlayerAttributes GetPlayerAttributes(const FUniqueNetId& LocalPlayerId);

	/**
	 * Sends a request to the session service to update a player's attributes.
	 *
	 * @param LocalPlayerId ID of the local player that attributes should be updated for
	 * @param NewAttributes Structure describing what attributes should be updated for this player
	 * @param Delegate Delegate to fire after task to update attributes completes
	 */
	bool UpdatePlayerAttributes(const FUniqueNetId& LocalPlayerId, const FOnlineSessionV2AccelBytePlayerAttributes& NewAttributes, const FOnUpdatePlayerAttributesComplete& Delegate=FOnUpdatePlayerAttributesComplete());

	/**
	 * Check if the player is the host of the P2P matchmaking match
	 */
	bool IsPlayerP2PHost(const FUniqueNetId& LocalUserId, FName SessionName);

	/**
	 * Attempt to find a specific player's member settings entry within a FOnlineSessionSettings instance.
	 *
	 * @param InSettings Settings instance that we are attempting to find within
	 * @param PlayerId ID of the player that you are attempting to find a settings entry for
	 * @param OutMemberSettings Settings for the player that were found
	 * @return bool true if found, false otherwise
	 */
	bool FindPlayerMemberSettings(FOnlineSessionSettings& InSettings, const FUniqueNetId& PlayerId, FSessionSettings& OutMemberSettings) const;

	/**
	 * Attempt to find a specific player's member settings entry within a FOnlineSessionSettings instance.
	 *
	 * @param InSettings Settings instance that we are attempting to find within
	 * @param PlayerId ID of the player that you are attempting to find a settings entry for
	 * @param OutMemberSettings Settings for the player that were found
	 * @return bool true if found, false otherwise
	 */
	bool FindPlayerMemberSettings(FOnlineSessionSettings& InSettings, const FUniqueNetId& PlayerId, TSharedPtr<FSessionSettings>& OutMemberSettings) const;

	/**
	 * Update session leader storage, only session leader that can use this.
	 * This will overwrite the current session storage if successful.
	 * Listen to UpdateSessionLeaderStorageComplete event for detailed information if the storage successfully updated in backend.
	 *
	 * @param LocalUserId UniqueNetId of user who will perform session leader storage update.
	 * @param SessionName The name of the session NAME_GameSession or NAME_PartySession.
	 * @param Data The JSON data for updating the leader storage.
	 * @return true if it successfully dispatch task to update the leader session storage.
	 */
	bool UpdateSessionLeaderStorage(const FUniqueNetId& LocalUserId, FName SessionName, FJsonObjectWrapper const& Data);

	/**
	 * Update session member storage.
	 * This will overwrite the current session storage if successful.
	 * Listen to OnUpdateSessionMemberStorageComplete event for detailed information if the storage successfully updated in backend.
	 *
	 * @param LocalUserId UniqueNetId of user who will perform session leader storage update.
	 * @param SessionName The name of the session NAME_GameSession or NAME_PartySession.
	 * @param Data The JSON data for updating the member session storage.
	 * @return true if it successfully dispatch task to update the member session storage.
	 */
	bool UpdateSessionMemberStorage(const FUniqueNetId& LocalUserId, FName SessionName, FJsonObjectWrapper const& Data);

	/**
	 * Query the backend for active matchmaking tickets, Listen to OnGetMyActiveMatchTicketComplete for the result.
	 * if active ticket is found, it will fill CurrentMatchmakingSearchHandle so ticket can be cancelled with CancelMatchmaking call.
	 *
	 * @param LocalUserId ID of the player that you are attempting to find active match ticket for
	 * @param SessionName Session name we are searching active match ticket for
	 * @param MatchPool Match pool we are going to search active ticket for
	 * @return true if query request is sent, false otherwise
	 */
	bool GetMyActiveMatchTicket(const FUniqueNetId& LocalUserId, FName SessionName, const FString& MatchPool);

	/**
	 * Check if a userId is a member in a named session
	 *
	 * @param UserId ID of the player that you are attempting to search in a session
	 * @param SessionName Session name we are searching the user in
	 * @return true if UserId is part of session member, false otherwise.
	 */
	bool SessionContainsMember(const FUniqueNetId& UserId, FName SessionName);

	bool CancelSessionInvite(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Invitee);
	
	bool CancelSessionInvite(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Invitee);

	/**
	 * Set enable match ticket status check polling.
	 * @param Enabled true will enable match ticket check polling
	 */
	void SetMatchTicketCheckEnabled(const bool Enabled);

	/**
	 * Get enabled state of match ticket status check polling.
	 * @return true if polling enabled.
	 */
	bool GetMatchTicketCheckEnabled() const;

	/**
	 * Set match ticket status check delay after matchmaking first started.
	 * @param Sec Delay time in second
	 */
	void SetMatchTicketCheckInitialDelay(const int32 Sec);

	/**
	 * Get match ticket status check delay after matchmaking first started.
	 * @return initial delay in sec
	 */
	int32 GetMatchTicketCheckInitialDelay() const;
	

	/**
	 * Set match ticket status check delay polling if the ticket state still waiting for match after first check.
	 * @param Sec Interval delay time in second
	 */
	void SetMatchTicketCheckPollInterval(const int32 Sec);

	/**
	 * Get match ticket status check delay polling if the ticket state still waiting for match after first check.
	 * @return Interval delay time in seconds
	 */
	int32 GetMatchTicketCheckPollInterval() const;

	/**
	 * Set enable session server check polling. 
	 * @param Enabled true will enable session server check polling
	 */
	void SetSessionServerCheckPollEnabled(bool Enabled);

	/**
	 * Get enabled state of session server check polling
	 * @return true if polling is enabled
	 */
	bool GetSessionServerCheckPollEnabled() const;

	/**
	 * Set session server status check after joining a session. 
	 * @param Sec Delay time in seconds
	 */
	void SetSessionServerCheckPollInitialDelay(int32 Sec);

	/**
	 * Get session server status check after joining a session.
	 * @return Initial delay in seconds
	 */
	int32 GetSessionServerCheckPollInitialDelay() const;

	/**
	 * Set session server status check polling interval if the previous attempt still waiting for Server.
	 * @param Sec interval delay in second
	 */
	void SetSessionServerCheckPollInterval(int32 Sec);

	/**
	 * Get session server status check polling interval if the previous attempt still waiting for server.
	 * @return Interval delay time in seconds
	 */
	int32 GetSessionServerCheckPollInterval() const;

	/**
	 * Set enabled state of session invite check polling after match is found.
	 * @param Enabled true will enable session invite check polling
	 */
	void SetSessionInviteCheckPollEnabled(bool Enabled);

	/**
	 * Get enabled state of session invite check polling after match is found. 
	 * @return true if polling is enabled
	 */
	bool GetSessionInviteCheckPollEnabled() const;

	/**
	 * Set session invite check initial delay after match found notification is received.
	 * @param Sec Initial delay in seconds
	 */
	void SetSessionInviteCheckPollInitialDelay(int32 Sec);

	/**
	 * Get session invite check initial delay after match found notification is received. 
	 * @return Initial delay in seconds
	 */
	int32 GetSessionInviteCheckPollInitialDelay() const;

	/**
	 * Set session invite check interval if previous call failed.
	 * @param Sec Initial delay in seconds
	 */
	void SetSessionInviteCheckPollInterval(int32 Sec);

	/**
	 * Get session invite check interval if previous call failed.
	 * @return Interval delay in seconds
	 */
	int32 GetSessionInviteCheckPollInterval() const;

	/**
	 * @brief Find a game or party session by its ID.
	 *
	 * @param SearchingUserId ID of the user to make the find session request as
	 * @param SessionType Type of session to search for
	 * @param SessionId String representation of the session ID being searched for
	 * @param CompletionDelegate Delegate fired after the operation to find the given session completes
	 * @return true if operation to find session has started, false otherwise
	 */
	bool FindSessionByStringId(const FUniqueNetId& SearchingUserId
		, const EAccelByteV2SessionType& SessionType
		, const FString& SessionId
		, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate);

	/**
	 * Begins cloud based matchmaking for a session
	 *
	 * @param LocalPlayers the ids of all local players that will participate in the match
	 * @param SessionName the name of the session to use, usually will be GAME_SESSION_NAME
	 * @param NewSessionSettings the desired settings to match against or create with when forming new sessions
	 * @param SearchSettings the desired settings that the matched session will have.
	 * @param CompletionDelegate Delegate triggered when call complete
	 *
	 * @return true if successful searching for sessions, false otherwise
	 */
	bool StartMatchmaking(const TArray<FSessionMatchmakingUser>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearchAccelByte>& SearchSettings, const FOnStartMatchmakingComplete& CompletionDelegate);
	
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
	 * Delegate fired when we get a game session or party invite from another player is time out. Includes an AccelByte invite structure to allow for discarding the invitation.
	 *
	 * @param UserId ID of the local user that received the timeout invitation notif
	 * @param Invite Invite structure that can be used to either accept or reject the invite
	 * @param SessionType Type of invitation that has been time out [GameSession or PartySession]
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnV2SessionInviteTimeoutReceived, const FUniqueNetId& /*UserId*/, const FOnlineSessionInviteAccelByte& /*Invite*/, EAccelByteV2SessionType /*SessionType*/);

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
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnMatchmakingExpired, TSharedPtr<FOnlineSessionSearchAccelByte> /*SearchHandler*/);

	/**
	 * Delegate fired when matchmaking ticket has been canceled.
	 */
	DEFINE_ONLINE_DELEGATE(OnMatchmakingCanceled);

	/**
	 * Delegate fired when matchmaking ticket has been canceled with reason as parameter.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnMatchmakingCanceledReason, const FOnlineErrorAccelByte& /*Error*/);
	
	/**
	 * Delegate fired when the game server receives a backfill proposal from matchmaking. Use AcceptBackfillProposal and
	 * RejectBackfillProposal to act on the proposal.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnBackfillProposalReceived, FAccelByteModelsV2MatchmakingBackfillProposalNotif /*Proposal*/);

	/**
	 * Delegate fired when the game server receives a backfill ticket expired from matchmaking.
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnBackfillTicketExpiredReceived, FName /*SessionName*/, FAccelByteModelsV2MatchmakingBackfillTicketExpireNotif /*BackfillExpiredTicket*/);

	/**
	 * Delegate fired when a session's invited members list changes.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionInvitesChanged, FName /*SessionName*/);

	/**
	 * Delegate fired when a local player has been kicked from a session. Fired before session is destroyed in case there
	 * is a need to do any extra clean up.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnKickedFromSession, FName /*SessionName*/);

	/**
	 * Delegate fired when a local player has received an update from the backend to a session
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionUpdateReceived, FName /*SessionName*/);

	/**
	 * Delegate fired when a session update request is completed. Together with the above OnSessionUpdateReceived delegate,
	 * the developer can separately listen to both update notifications and update request completion, without breaking changes
	 * to the regular OnUpdateSessionComplete delegate, which is used to handle both cases
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnSessionUpdateRequestComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);

	/**
	 * Delegate fired when a session update fails due to a version mismatch
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnSessionUpdateConflictError, FName /*SessionName*/, FOnlineSessionSettings /*FailedSessionSettings*/);

	DEFINE_ONLINE_DELEGATE(OnAMSDrainReceived);

	/**
	 * Delegate fired when a local player has sent an invite to a player
	 */
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnSendSessionInviteComplete, const FUniqueNetId& /*LocalSenderId*/, FName /*SessionName*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*InviteeId*/);

	/**
	 * Delegate fired when a local player rejects invite
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnSessionInviteRejected, FName /*SessionName*/, const FUniqueNetId& /*RejectedId*/);

	/**
	 * Delegate fired when a player's attributes have been initialized. Should be fired once toward the end of login.
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnPlayerAttributesInitialized, const FUniqueNetId& /*LocalUserId*/, bool /*bWasSuccessful*/);

	/*
	 * Delegate fired when server query game sessions complete
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnServerQueryGameSessionsComplete, const FAccelByteModelsV2PaginatedGameSessionQueryResult& /*GameSessionsQueryResult*/, const FOnlineError& /*ErrorInfo*/)

	/**
	 * Delegate fired when server query party sessions complete
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnServerQueryPartySessionsComplete, const FAccelByteModelsV2PaginatedPartyQueryResult& /*PartySessionsQueryResult*/, const FOnlineError& /*ErrorInfo*/)

	/**
	 * Delegate fired when get my active match ticket complete,
	 * SearchHandle will be invalid if no active ticket is found.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnGetMyActiveMatchTicketComplete, bool /*bWasSuccessful*/, FName /*SessionName*/, TSharedPtr<FOnlineSessionSearch> /*SearchHandle*/)

	/**
	 * Delegate fired when get match ticket details complete.
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnGetMatchTicketDetailsComplete, const FAccelByteModelsV2MatchmakingGetTicketDetailsResponse& /*TicketDetailsResult*/, const FOnlineError& /*ErrorInfo*/)

	/**
	 * Delegate broadcast when a session that the player is in locally has been removed on the backend. Gives the game an
	 * opportunity to clean up state.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionRemoved, FName /*SessionName*/);

	/**
	 * Delegate broadcast when another game session member promoted to be a game session leader.
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnPromoteGameSessionLeaderComplete, const FUniqueNetId& /*PromotedUserId*/, const FOnlineErrorAccelByte& /*ErrorInfo*/)

	/**
	 * Delegate fired when session leader storage has received an update
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionLeaderStorageUpdateReceived, FName /*SessionName*/);

	/**
	 * Delegate fired when a session member's storage has received an update
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnSessionMemberStorageUpdateReceived, FName /*SessionName*/, const FUniqueNetId& /*UpdatedUserId*/);

	/**
	 * Delegate fired when session leader storage has received an update
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnUpdateSessionLeaderStorageComplete, FName /*SessionName*/, const FOnlineError& /*ErrorInfo*/);

	/**
	 * Delegate fired when a session member's storage has received an update
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnUpdateSessionMemberStorageComplete, FName /*SessionName*/, const FUniqueNetId& /*UpdatedUserId*/, const FOnlineError& /*ErrorInfo*/);

	/**
	 * Delegate fired when send server ready completed.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSendDSSessionReadyComplete, const FOnlineError& /*ErrorInfo*/);

	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnV2SessionEnded, FName /*SessionName*/);

	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnCancelSessionInviteComplete, const FUniqueNetId& /*LocalUserId*/, FName /*SessionName*/, const FUniqueNetId& /*Invitee*/, const FOnlineError& /*ErrorInfo*/)

	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionInviteCanceled, const FString& /*SessionID*/)

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
	 * UTC Time for the next poll to check matchmaking details,
	 * Intended to be used for trigger manual poll matchmaking progress. 
	 */
	FDateTime NextMatchmakingDetailPollTime{0};

	/**
	 * enable match ticket details check polling.
	 */
	bool bMatchmakingDetailCheckEnabled{true};

	/**
	 * Delay time for polling match ticket details from success start matchmaking to first match ticket polling start
	 */
	int32 MatchTicketCheckInitialDelay{30};

	/**
	 * Delay time for polling match ticket details 
	 */
	int32 MatchTicketCheckPollInterval{15};

	bool bSessionServerCheckPollEnabled{true};
	int32 SessionServerCheckPollInitialDelay{30};
	int32 SessionServerCheckPollInterval{15};

	bool bSessionInviteCheckPollEnabled{true};
	int32 SessionInviteCheckPollInitialDelay{30};
	int32 SessionInviteCheckPollInterval{15};
	
	mutable FCriticalSection SessionServerCheckTimesLock;
	TArray<FSessionServerCheckPollItem> SessionServerCheckPollTimes;

	mutable FCriticalSection SessionInviteCheckTimesLock;
	TArray<FSessionInviteCheckPollItem> SessionInviteCheckPollTimes;

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
	* Check for updates and apply them to each stored session
	*/
	void UpdateSessionEntries();

	/**
	 * Get session info from match ticket and spoof match notification
	 */
	void OnMatchTicketCheckGetSessionInfoById(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SessionSearchResult);

	/**
	 * Check matchmaking progress in case matchmaking found notifications is not received in a timely manner.
	 */
	void CheckMatchmakingProgress();

	/**
	 * Check session's dedicated server readiness when notification is not received in a timely manner.
	 */
	void CheckSessionServerProgress();

	/**
	 * Check session's invite when notification is not received in a timely manner after match found is notified.
	 */
	void CheckSessionInviteAfterMatchFound();
	
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
	 * Fill out SessionSettings field of a game session with constants from the backend data
	 */
	void AddBuiltInGameSessionSettingsToSessionSettings(FOnlineSessionSettings& OutSettings, const FAccelByteModelsV2GameSession& GameSession);

	/**
	 * Fill out SessionSettings field of a party session with constants from the backend data
	 */
	void AddBuiltInPartySessionSettingsToSessionSettings(FOnlineSessionSettings& OutSettings, const FAccelByteModelsV2PartySession& PartySession);

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
	bool GetServerPort(int32& OutPort, bool bIsLocal=false) const;

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
	 * Read a base session model into a session settings instance
	 */
	bool ReadSessionSettingsFromSessionModel(FOnlineSessionSettings& OutSettings, const FAccelByteModelsV2BaseSession& Session) const;

	/**
	 * Read a JSON object into a session member settings instance
	 */
	bool ReadMemberSettingsFromJsonObject(FSessionSettings& OutMemberSettings, const TSharedRef<FJsonObject>& Object) const;

	/**
	 * Convert a session search parameters into a json object that can be used to fill match ticket attributes
	 */
	TSharedRef<FJsonObject> ConvertSearchParamsToJsonObject(const FOnlineSearchSettings& Params) const;

	/**
	 * Attempt to connect to a P2P session
	 */
	void ConnectToJoinedP2PSession(FName SessionName, EOnlineSessionP2PConnectedAction Action);

	/**
	 * Update game session data from a backend model. Used for update notifications and refreshing a game session manually.
	 */
	void UpdateInternalGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& UpdatedGameSession, bool& bIsConnectingToP2P, bool bIsFirstJoin=false);

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

	/**
	 * Send ready message to AMS
	 */
	void SendReadyToAMS();

	/**
	* @brief set DS session timeout to the new number given.
	* 
	* @param NewTimeout new number for DS session timeout.
	*/
	void SetDSTimeout(int32 NewTimeout);

	/**
	* @brief reset DS session timeout to the one that already set in the fleet settings.
	*/
	void ResetDSTimeout();

	/**
	 * Disconnect a server from the AMS, unregistering any delegates bound.
	 */
	void DisconnectFromAMS();

	void OnMatchTicketCheckGetMatchTicketDetails(const FAccelByteModelsV2MatchmakingGetTicketDetailsResponse& Response, const FOnlineError& OnlineError);
	void FinalizeStartMatchmakingComplete();

	/** 
	 * Set match ticket details poll to start polling time
	 */
	void StartMatchTicketCheckPoll();

	/**
	 * Set match ticket details poll to next polling time
	 */
	void SetMatchTicketCheckPollToNextPollTime();

	/**
	 * Stop match ticket details poll to next polling time
	 */
	void StopMatchTicketCheckPoll();

	// FOnSingleSessionResultCompleteDelegate OnFindMatchmakingGameSessionByIdCompleteDelegate;

	void SendDSStatusChangedNotif(const int32 LocalUserNum, const TSharedPtr<FAccelByteModelsV2GameSession>& SessionData);
	void StartSessionServerCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId, const FName SessionName);
	void SetSessionServerCheckPollNextPollTime(const FUniqueNetIdPtr& SearchingPlayerId, const FName SessionName);
	void StopSessionServerCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId, const FName SessionName);
	void OnSessionServerCheckGetSession(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& OnlineSessionSearchResult);

	void SendSessionInviteNotif(int32 LocalUserNum, const FString& SessionId) const;
	void StartSessionInviteCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId, const FString& SessionId);
	void SetSessionInviteCheckPollNextPollTime(const FUniqueNetIdPtr& SearchingPlayerId, const FString& SessionId);
	void StopSessionInviteCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId, const FString& SessionId);
	void OnSessionInviteCheckGetSession(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& OnlineSearchResult);
	FOnSingleSessionResultCompleteDelegate OnSessionInviteCheckGetSessionDelegate;

	/**
	* Initialize Metric Exporter.
	*
	* @param Address StatsD IPv4 address
	* @param Port StatsD port
	* @param IntervalSeconds Interval of exporting metric to StatsD
	*/
	void InitMetricExporter(const FString& Address, uint16 Port, uint16 IntervalSeconds = 60);

	/**
	 * Set Label to a specific Key of metric
	 *
	 * @param Key Key to add label
	 * @param Value label name for the key
	 */
	void SetMetricLabel(const FString& Key, const FString& Value);

	/**
	* Enqueue Metric
	*
	* @param Key The key of the metric
	* @param Value Floating number value of the metric
	*/
	void EnqueueMetric(const FString& Key, double Value);

	/**
	 * Enqueue Metric
	 *
	 * @param Key The key of the metric
	 * @param Value Integer value of the metric
	 */
	void EnqueueMetric(const FString& Key, int32 Value);

	/**
	 * Enqueue Metric
	 *
	 * @param Key The key of the metric
	 * @param Value String value of the metric
	 */
	void EnqueueMetric(const FString& Key, const FString& Value);

	/**
	* Set Sending optional metrics or not
	*/
	void SetOptionalMetricsEnabled(bool Enable);

	/**
	* Set the StatsD Metric Collector.
	* By default it will use AccelByteStatsDMetricCollector class.
	* Should be set if custom collector is needed.
	*
	* @param Collector The collector object inherited from IAccelByteStatsDMetricCollector
	*/
	void SetMetricCollector(const TSharedPtr<AccelByte::IAccelByteStatsDMetricCollector>& Collector);

	/**
	 * Makes a call to grab the most recent stored player attributes from the session service, and updates them with the
	 * current platform. Intended to be called at the end of the login process.
	 *
	 * @param LocalPlayerId ID of the player that we want to initialize attributes for
	 */
	void InitializePlayerAttributes(const FUniqueNetId& LocalPlayerId);

	/**
	 * Get the internal model for a player's attributes.
	 *
	 * @param LocalPlayerId ID of the player that we want to get the internal attributes model for
	 */
	FAccelByteModelsV2PlayerAttributes* GetInternalPlayerAttributes(const FUniqueNetId& LocalPlayerId);

	/**
	 * Stores attributes for a player in this interface. Intended to be used at login.
	 *
	 * @param LocalPlayerId ID of the player that we wish to store attributes for
	 * @param Attributes Attributes that we wish to store locally for this player, will be moved directly into map
	 */
	void StorePlayerAttributes(const FUniqueNetId& LocalPlayerId, FAccelByteModelsV2PlayerAttributes&& Attributes);

	/**
	 * @brief Query game sessions from a server, listen for OnServerQueryGameSessionsComplete delegate for the result.
	 *
	 * @param Request Request for the query for filtering the result
	 * @param Offset Offset of the query
	 * @param Limit Maximum number of game sessions listed
	 * @return true if request successfully sent
	 */
	bool ServerQueryGameSessions(const FAccelByteModelsV2ServerQueryGameSessionsRequest& Request, int64 Offset = 0, int64 Limit = 20);

	/**
	 * @brief Query party sessions from a server, listen for OnServerQueryPartySessionsComplete delegate for the result.
	 *
	 * @param Request Request for the query for filtering the result
	 * @param Offset Offset of the query
	 * @param Limit Maximum number of game sessions listed
	 * @return true if request successfully sent
	 */
	bool ServerQueryPartySessions(const FAccelByteModelsV2QueryPartiesRequest& Request, int64 Offset = 0, int64 Limit = 20);

	/**
	 * Add a canceled ticket ID to this interface instance for tracking
	 */
	void AddCanceledTicketId(const FString& TicketId);

	/**
	 * @brief Get whether Server is using AMS or not
	 *
	 * @return true if Server is using AMS
	 */
	bool IsServerUseAMS() const;

	/**
	 * @brief Used for sending dedicated server ready to session service.
	 * This method used by the DS if dsManualSetReady flag enabled in session template to indicate that
	 * this DS ready to receive connection.
	 *
	 * @return true if request sent to the session service
	 */
	bool SendDSSessionReady();

	/**
	 * @brief Clean up data related to a user when that user has logged out.
	 * Called internally by FOnlineIdentityAccelByte::OnLogout.
	 *
	 * @param LocalUserId ID of the user that has logged out that we want to clean up after
	 */
	void HandleUserLogoutCleanUp(const FUniqueNetId& LocalUserId);

private:
	/** Parent subsystem of this interface instance */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Critical section to lock sessions map while accessing */
	mutable FCriticalSection SessionLock;

	/** Sessions stored in this interface, associated by session name */
	TMap<FName, TSharedPtr<FNamedOnlineSession>> Sessions;

	/** Flag denoting whether there is already a task in progress to get a session associated with a server */
	bool bIsGettingServerClaimedSession{ false };

	/** Array of session names that have an update queued from the backend */
	TArray<FName> SessionsWithPendingQueuedUpdates{};

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

	/** Stored override for a local server name to register with. */
	FString LocalServerNameOverride{};

	/** Stored override for a local server IP to register with. */
	FString LocalServerIpOverride{};

	/** Stored override for a local server port to register with. */
	int32 LocalServerPortOverride{-1};

	/** Attributes for a player using this session interface instance, contains crossplay and platform information */
	TUniqueNetIdMap<FAccelByteModelsV2PlayerAttributes> UserIdToPlayerAttributesMap{};

	/** Array of ticket IDs that we have explicitly canceled, used to prevent race condition between cancel and start notification */
	TArray<FString> CanceledTicketIds{};

	/** Time that we last added a ticket ID to the list of canceled IDs */
	double LastCanceledTicketIdAddedTimeSeconds = 0.0;

	/** Time in seconds that we clear the array of canceled ticket IDs (default to five minutes) */
	double ClearCanceledTicketIdsTimeInSeconds = 300.0;

	/**
	* It decides whether the GameServer is automatically register itself or should be done manually. 
	* Value depends to the configuration in the DefaultEngine.ini
	* Section: [OnlineSubsystemAccelByte]
	* Key: bManualRegisterServer (boolean)
	* Effect TRUE: Developer is required to manually call the SendServerReady() after perform RegisterServer()
	* Effect FALSE: Automatically handled & there is no need to call SendServerReady() after perform the RegisterServer()
	* Default behavior: if not specified then FALSE
	*/
	bool bManualRegisterServer = false;

	/** Trigger warning to notify that the DS is not flag itself as ready after several minutes. */
	bool OnServerNotSendReadyWhenTimesUp(float DeltaTime, FOnRegisterServerComplete Delegate);

	/** Reset the bound delegate and timer. */
	void ResetWarningReminderForServerSendReady();

	/** Prevent a dangling dedicated server that forgot to flag the server ready (Call SendServerReady() function) */
	FTickerDelegate SendServerReadyWarningReminderDelegate;
	FDelegateHandleAlias SendServerReadyWarningReminderHandle;
	const int SendServerReadyWarningInMinutes = 5;

	FDelegateHandle GetMatchTicketDetailsCompleteDelegateHandle;
	FOnSingleSessionResultCompleteDelegate OnMatchTicketCheckGetMatchSessionDetailsDelegate;

	FOnSingleSessionResultCompleteDelegate OnSessionServerCheckGetSessionDelegate;

	FOnSingleSessionResultCompleteDelegate OnFindGameSessionForInviteCompleteDelegate;

	/** Hidden on purpose */
	FOnlineSessionV2AccelByte() :
		AccelByteSubsystem(nullptr)
	{
	}

	bool CreatePartySession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings);
	bool CreateGameSession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings, bool bSendCreateRequest=true);

	//~ Begin Game Session Notification Handlers
	void OnInvitedToGameSessionNotification(FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent, int32 LocalUserNum);
	void OnGameSessionInvitationTimeoutNotification(FAccelByteModelsV2GameSessionUserInviteTimeoutEvent InviteEvent, int32 LocalUserNum);
	void OnFindGameSessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent);
	void OnGameSessionMembersChangedNotification(FAccelByteModelsV2GameSessionMembersChangedEvent MembersChangedEvent, int32 LocalUserNum);
	void OnGameSessionUpdatedNotification(FAccelByteModelsV2GameSession UpdatedGameSession, int32 LocalUserNum);
	void OnKickedFromGameSessionNotification(FAccelByteModelsV2GameSessionUserKickedEvent KickedEvent, int32 LocalUserNum);
	void OnDsStatusChangedNotification(FAccelByteModelsV2DSStatusChangedNotif DsStatusChangeEvent, int32 LocalUserNum);
	void OnGameSessionInviteRejectedNotification(FAccelByteModelsV2GameSessionUserRejectedEvent RejectEvent, int32 LocalUserNum);
	void OnGameSessionInviteCanceledNotification(const FAccelByteModelsV2GameSessionInviteCanceledEvent& CanceledEvent, int32 LocalUserNum);
	//~ End Game Session Notification Handlers

	//~ Begin Party Session Notification Handlers
	void OnInvitedToPartySessionNotification(FAccelByteModelsV2PartyInvitedEvent InviteEvent, int32 LocalUserNum);
	void OnPartySessionInvitationTimeoutNotification(FAccelByteModelsV2PartyInviteTimeoutEvent InvitationTimedOutEvent, int32 LocalUserNum);
	void OnKickedFromPartySessionNotification(FAccelByteModelsV2PartyUserKickedEvent KickedEvent, int32 LocalUserNum);
	void OnFindPartySessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2PartyInvitedEvent InviteEvent);
	void OnPartySessionMembersChangedNotification(FAccelByteModelsV2PartyMembersChangedEvent MemberChangeEvent, int32 LocalUserNum);
	void OnPartySessionUpdatedNotification(FAccelByteModelsV2PartySession UpdatedPartySession, int32 LocalUserNum);
	void OnPartySessionInviteRejectedNotification(FAccelByteModelsV2PartyUserRejectedEvent RejectEvent, int32 LocalUserNum);
	void OnPartySessionInviteCanceledNotification(const FAccelByteModelsV2PartyInviteCanceledEvent& CanceledEvent, int32 LocalUserNum);
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
	void OnV2BackfillTicketExpiredNotification(const FAccelByteModelsV2MatchmakingBackfillTicketExpireNotif& Notification);
	void OnV2DsSessionMemberChangedNotification(const FAccelByteModelsV2GameSession& Notification);
	void OnV2DsSessionEndedNotification(const FAccelByteModelsSessionEndedNotification& Notification);
	void OnDSHubConnectSuccessNotification();
	void OnDSHubConnectionClosedNotification(int32 StatusCode, const FString& Reason, bool bWasClean);
	//~ End Server Notification Handlers

	//~ Begin Session Storage Notification Handler
	void OnSessionStorageChangedNotification(FAccelByteModelsV2SessionStorageChangedEvent Notification, int32 LocalUserNum);
	//~ End Session Storage Notification Handler

	//~ Begin Lobby Multicast Notification Handler
	void UnbindLobbyMulticastDelegate();
	FDelegateHandle OnGameSessionInviteCanceledHandle;
	FDelegateHandle OnPartyInviteCanceledHandle;
	//~ End Lobby Multicast Notification Handler

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
	void OnICEConnectionComplete(const FString& PeerId, const AccelByte::NetworkUtilities::EAccelByteP2PConnectionStatus& Status, FName SessionName, EOnlineSessionP2PConnectedAction Action);

	/**
	 * Internal method to get a named session as a const pointer.
	 */
	FNamedOnlineSession* GetNamedSession(FName SessionName) const;

	/**
	 * Enqueue a session data update for the next tick if its version is more up-to-date than existing data
	 */
	void EnqueueBackendDataUpdate(const FName& SessionName, const TSharedPtr<FAccelByteModelsV2BaseSession>& SessionData, const bool bIsDSReadyUpdate=false);

	void OnAMSDrain();

	/**
	 * @brief Handler for auto join game session from backend.
	 * @param GameSession game session information from the backend.
	 * @param LocalUserNum Local user num.
	 * @return true if successfully synced auto join game session from backend.
	 */
	bool HandleAutoJoinGameSession(const FAccelByteModelsV2GameSession& GameSession
		, int32 LocalUserNum
		, bool bHasDsError=false
		, FString DsErrorString=FString());

protected:
	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings) override;
	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSession& Session) override;

	// Making this async task a friend so that it can add new named sessions
	friend class FOnlineAsyncTaskAccelByteGetServerClaimedV2Session;

private:
	bool bFindMatchmakingGameSessionByIdInProgress{false};

public:
	void SetFindMatchmakingGameSessionByIdInProgress(const bool State)
	{
		bFindMatchmakingGameSessionByIdInProgress = State;
	}

private:
	void UpdateSessionInvite(const FOnlineSessionInviteAccelByte& NewInvite);
	bool RemoveSessionInvite(const FString& ID);
};

typedef TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> FOnlineSessionV2AccelBytePtr;
