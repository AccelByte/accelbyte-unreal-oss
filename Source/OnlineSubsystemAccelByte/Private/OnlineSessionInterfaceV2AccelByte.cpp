// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteCreateGameSessionV2.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteUpdateGameSessionV2.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteJoinV2GameSession.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteFindV2GameSessionById.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteFindGameSessionsV2.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteQueryAllV2SessionInvites.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteLeaveV2GameSession.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteRestoreAllV2Sessions.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteRefreshV2GameSession.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteSendV2GameSessionInvite.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteRejectV2GameSessionInvite.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteCreateV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteLeaveV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteJoinV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteSendV2PartyInvite.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteUpdatePartyV2.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteKickV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteRejectV2PartyInvite.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteRefreshV2PartySession.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteFindV2PartyById.h"
#include "AsyncTasks/Matchmaking/OnlineAsyncTaskAccelByteStartV2Matchmaking.h"
#include "AsyncTasks/Matchmaking/OnlineAsyncTaskAccelByteCancelV2Matchmaking.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteGetServerAssociatedSessionV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteRegisterRemoteServerV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteRegisterLocalServerV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteUnregisterRemoteServerV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteUnregisterLocalServerV2.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Api/AccelByteLobbyApi.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "AccelByteNetworkUtilities.h"
#include "OnlineSubsystemUtils.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineSessionV2AccelByte"
#define ACCELBYTE_P2P_TRAVEL_URL_FORMAT TEXT("accelbyte.%s:11223")

FOnlineSessionInfoAccelByteV2::FOnlineSessionInfoAccelByteV2(const FString& SessionIdStr)
	: SessionId(MakeShared<const FUniqueNetIdAccelByteResource>(SessionIdStr))
{
}

const FUniqueNetId& FOnlineSessionInfoAccelByteV2::GetSessionId() const
{
	return SessionId.Get();
}

const uint8* FOnlineSessionInfoAccelByteV2::GetBytes() const
{
	// #NOTE (Maxwell): EOS OSS has this implemented as nullptr as well - not sure why we'd ever need to get the session
	// information as a byte array? Leaving this unimplemented for now until we have a need for it.
	return nullptr;
}

int32 FOnlineSessionInfoAccelByteV2::GetSize() const
{
	// #NOTE (Maxwell): Probably also used along with the GetBytes method - thus this will be unimplemented for now
	return 0;
}

bool FOnlineSessionInfoAccelByteV2::IsValid() const
{
	return SessionId->IsValid();
}

FString FOnlineSessionInfoAccelByteV2::ToString() const
{
	// Just logging session ID and host address together - not sure if we need more than this
	return FString::Printf(TEXT("%s-%s"), *SessionId->ToString(), (HostAddress.IsValid()) ? *HostAddress->ToString(true) : TEXT("NONE"));
}

FString FOnlineSessionInfoAccelByteV2::ToDebugString() const
{
	// Same as above, just more descriptive
	return FString::Printf(TEXT("SessionId: %s; HostAddress: %s"), *SessionId->ToString(), (HostAddress.IsValid()) ? *HostAddress->ToString(true) : TEXT("NONE"));
}

void FOnlineSessionInfoAccelByteV2::SetHostAddress(const TSharedRef<FInternetAddr>& InHostAddress)
{
	HostAddress = InHostAddress;
}

TSharedPtr<FInternetAddr> FOnlineSessionInfoAccelByteV2::GetHostAddress() const
{
	return HostAddress;
}

void FOnlineSessionInfoAccelByteV2::SetBackendSessionData(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBackendSessionData)
{
	BackendSessionData = InBackendSessionData;

	// If we have valid session data, then we want to fill out some of the convenience info members in this class such as
	// the invited players array for developers.
	UpdatePlayerLists();

	// Also try and update the leader ID for this session. Method will bail if this is not a party session.
	UpdateLeaderId();

	// Finally, try and update connection information for game sessions
	UpdateConnectionInfo();
}

TSharedPtr<FAccelByteModelsV2BaseSession> FOnlineSessionInfoAccelByteV2::GetBackendSessionData() const
{
	return BackendSessionData;
}

TArray<FAccelByteModelsV2GameSessionTeam> FOnlineSessionInfoAccelByteV2::GetTeamAssignments() const
{
	return Teams;
}

void FOnlineSessionInfoAccelByteV2::SetTeamAssignments(const TArray<FAccelByteModelsV2GameSessionTeam>& InTeams)
{
	Teams = InTeams;
}

bool FOnlineSessionInfoAccelByteV2::HasConnectionInfo() const
{
	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2GameSession>(BackendSessionData);
	if (!GameSessionBackendData.IsValid())
	{
		return false;
	}

	if (GameSessionBackendData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::DS)
	{
		return HostAddress.IsValid() && HostAddress->IsValid();
	}
	else if (GameSessionBackendData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::P2P)
	{
		return !PeerId.IsEmpty();
	}

	return false;
}

FString FOnlineSessionInfoAccelByteV2::GetConnectionString() const
{
	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2GameSession>(BackendSessionData);
	if (!GameSessionBackendData.IsValid())
	{
		return TEXT("");
	}

	if (!HasConnectionInfo())
	{
		return TEXT("");
	}

	if (GameSessionBackendData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::DS)
	{
		return HostAddress->ToString(true);
	}
	else if (GameSessionBackendData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::P2P)
	{
		return FString::Printf(ACCELBYTE_P2P_TRAVEL_URL_FORMAT, *PeerId);
	}

	return TEXT("");
}

FUniqueNetIdPtr FOnlineSessionInfoAccelByteV2::GetLeaderId() const
{
	return LeaderId;
}

FString FOnlineSessionInfoAccelByteV2::GetPeerId() const
{
	return PeerId;
}

EAccelByteV2SessionConfigurationServerType FOnlineSessionInfoAccelByteV2::GetServerType() const
{
	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2GameSession>(BackendSessionData);
	if (!GameSessionBackendData.IsValid())
	{
		return EAccelByteV2SessionConfigurationServerType::NONE;
	}

	return GameSessionBackendData->Configuration.Type;
}

TArray<FUniqueNetIdRef> FOnlineSessionInfoAccelByteV2::GetJoinedMembers() const
{
	return JoinedMembers;
}

void FOnlineSessionInfoAccelByteV2::UpdatePlayerLists()
{
	if (!BackendSessionData.IsValid())
	{
		return;
	}

	// Clear both the invited player and joined player arrays, will be refilled as we iterate through the new backend member array
	InvitedPlayers.Empty();
	JoinedMembers.Empty();

	for (const FAccelByteModelsV2SessionUser& Member : BackendSessionData->Members)
	{
		// Make sure the player has either been invited to this session, or is joined/connected to it. If not, move to next member.
		const bool bIsInviteStatus = Member.Status == EAccelByteV2SessionMemberStatus::INVITED;
		const bool bIsJoinedStatus = (Member.Status == EAccelByteV2SessionMemberStatus::JOINED || Member.Status == EAccelByteV2SessionMemberStatus::CONNECTED);
		if (!bIsInviteStatus && !bIsJoinedStatus)
		{
			continue;
		}

		// If the member has a blank ID, then continue to the next member
		// Ensured as this should never happen with real data
		if (!ensure(!Member.ID.IsEmpty()))
		{
			continue;
		}

		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = Member.ID;
		CompositeId.PlatformType = Member.PlatformID;
		CompositeId.PlatformId = Member.PlatformUserID;

		// #TODO Since we clear the previous array and are allocating new IDs here, debating on whether it makes sense to make a
		// user ID cache to cut down on the amount of times that we create new ID instances and instead reuse existing IDs.
		TSharedPtr<const FUniqueNetIdAccelByteUser> MemberId = FUniqueNetIdAccelByteUser::Create(CompositeId);
		if (ensure(MemberId.IsValid()) && bIsInviteStatus)
		{
			InvitedPlayers.Emplace(MemberId.ToSharedRef());
		}
		else if (ensure(MemberId.IsValid()) && bIsJoinedStatus)
		{
			JoinedMembers.Emplace(MemberId.ToSharedRef());
		}
	}
}

void FOnlineSessionInfoAccelByteV2::UpdateLeaderId()
{
	if (!BackendSessionData.IsValid())
	{
		return;
	}

	TSharedPtr<FAccelByteModelsV2PartySession> PartyBackendSessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(BackendSessionData);
	if (!PartyBackendSessionData.IsValid())
	{
		return;
	}

	FAccelByteModelsV2SessionUser* FoundLeaderMember = PartyBackendSessionData->Members.FindByPredicate([&PartyBackendSessionData](const FAccelByteModelsV2SessionUser& Member) {
		return Member.ID.Equals(PartyBackendSessionData->LeaderID);
	});

	if (FoundLeaderMember != nullptr)
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = FoundLeaderMember->ID;
		CompositeId.PlatformType = FoundLeaderMember->PlatformID;
		CompositeId.PlatformId = FoundLeaderMember->PlatformUserID;

		LeaderId = FUniqueNetIdAccelByteUser::Create(CompositeId);
		ensure(LeaderId.IsValid());
	}
}

void FOnlineSessionInfoAccelByteV2::UpdateConnectionInfo()
{
	if (!BackendSessionData.IsValid())
	{
		return;
	}

	// If this is a dedicated session and our server is ready, grab server address info so that we can join
	TSharedPtr<FAccelByteModelsV2GameSession> GameBackendSessionData = StaticCastSharedPtr<FAccelByteModelsV2GameSession>(BackendSessionData);
	if (!GameBackendSessionData.IsValid())
	{
		return;
	}

	const bool bIsDsJoinable = GameBackendSessionData->DSInformation.Server.Status.Equals(TEXT("READY"), ESearchCase::IgnoreCase) || GameBackendSessionData->DSInformation.Server.Status.Equals(TEXT("BUSY"), ESearchCase::IgnoreCase);
	if (GameBackendSessionData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::DS && bIsDsJoinable)
	{
		// Grab socket subsystem to create a new FInternetAddr instance
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		ensure(SocketSubsystem != nullptr);

		HostAddress = SocketSubsystem->CreateInternetAddr();

		// Set the IP of the address to be the string we get from backend, error out if the string is invalid
		bool bIsIpValid = true;
		HostAddress->SetIp(*GameBackendSessionData->DSInformation.Server.Ip, bIsIpValid);
		HostAddress->SetPort(GameBackendSessionData->DSInformation.Server.Port);
		ensure(bIsIpValid);
	}
	else if (GameBackendSessionData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::P2P)
	{
		// If this is a P2P session, then we don't want to set the HostAddress, but rather set the remote ID to the user ID
		// of the player that created the session.
		//
		// #TODO This doesn't account for if a player drops and we need a new ID for them, may need to figure something out for this...
		PeerId = GameBackendSessionData->CreatedBy;
	}
}

FOnlineSessionSearchAccelByte::FOnlineSessionSearchAccelByte(const TSharedRef<FOnlineSessionSearch>& InBaseSearch)
{
	TimeoutInSeconds = InBaseSearch->TimeoutInSeconds;
	bIsLanQuery = InBaseSearch->bIsLanQuery;
	MaxSearchResults = InBaseSearch->MaxSearchResults;
	PingBucketSize = InBaseSearch->PingBucketSize;
	PlatformHash = InBaseSearch->PlatformHash;
	QuerySettings = InBaseSearch->QuerySettings;
	SearchResults = InBaseSearch->SearchResults;
	SearchState = InBaseSearch->SearchState;
}

FUniqueNetIdPtr FOnlineSessionSearchAccelByte::GetSearchingPlayerId() const
{
	return SearchingPlayerId;
}

FString FOnlineSessionSearchAccelByte::GetTicketId() const
{
	return TicketId;
}

FName FOnlineSessionSearchAccelByte::GetSearchingSessionName() const
{
	return SearchingSessionName;
}

FOnlineSessionV2AccelByte::FOnlineSessionV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
{
}

bool FOnlineSessionV2AccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineSessionV2AccelBytePtr& OutInterfaceInstance)
{
#if AB_USE_V2_SESSIONS
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	return OutInterfaceInstance.IsValid();
#else
	OutInterfaceInstance = nullptr;
	return false;
#endif
}

bool FOnlineSessionV2AccelByte::GetFromWorld(const UWorld* World, FOnlineSessionV2AccelBytePtr& OutInterfaceInstance)
{
#if AB_USE_V2_SESSIONS
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
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

void FOnlineSessionV2AccelByte::Init()
{
	if (IsRunningDedicatedServer())
	{
		const FOnGetServerAssociatedSessionCompleteDelegate OnGetServerAssociatedSessionCompleteDelegate = FOnGetServerAssociatedSessionCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnGetServerAssociatedSessionComplete_Internal);
		AddOnGetServerAssociatedSessionCompleteDelegate_Handle(OnGetServerAssociatedSessionCompleteDelegate);
	}
}

void FOnlineSessionV2AccelByte::Tick(float DeltaTime)
{
	// #NOTE (Voltaire) Method was added to satisfy session interface pairity in FOnlineSubsystemAccelByte::Tick
}

void FOnlineSessionV2AccelByte::RegisterSessionNotificationDelegates(const FUniqueNetId& PlayerId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s"), *PlayerId.ToDebugString());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	int32 LocalUserNum = 0;
	ensure(IdentityInterface->GetLocalUserNum(PlayerId, LocalUserNum));

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(PlayerId);
	ensure(ApiClient.IsValid());

	#define BIND_LOBBY_NOTIFICATION(NotificationDelegateName, Verb) \
		typedef AccelByte::Api::Lobby::FV2##NotificationDelegateName##Notif F##Verb##NotificationDelegate; \
		const F##Verb##NotificationDelegate On##Verb##NotificationDelegate = F##Verb##NotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::On##Verb##Notification, LocalUserNum); \
		ApiClient->Lobby.SetV2##NotificationDelegateName##NotifDelegate(On##Verb##NotificationDelegate); \

	// Begin Game Session Notifications
	BIND_LOBBY_NOTIFICATION(GameSessionInvited, InvitedToGameSession);
	BIND_LOBBY_NOTIFICATION(GameSessionMembersChanged, GameSessionMembersChanged);
	BIND_LOBBY_NOTIFICATION(GameSessionUpdated, GameSessionUpdated);
	BIND_LOBBY_NOTIFICATION(DSStatusChanged, DsStatusChanged);
	//~ End Game Session Notifications

	// Begin Party Session Notifications
	BIND_LOBBY_NOTIFICATION(PartyInvited, InvitedToPartySession);
	BIND_LOBBY_NOTIFICATION(PartyMembersChanged, PartySessionMembersChanged);
	BIND_LOBBY_NOTIFICATION(PartyUpdated, PartySessionUpdated);
	//~ End Party Session Notifications

	// Begin Matchmaking Notifications
	BIND_LOBBY_NOTIFICATION(MatchmakingStart, MatchmakingStarted);
	BIND_LOBBY_NOTIFICATION(MatchmakingMatchFound, MatchmakingMatchFound);
	BIND_LOBBY_NOTIFICATION(MatchmakingExpired, MatchmakingExpired);
	//~ End Matchmaking Notifications

	#undef BIND_LOBBY_NOTIFICATION

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

EAccelByteV2SessionJoinability FOnlineSessionV2AccelByte::GetJoinabilityFromString(const FString& JoinabilityStr) const
{
	UEnum* JoinabilityEnum = StaticEnum<EAccelByteV2SessionJoinability>();
	ensure(JoinabilityEnum != nullptr);

	int64 FoundJoinTypeEnumValue = JoinabilityEnum->GetValueByNameString(JoinabilityStr);
	if (FoundJoinTypeEnumValue != INDEX_NONE)
	{
		return static_cast<EAccelByteV2SessionJoinability>(FoundJoinTypeEnumValue);
	}

	// By default, return joinabilty as empty in case the creator didn't specify a join type
	return EAccelByteV2SessionJoinability::EMPTY;
}

EOnlineSessionTypeAccelByte FOnlineSessionV2AccelByte::GetSessionTypeFromString(const FString& SessionTypeStr) const
{
	UEnum* SessionTypeEnum = StaticEnum<EOnlineSessionTypeAccelByte>();
	ensure(SessionTypeEnum != nullptr);

	int64 FoundSessionTypeEnumValue = SessionTypeEnum->GetValueByNameString(SessionTypeStr);
	if (FoundSessionTypeEnumValue != INDEX_NONE)
	{
		return static_cast<EOnlineSessionTypeAccelByte>(FoundSessionTypeEnumValue);
	}

	// By default, return session type as unknown if not found
	return EOnlineSessionTypeAccelByte::Unknown;
}

EAccelByteV2SessionJoinability FOnlineSessionV2AccelByte::GetJoinabiltyFromSessionSettings(const FOnlineSessionSettings& Settings) const
{
	FString JoinTypeString;
	Settings.Get(SETTING_SESSION_JOIN_TYPE, JoinTypeString);
	return GetJoinabilityFromString(JoinTypeString);
}

FString FOnlineSessionV2AccelByte::GetJoinabilityAsString(const EAccelByteV2SessionJoinability& Joinability)
{
	UEnum* JoinabilityEnum = StaticEnum<EAccelByteV2SessionJoinability>();
	ensure(JoinabilityEnum != nullptr);

	return JoinabilityEnum->GetAuthoredNameStringByValue(static_cast<int64>(Joinability)).ToUpper();
}

TSharedPtr<const FUniqueNetId> FOnlineSessionV2AccelByte::CreateSessionIdFromString(const FString& SessionIdStr)
{
	return MakeShared<const FUniqueNetIdAccelByteResource>(SessionIdStr);
}

FNamedOnlineSession* FOnlineSessionV2AccelByte::AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings)
{
	FScopeLock ScopeLock(&SessionLock);

	TSharedPtr<FNamedOnlineSession> NewNamedSession = MakeShared<FNamedOnlineSession>(SessionName, SessionSettings);
	Sessions.Emplace(SessionName, NewNamedSession);

	return NewNamedSession.Get();
}

FNamedOnlineSession* FOnlineSessionV2AccelByte::AddNamedSession(FName SessionName, const FOnlineSession& Session)
{
	FScopeLock ScopeLock(&SessionLock);

	TSharedPtr<FNamedOnlineSession> NewNamedSession = MakeShared<FNamedOnlineSession>(SessionName, Session);
	Sessions.Emplace(SessionName, NewNamedSession);

	return NewNamedSession.Get();
}

FNamedOnlineSession* FOnlineSessionV2AccelByte::GetNamedSession(FName SessionName)
{
	FScopeLock ScopeLock(&SessionLock);

	TSharedPtr<FNamedOnlineSession>* FoundNamedSession = Sessions.Find(SessionName);
	if (FoundNamedSession)
	{
		return (*FoundNamedSession).Get();
	}
	
	return nullptr;
}

FNamedOnlineSession* FOnlineSessionV2AccelByte::GetNamedSession(FName SessionName) const
{
	FScopeLock ScopeLock(&SessionLock);

	const TSharedPtr<FNamedOnlineSession>* FoundNamedSession = Sessions.Find(SessionName);
	if (FoundNamedSession)
	{
		return (*FoundNamedSession).Get();
	}

	return nullptr;
}

void FOnlineSessionV2AccelByte::RemoveNamedSession(FName SessionName)
{
	FScopeLock ScopeLock(&SessionLock);
	Sessions.Remove(SessionName);
}

bool FOnlineSessionV2AccelByte::HasPresenceSession()
{
	return false;
}

EOnlineSessionState::Type FOnlineSessionV2AccelByte::GetSessionState(FName SessionName) const
{
	FScopeLock ScopeLock(&SessionLock);

	const TSharedPtr<FNamedOnlineSession>* FoundNamedSession = Sessions.Find(SessionName);
	if (FoundNamedSession)
	{
		return (*FoundNamedSession)->SessionState;
	}

	return EOnlineSessionState::NoSession;
}

bool FOnlineSessionV2AccelByte::CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerNum: %d; SessionName: %s"), HostingPlayerNum, *SessionName.ToString());

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(HostingPlayerNum);
	ensure(PlayerId.IsValid());

	AB_OSS_INTERFACE_TRACE_END(TEXT("Passing to create session with player '%s'!"), *PlayerId->ToDebugString());
	return CreateSession(PlayerId.ToSharedRef().Get(), SessionName, NewSessionSettings);
}

bool FOnlineSessionV2AccelByte::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToString(), *SessionName.ToString());

	// Check if we already have a session with this name, and if so bail with warning
	if (GetNamedSession(SessionName) != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session with name '%s' as a session with that name already exists!"), *SessionName.ToString());
		return false;
	}

	FString SessionTypeString = TEXT("");
	if (!NewSessionSettings.Get(SETTING_SESSION_TYPE, SessionTypeString))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create new session as the SETTING_SESSION_TYPE was blank! Needs to be either SETTING_SESSION_TYPE_PARTY_SESSION or SETTING_SESSION_TYPE_GAME_SESSION!"));
		return false;
	}

	EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(NewSessionSettings);
	if (SessionType == EOnlineSessionTypeAccelByte::Unknown)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create new session as the SETTING_SESSION_TYPE was blank or set to an unsupported value! Needs to be either SETTING_SESSION_TYPE_PARTY_SESSION or SETTING_SESSION_TYPE_GAME_SESSION!"));
		return false;
	}

	if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		return CreatePartySession(HostingPlayerId, SessionName, NewSessionSettings);
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		return CreateGameSession(HostingPlayerId, SessionName, NewSessionSettings);
	}

	return false;
}

bool FOnlineSessionV2AccelByte::CreatePartySession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToDebugString(), *SessionName.ToString());

	// Check whether or not our join type is set to something other than invite only, if so throw a warning and kill the task
	FString SessionJoinTypeString = TEXT("");
	NewSessionSettings.Get(SETTING_SESSION_JOIN_TYPE, SessionJoinTypeString);

	const EAccelByteV2SessionJoinability Joinability = GetJoinabilityFromString(SessionJoinTypeString);
	if (Joinability != EAccelByteV2SessionJoinability::EMPTY && Joinability != EAccelByteV2SessionJoinability::INVITE_ONLY)
	{
		UE_LOG_AB(Warning, TEXT("Failed to create party as SETTING_SESSION_JOIN_TYPE setting is set to a value other than INVITE_ONLY!"));
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		return false;
	}

	// Create new session instance for this party that we are trying to create, and reflect state that we are creating
	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, NewSessionSettings);
	NewSession->SessionState = EOnlineSessionState::Creating;

	NewSession->bHosting = true;

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	NewSession->OwningUserId = HostingPlayerId.AsShared();
	NewSession->OwningUserName = IdentityInterface->GetPlayerNickname(HostingPlayerId);
	NewSession->LocalOwnerId = HostingPlayerId.AsShared();
	
	NewSession->NumOpenPublicConnections = NewSessionSettings.NumPublicConnections;
	NewSession->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
	
	// Set the build ID for the session
	NewSession->SessionSettings.BuildUniqueId = GetBuildUniqueId();

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateV2Party>(AccelByteSubsystem, HostingPlayerId, SessionName, NewSessionSettings);
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::CreateGameSession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings, bool bSendCreateRequest)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToDebugString(), *SessionName.ToString());

	// Create new session and update state
	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, NewSessionSettings);
	NewSession->SessionState = EOnlineSessionState::Creating;

	// Set number of open connections to the requested connection count from settings
	NewSession->NumOpenPublicConnections = NewSessionSettings.NumPublicConnections;
	NewSession->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
	
	// If we are sending a create request for this session, then that means that we know who is hosting already.
	// For servers, when this flag will be false, we will fill out the owner information once we get the backend
	// session data.
	if (bSendCreateRequest)
	{
		const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
		ensure(IdentityInterface.IsValid());
		NewSession->OwningUserId = HostingPlayerId.AsShared();
		NewSession->OwningUserName = IdentityInterface->GetPlayerNickname(HostingPlayerId);
		NewSession->LocalOwnerId = HostingPlayerId.AsShared();
	}

	NewSession->bHosting = true;

	// Set the build ID for the session
	NewSession->SessionSettings.BuildUniqueId = GetBuildUniqueId();

	if (bSendCreateRequest)
	{
		// From here, kick off async task to create the session on backend, depending on the outcome, this session will either
		// be removed or filled out further
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateGameSessionV2>(AccelByteSubsystem, HostingPlayerId, SessionName, NewSessionSettings);
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::FinalizeCreateGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& BackendSessionInfo)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *BackendSessionInfo.ID);

	FNamedOnlineSession* NewSession = GetNamedSession(SessionName);
	ensure(NewSession != nullptr);

	// Create new session info based off of the created session, start by filling session ID
	TSharedRef<FOnlineSessionInfoAccelByteV2> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV2>(BackendSessionInfo.ID);
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(BackendSessionInfo));
	SessionInfo->SetTeamAssignments(BackendSessionInfo.Teams);
	NewSession->SessionInfo = SessionInfo;

	// Closed and invite only sessions populate the private connection num, open populates the public num
	if (BackendSessionInfo.Configuration.Joinability == EAccelByteV2SessionJoinability::INVITE_ONLY || BackendSessionInfo.Configuration.Joinability == EAccelByteV2SessionJoinability::CLOSED)
	{
		NewSession->SessionSettings.NumPrivateConnections = BackendSessionInfo.Configuration.MaxPlayers;
		NewSession->NumOpenPrivateConnections = NewSession->SessionSettings.NumPrivateConnections;

		NewSession->SessionSettings.NumPublicConnections = 0;
		NewSession->NumOpenPublicConnections = 0;
	}
	else if (BackendSessionInfo.Configuration.Joinability == EAccelByteV2SessionJoinability::OPEN)
	{
		NewSession->SessionSettings.NumPublicConnections = BackendSessionInfo.Configuration.MaxPlayers;
		NewSession->NumOpenPublicConnections = NewSession->SessionSettings.NumPrivateConnections;

		NewSession->SessionSettings.NumPrivateConnections = 0;
		NewSession->NumOpenPrivateConnections = 0;
	}

	// Register ourselves to the session as well
	ensure(NewSession->LocalOwnerId.IsValid());
	RegisterPlayer(SessionName, NewSession->LocalOwnerId.ToSharedRef().Get(), false);

	// If this is a P2P session, then we want to run the code to register the P2P socket subsystem
	if (SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P)
	{
		SetupAccelByteP2PConnection(NewSession->LocalOwnerId.ToSharedRef().Get());
	}

	NewSession->SessionState = EOnlineSessionState::Pending;

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::FinalizeCreatePartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& BackendSessionInfo)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *BackendSessionInfo.ID);

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	ensure(Session != nullptr);

	// Create new session info based off of the created session, set by filling session ID
	TSharedRef<FOnlineSessionInfoAccelByteV2> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV2>(BackendSessionInfo.ID);
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(BackendSessionInfo));
	Session->SessionInfo = SessionInfo;

	// Parties are always invite only, so we just want to update the private connection num
	Session->SessionSettings.NumPrivateConnections = BackendSessionInfo.Configuration.MaxPlayers;
	Session->NumOpenPrivateConnections = Session->SessionSettings.NumPrivateConnections;

	// Make sure to also register ourselves to the session
	ensure(Session->LocalOwnerId.IsValid());
	RegisterPlayer(SessionName, Session->LocalOwnerId.ToSharedRef().Get(), false);

	Session->SessionState = EOnlineSessionState::Pending;

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineSessionV2AccelByte::ConstructGameSessionFromBackendSessionModel(const FAccelByteModelsV2GameSession& BackendSession, FOnlineSession& OutResult)
{
	// First, read attributes from backend model into session settings
	if (BackendSession.Attributes.JsonObject.IsValid())
	{
		OutResult.SessionSettings = ReadSessionSettingsFromJsonObject(BackendSession.Attributes.JsonObject.ToSharedRef());
	}

	// Add some session attributes to the settings object so that developers can update them if needed
	OutResult.SessionSettings.Set(SETTING_SESSION_TYPE, SETTING_SESSION_TYPE_GAME_SESSION);
	OutResult.SessionSettings.Set(SETTING_SESSION_JOIN_TYPE, GetJoinabilityAsString(BackendSession.Configuration.Joinability));
	OutResult.SessionSettings.Set(SETTING_SESSION_MATCHPOOL, BackendSession.MatchPool);
	OutResult.SessionSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, BackendSession.Configuration.MinPlayers);
	OutResult.SessionSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, BackendSession.Configuration.InactiveTimeout);
	OutResult.SessionSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, BackendSession.Configuration.InactiveTimeout);
	OutResult.SessionSettings.Set(SETTING_GAMESESSION_CLIENTVERSION, BackendSession.Configuration.ClientVersion);
	OutResult.SessionSettings.Set(SETTING_GAMESESSION_DEPLOYMENT, BackendSession.Configuration.Deployment);
	OutResult.SessionSettings.Set(SETTING_GAMESESSION_REQUESTEDREGIONS, ConvertToRegionListString(BackendSession.Configuration.RequestedRegions));

	if (!BackendSession.CreatedBy.IsEmpty())
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = BackendSession.CreatedBy;

		TSharedPtr<const FUniqueNetIdAccelByteUser> OwnerId = FUniqueNetIdAccelByteUser::Create(CompositeId);
		ensure(OwnerId.IsValid());

		OutResult.OwningUserId = OwnerId;
	}

	TSharedRef<FOnlineSessionInfoAccelByteV2> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV2>(BackendSession.ID);
	SessionInfo->SetTeamAssignments(BackendSession.Teams);
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(BackendSession));

	// Closed and invite only sessions populate the private connection num, open populates the public num
	if (BackendSession.Configuration.Joinability == EAccelByteV2SessionJoinability::INVITE_ONLY || BackendSession.Configuration.Joinability == EAccelByteV2SessionJoinability::CLOSED)
	{
		OutResult.SessionSettings.NumPrivateConnections = BackendSession.Configuration.MaxPlayers;
		OutResult.NumOpenPrivateConnections = OutResult.SessionSettings.NumPrivateConnections - SessionInfo->GetJoinedMembers().Num();
		if (OutResult.NumOpenPrivateConnections < 0)
		{
			OutResult.NumOpenPrivateConnections = 0;
		}

		OutResult.SessionSettings.NumPublicConnections = 0;
		OutResult.NumOpenPublicConnections = 0;
	}
	else if (BackendSession.Configuration.Joinability == EAccelByteV2SessionJoinability::OPEN)
	{
		OutResult.SessionSettings.NumPublicConnections = BackendSession.Configuration.MaxPlayers;
		OutResult.NumOpenPublicConnections = OutResult.SessionSettings.NumPublicConnections - SessionInfo->GetJoinedMembers().Num();
		if (OutResult.NumOpenPublicConnections < 0)
		{
			OutResult.NumOpenPublicConnections = 0;
		}

		OutResult.SessionSettings.NumPrivateConnections = 0;
		OutResult.NumOpenPrivateConnections = 0;
	}

	OutResult.SessionInfo = SessionInfo;
	return true;
}

bool FOnlineSessionV2AccelByte::ConstructPartySessionFromBackendSessionModel(const FAccelByteModelsV2PartySession& BackendSession, FOnlineSession& OutResult)
{
	// First, read attributes from backend model into session settings
	if (BackendSession.Attributes.JsonObject.IsValid())
	{
		OutResult.SessionSettings = ReadSessionSettingsFromJsonObject(BackendSession.Attributes.JsonObject.ToSharedRef());
	}

	// Add some session attributes to the settings object so that developers can update them if needed
	OutResult.SessionSettings.Set(SETTING_SESSION_TYPE, SETTING_SESSION_TYPE_PARTY_SESSION);
	OutResult.SessionSettings.Set(SETTING_SESSION_JOIN_TYPE, GetJoinabilityAsString(BackendSession.Configuration.Joinability));
	OutResult.SessionSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, BackendSession.Configuration.MinPlayers);
	OutResult.SessionSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, BackendSession.Configuration.InactiveTimeout);
	OutResult.SessionSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, BackendSession.Configuration.InactiveTimeout);

	if (!BackendSession.CreatedBy.IsEmpty())
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = BackendSession.CreatedBy;

		TSharedPtr<const FUniqueNetIdAccelByteUser> OwnerId = FUniqueNetIdAccelByteUser::Create(CompositeId);
		ensure(OwnerId.IsValid());

		OutResult.OwningUserId = OwnerId;
	}

	TSharedRef<FOnlineSessionInfoAccelByteV2> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV2>(BackendSession.ID);
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(BackendSession));
	
	// Party sessions are always invite only, thus just update the private connection num
	OutResult.SessionSettings.NumPrivateConnections = BackendSession.Configuration.MaxPlayers;
	OutResult.NumOpenPrivateConnections = OutResult.SessionSettings.NumPrivateConnections - SessionInfo->GetJoinedMembers().Num();
	if (OutResult.NumOpenPrivateConnections < 0)
	{
		OutResult.NumOpenPrivateConnections = 0;
	}

	OutResult.SessionInfo = SessionInfo;
	return true;
}

bool FOnlineSessionV2AccelByte::ShouldSkipAddingFieldToSessionAttributes(const FName& FieldName) const
{
	return FieldName == SETTING_SESSION_TYPE ||
		FieldName == SETTING_SESSION_JOIN_TYPE ||
		FieldName == SETTING_SESSION_MATCHPOOL ||
		FieldName == SETTING_SESSION_MINIMUM_PLAYERS ||
		FieldName == SETTING_SESSION_INACTIVE_TIMEOUT ||
		FieldName == SETTING_SESSION_INVITE_TIMEOUT ||
		FieldName == SETTING_GAMESESSION_REQUESTEDREGIONS ||
		FieldName == SETTING_GAMESESSION_DEPLOYMENT ||
		FieldName == SETTING_GAMESESSION_CLIENTVERSION ||
		FieldName == SETTING_GAMESESSION_SERVERNAME;
}

bool FOnlineSessionV2AccelByte::GetServerLocalIp(FString& OutIp) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (SocketSubsystem == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get local IP for server as our socket subsystem is invalid!"));
		return false;
	}

	bool bCanBindAll;
	TSharedPtr<FInternetAddr> LocalIP = SocketSubsystem->GetLocalHostAddr(*GLog, bCanBindAll);
	if (!LocalIP.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get local IP for server as our local host address from our socket subsystem is invalid!"));
		return false;
	}

	OutIp = LocalIP->ToString(false);

	AB_OSS_INTERFACE_TRACE_END(TEXT("OutIp: %s"), *OutIp);
	return true;
}

bool FOnlineSessionV2AccelByte::GetServerPort(int32& OutPort) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get port for server as the current game instance is not a dedicated server!"));
		return false;
	}

	if (GEngine == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get port for server as our engine singleton instance was invalid!"));
		return false;
	}

	// As far as I am aware, the only way to get the currently bound port for a server (at least within the OSS) is to
	// get the current world from the engine singleton, grab the URL from the world, and then extract the port. Not
	// sure if there is a better way to do this, definitely something to look into so we don't have to rely on engine
	// singleton or the world...
#if (ENGINE_MAJOR_VERSION >= 5) || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26)
	UWorld* World = GEngine->GetCurrentPlayWorld();
	if (World == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get port for server as our world instance was invalid!"));
		return false;
	}

	OutPort = World->URL.Port;
#else
	OutPort = 7777; // #TODO(Maxwell): Figure out a better way to get bound port if not 4.26...
#endif

	AB_OSS_INTERFACE_TRACE_END(TEXT("OutPort: %d"), OutPort);
	return true;
}

bool FOnlineSessionV2AccelByte::GetLocalServerName(FString& OutServerName) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// First, start by trying to find server name as an environment variable
	OutServerName = FPlatformMisc::GetEnvironmentVariable(TEXT("SERVER_NAME"));
	if (OutServerName.IsEmpty())
	{
		// If we didn't find a server name in the environment, try and find it on the command line
		if (!FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), OutServerName))
		{
			// If all else fails, try and get the machine name to use as server name
			OutServerName = FPlatformProcess::ComputerName();
		}
	}

	// If we didn't get a server name through any of those methods, just bail
	if (OutServerName.IsEmpty())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find suitable server name from environment, command line, or computer name!"));
		return false;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("OutServerName: %s"), *OutServerName);
	return true;
}

bool FOnlineSessionV2AccelByte::GetServerAssociatedSession(FName SessionName)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		return false;
	}

	// First, check if we already have a game session created for us
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to create local session for associated dedicated server session as we already locally have a game session instance!"));
		return false;
	}

	FOnlineSessionSettings NewSessionSettings;
	NewSessionSettings.Set(SETTING_SESSION_TYPE, SETTING_SESSION_TYPE_GAME_SESSION);
	NewSessionSettings.bIsDedicated = true;

	// Flag that creation is underway
	bIsGettingServerAssociatedSession = true;

	// Create a new game session instance that will be further filled out by the subsequent async task.
	// #NOTE Intentionally passing an invalid ID as we don't know who is hosting the session currently.
	CreateGameSession(FUniqueNetIdAccelByteUser::Invalid().Get(), SessionName, NewSessionSettings, false);

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2>(AccelByteSubsystem, SessionName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::RemoveRestoreSessionById(const FString& SessionIdStr)
{
	int32 FoundRestoreSession = RestoredSessions.IndexOfByPredicate([SessionIdStr](const FOnlineRestoredSessionAccelByte& RestoredSession) {
		return RestoredSession.Session.GetSessionIdStr().Equals(SessionIdStr);
	});

	if (FoundRestoreSession != INDEX_NONE)
	{
		RestoredSessions.RemoveAt(FoundRestoreSession);
		return true;
	}

	return false;
}

bool FOnlineSessionV2AccelByte::RemoveInviteById(const FString& SessionIdStr)
{
	// Try and remove the party invite from our list of invites
	// Also, we will want to make sure to remove any duplicate invites
	int32 RemoveResult = SessionInvites.RemoveAll([SessionIdStr](const FOnlineSessionInviteAccelByte& Invite) {
		return Invite.Session.GetSessionIdStr() == SessionIdStr;
	});

	if (RemoveResult < 1)
	{
		return false;
	}

	TriggerOnInviteListUpdatedDelegates();
	return true;
}

bool FOnlineSessionV2AccelByte::LeaveSession(const FUniqueNetId& LocalUserId, const EOnlineSessionTypeAccelByte& SessionType, const FString& SessionId, const FOnLeaveSessionComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionId: %s"), *LocalUserId.ToDebugString(), *SessionId);

	if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLeaveV2GameSession>(AccelByteSubsystem, LocalUserId, SessionId, Delegate);
	
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending request to leave game session!"));
		return true;
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLeaveV2Party>(AccelByteSubsystem, LocalUserId, SessionId, Delegate);
		
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending request to leave party session!"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

TSharedRef<FJsonObject> FOnlineSessionV2AccelByte::ConvertSessionSettingsToJsonObject(const FOnlineSessionSettings& Settings) const
{
	TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
	for (const TPair<FName, FOnlineSessionSetting>& Setting : Settings.Settings)
	{
		if (ShouldSkipAddingFieldToSessionAttributes(Setting.Key))
		{
			continue;
		}

		// Add the setting to the attributes object. With the setting key as the field name, converted to all uppercase to avoid
		// FName weirdness with casing. Adding with type suffix so that we can easily read attributes from backend JSON.
		Setting.Value.Data.AddToJsonObject(OutObject, Setting.Key.ToString().ToUpper(), true);
	}

	return OutObject;
}

FOnlineSessionSettings FOnlineSessionV2AccelByte::ReadSessionSettingsFromJsonObject(const TSharedRef<FJsonObject>& Object) const
{
	FOnlineSessionSettings OutSettings{};
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Attribute : Object->Values)
	{
		if (!Attribute.Value.IsValid())
		{
			continue;
		}

		// Read attribute into a variant data type
		FVariantData VariantValue;
		FString AttributeNameNoSuffix;
		if (!VariantValue.FromJsonValue(Attribute.Key, Attribute.Value.ToSharedRef(), AttributeNameNoSuffix))
		{
			UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as variant data, skipping!"), *Attribute.Key);
			continue;
		}

		// Add attribute to session settings key/value pair
		OutSettings.Set(FName(AttributeNameNoSuffix), VariantValue);
	}

	return OutSettings;
}

TSharedRef<FJsonObject> FOnlineSessionV2AccelByte::ConvertSearchParamsToJsonObject(const FSearchParams& Params) const
{
	TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
	for (const TPair<FName, FOnlineSessionSearchParam>& Param : Params)
	{
		if (Param.Key == SETTING_GAMESESSION_CLIENTVERSION)
		{
			// Client version is a special key in the match ticket attributes. So we need to send it as a snake case field with
			// no type suffix.
			Param.Value.Data.AddToJsonObject(OutObject, TEXT("client_version"), false);
			continue;
		}

		if (Param.Key == SETTING_GAMESESSION_SERVERNAME)
		{
			// Server name is a special key in the match ticket attributes. So we need to send it as a snake case field with
			// no type suffix.
			Param.Value.Data.AddToJsonObject(OutObject, TEXT("server_name"), false);
			continue;
		}

		if (ShouldSkipAddingFieldToSessionAttributes(Param.Key))
		{
			continue;
		}
		
		// Add the setting to the attributes object. With the setting key as the field name, converted to all uppercase to avoid
		// FName weirdness with casing.
		// Removing type suffix because key need to be same as what is configured in ruleset.
		Param.Value.Data.AddToJsonObject(OutObject, Param.Key.GetPlainNameString(), false);
	}

	return OutObject;
}

void FOnlineSessionV2AccelByte::ConnectToJoinedP2PSession(FName SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!ensure(Session != nullptr))
	{
		return;
	}

	// Attempt to register our socket subsystem as default
	FAccelByteNetworkUtilitiesModule::Get().RegisterDefaultSocketSubsystem();

	// Now, grab the API client for the user that is setting up P2P and run set up in the module
	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	ensure(Session->LocalOwnerId.IsValid());
	const AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(Session->LocalOwnerId.ToSharedRef().Get());
	ensure(ApiClient.IsValid());

	FAccelByteNetworkUtilitiesModule::Get().Setup(ApiClient);

	// Register a delegate so that we can fire our join session complete delegate when connection finishes
	const FAccelByteNetworkUtilitiesModule::OnICEConnected OnICEConnectionCompleteDelegate = FAccelByteNetworkUtilitiesModule::OnICEConnected::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnICEConnectionComplete, SessionName);
	FAccelByteNetworkUtilitiesModule::Get().RegisterICEConnectedDelegate(OnICEConnectionCompleteDelegate);

	// Request connection to the peer
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	ensure(SessionInfo.IsValid());

	FAccelByteNetworkUtilitiesModule::Get().RequestConnect(SessionInfo->GetPeerId());
}

void FOnlineSessionV2AccelByte::UpdateInternalGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& UpdatedGameSession)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' does not exist locally!"), *SessionName.ToString());
		return;
	}

	const EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EOnlineSessionTypeAccelByte::GameSession)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally is not a game session!"), *SessionName.ToString());
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally does not have valid session info!"), *SessionName.ToString());
		return;
	}

	// First update the session data associated with the session info structure
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(UpdatedGameSession));

	// First, read all attributes from the session, as that will overwrite all of the reserved/built-in settings
	if (UpdatedGameSession.Attributes.JsonObject.IsValid())
	{
		Session->SessionSettings = ReadSessionSettingsFromJsonObject(UpdatedGameSession.Attributes.JsonObject.ToSharedRef());
	}

	// After loading in custom attributes, load in reserved/built-in settings
	Session->SessionSettings.Set(SETTING_SESSION_TYPE, SETTING_SESSION_TYPE_GAME_SESSION);
	Session->SessionSettings.Set(SETTING_SESSION_MATCHPOOL, UpdatedGameSession.MatchPool);
	Session->SessionSettings.Set(SETTING_SESSION_MATCHPOOL, UpdatedGameSession.MatchPool);
	Session->SessionSettings.Set(SETTING_SESSION_JOIN_TYPE, GetJoinabilityAsString(UpdatedGameSession.Configuration.Joinability));
	Session->SessionSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, UpdatedGameSession.Configuration.MinPlayers);
	Session->SessionSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, UpdatedGameSession.Configuration.InviteTimeout);
	Session->SessionSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, UpdatedGameSession.Configuration.InactiveTimeout);
	Session->SessionSettings.Set(SETTING_GAMESESSION_CLIENTVERSION, UpdatedGameSession.Configuration.ClientVersion);
	Session->SessionSettings.Set(SETTING_GAMESESSION_DEPLOYMENT, UpdatedGameSession.Configuration.Deployment);
	Session->SessionSettings.Set(SETTING_GAMESESSION_REQUESTEDREGIONS, ConvertToRegionListString(UpdatedGameSession.Configuration.RequestedRegions));
	SetSessionMaxPlayerCount(Session, UpdatedGameSession.Configuration.MaxPlayers);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::UpdateInternalPartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& UpdatedPartySession)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for party session as the session named '%s' does not exist locally!"), *SessionName.ToString());
		return;
	}

	const EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EOnlineSessionTypeAccelByte::PartySession)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for party session as the session named '%s' locally is not a party session!"), *SessionName.ToString());
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for party session as the session named '%s' locally does not have valid session info!"), *SessionName.ToString());
		return;
	}

	// First update the session data associated with the session info structure
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(UpdatedPartySession));

	// First, read all attributes from the session, as that will overwrite all of the reserved/built-in settings
	if (UpdatedPartySession.Attributes.JsonObject.IsValid())
	{
		Session->SessionSettings = ReadSessionSettingsFromJsonObject(UpdatedPartySession.Attributes.JsonObject.ToSharedRef());
	}

	// After reading attributes, reload reserved/built-in settings for the session
	Session->SessionSettings.Set(SETTING_SESSION_TYPE, SETTING_SESSION_TYPE_PARTY_SESSION);
	Session->SessionSettings.Set(SETTING_SESSION_JOIN_TYPE, GetJoinabilityAsString(UpdatedPartySession.Configuration.Joinability));
	Session->SessionSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, UpdatedPartySession.Configuration.MinPlayers);
	Session->SessionSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, UpdatedPartySession.Configuration.InviteTimeout);
	Session->SessionSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, UpdatedPartySession.Configuration.InactiveTimeout);
	SetSessionMaxPlayerCount(Session, UpdatedPartySession.Configuration.MaxPlayers);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineSessionV2AccelByte::StartSession(FName SessionName)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to start session as the session does not exist locally!"));
		
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// If the session is not waiting to start or already ended (meaning we can restart) then error out
	if (Session->SessionState != EOnlineSessionState::Pending && Session->SessionState != EOnlineSessionState::Ended)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to start session in state '%s'! Session needs to be in 'Pending' or 'Ended' state!"), EOnlineSessionState::ToString(Session->SessionState));

		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// #NOTE #SESSIONv2 There is a 'Starting' state before this 'InProgress' state - intended to allow for sending data
	// to backend for starting session. Maybe want to supply some kind of handler here to allow developer to make
	// those calls?
	Session->SessionState = EOnlineSessionState::InProgress;

	AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
		SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, true);
	});
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData /*= true*/)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; bShouldRefreshOnlineData: %s"), *SessionName.ToString(), LOG_BOOL_FORMAT(bShouldRefreshOnlineData));

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session with name '%s' as it does not exist!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, false);
		});
		return false;
	}

	Session->SessionSettings = UpdatedSessionSettings;
	
	// If we don't need to refresh our session data on the backend, just bail here
	if (!bShouldRefreshOnlineData)
	{
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
		});
		AB_OSS_INTERFACE_TRACE_END(TEXT("Skipping updating session settings on backend as bShouldRefreshOnlineData is marked false!"));
		return false;
	}

	EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateGameSessionV2>(AccelByteSubsystem, SessionName, UpdatedSessionSettings);
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		if (IsRunningDedicatedServer())
		{
			AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
				SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
			});
			AB_OSS_INTERFACE_TRACE_END(TEXT("Game servers are not able to update party sessions!"));
			return false;
		}

		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdatePartyV2>(AccelByteSubsystem, SessionName, UpdatedSessionSettings);
	}
	
	AB_OSS_INTERFACE_TRACE_END(TEXT("Created async task to update session data on backend!"));
	return true;
}

bool FOnlineSessionV2AccelByte::EndSession(FName SessionName)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to end session as the session does not exist locally!"));

		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// If the session is not in progress, then don't allow us to end it!
	if (Session->SessionState != EOnlineSessionState::InProgress)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to end session in state '%s'! Session needs to be in an 'InProgress' state!"), EOnlineSessionState::ToString(Session->SessionState));

		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// #NOTE #SESSIONv2 There is also an 'Ending' before this one state for committing stats and other data to the backend.
	// Maybe a future improvement to have some kind of mechanism for allowing the developer to commit end game info before
	// fully ending session?
	Session->SessionState = EOnlineSessionState::Ended;

	AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
		SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, true);
	});

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate /*= FOnDestroySessionCompleteDelegate()*/)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session as the session does not exist locally!"));

		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName, CompletionDelegate]() {
			CompletionDelegate.ExecuteIfBound(SessionName, false);
			SessionInterface->TriggerOnDestroySessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	ensure(SessionInfo.IsValid());

	if (SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P)
	{
		TeardownAccelByteP2PConnection();
	}

	// Bail if we are already destroying session!
	if (Session->SessionState == EOnlineSessionState::Destroying)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to destroy session already in the process of being destroyed!"));
		// Purposefully not calling delegate here, as we are still awaiting the previous destroy call to execute delegate
		return false;
	}

	Session->SessionState = EOnlineSessionState::Destroying;

	if (!IsRunningDedicatedServer())
	{
		// Grab local owner ID so that we can make a call to leave session on their behalf
		ensure(Session->LocalOwnerId.IsValid());
		const FUniqueNetIdRef SessionOwnerId = Session->LocalOwnerId.ToSharedRef();

		// Fire off call to leave session. Appropriate delegates will be fired by this method, along with clean up of sessions.
		EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);

		const FOnLeaveSessionComplete OnLeaveSessionCompleteDelegate = FOnLeaveSessionComplete::CreateLambda([CompletionDelegate, SessionName](bool bWasSuccessful, FString SessionId) {
			CompletionDelegate.ExecuteIfBound(SessionName, bWasSuccessful);
		});
		LeaveSession(SessionOwnerId.Get(), SessionType, Session->GetSessionIdStr(), OnLeaveSessionCompleteDelegate);
		
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending request for player '%s' to leave session!"), *SessionOwnerId->ToDebugString());
	}
	else
	{
		// If this is a server, just remove the session from the interface and move on
		RemoveNamedSession(SessionName);
		
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName, CompletionDelegate]() {
			CompletionDelegate.ExecuteIfBound(SessionName, true);
			SessionInterface->TriggerOnDestroySessionCompleteDelegates(SessionName, true);
		});
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	}

	return true;
}

bool FOnlineSessionV2AccelByte::IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find player in session '%s' as the session provided doesn't exist!"), *SessionName.ToString());
		return false;
	}

	// Create a unique ID matcher instance to check if the registered players array contains the unique ID specified
	FUniqueNetIdMatcher PlayerMatch(UniqueId);
	return Session->RegisteredPlayers.ContainsByPredicate(PlayerMatch);
}

bool FOnlineSessionV2AccelByte::StartMatchmaking(const TArray<TSharedRef<const FUniqueNetId>>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	// For UE 4.25 or lower, this start matchmaking function is the only one defined in the interface. In later versions
	// we want to use the new method with FSessionMatchmakingUser structures. However, for earlier versions, just call
	// into that method for back compat.
	TArray<FSessionMatchmakingUser> LocalUsers;
	for (const TSharedRef<const FUniqueNetId>& LocalPlayer : LocalPlayers)
	{
		FSessionMatchmakingUser LocalUser{LocalPlayer, FOnlineKeyValuePairs<FString, FVariantData>()};
		LocalUsers.Emplace(LocalUser);
	}

	return StartMatchmaking(LocalUsers, SessionName, NewSessionSettings, SearchSettings, FOnStartMatchmakingComplete());
#else
	UE_LOG_AB(Warning, TEXT("FOnlineSessionV2AccelByte::StartMatchmaking without a complete delegate is no longer supported. Please use the version with a CompletionDelegate attached."));
	return false;
#endif
}

bool FOnlineSessionV2AccelByte::StartMatchmaking(const TArray<FSessionMatchmakingUser>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnStartMatchmakingComplete& CompletionDelegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s; SessionName: %s"), ((LocalPlayers.IsValidIndex(0)) ? *LocalPlayers[0].UserId->ToDebugString() : TEXT("")), *SessionName.ToString());

	// Check if we already have a session stored with the name specified, if so, inform that they have to leave before matchmaking
	if (GetNamedSession(SessionName) != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start matchmaking for session '%s' as we are already in a session with that name! Call LeaveSession before starting matchmaking!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), EmptyResults);
		});
		return false;
	}

	// Fail the matchmaking request if we are already matchmaking currently
	if (SearchSettings->SearchState != EOnlineAsyncTaskState::NotStarted || (CurrentMatchmakingSearchHandle.IsValid() && CurrentMatchmakingSearchHandle->SearchState != EOnlineAsyncTaskState::NotStarted))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start matchmaking as we are already currently matchmaking!"));
		AccelByteSubsystem->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::AlreadyPending), EmptyResults);
		});
		return false;
	}
	
	SessionSearchPtr = SearchSettings;
	ExplicitSessionSettings = MakeShared<FOnlineSessionSettings>(NewSessionSettings);

	// Make sure that we have only one player passed into the local players array to matchmake with
	if (LocalPlayers.Num() < 1)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start matchmaking without specifying at least one local player!"));
		AccelByteSubsystem->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), EmptyResults);
		});
		return false;
	}

	if (LocalPlayers.Num() > 1)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("StartMatchmaking does not support matchmaking with more than one local user!"));
		AccelByteSubsystem->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), EmptyResults);
		});
		return false;
	}

	FString MatchPool{};
	if (!SearchSettings->QuerySettings.Get(SETTING_SESSION_MATCHPOOL, MatchPool) || MatchPool.IsEmpty())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("No match pool was set in the session search settings! In order to start matchmaking, you must specify what match pool to search by setting SETTING_SESSION_MATCHPOOL in the SearchSettings passed in!"));
		AccelByteSubsystem->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), EmptyResults);
		});
		return false;
	}

	// Convert our session search handle into an AccelByte subclass so that we can store ticket ID
	CurrentMatchmakingSearchHandle = MakeShared<FOnlineSessionSearchAccelByte>(SearchSettings);
	CurrentMatchmakingSearchHandle->SearchingPlayerId = LocalPlayers[0].UserId;
	CurrentMatchmakingSearchHandle->SearchingSessionName = SessionName;

	SearchSettings = CurrentMatchmakingSearchHandle.ToSharedRef(); // Make sure that the caller also gets the new subclassed search handle
	CurrentMatchmakingSessionSettings = NewSessionSettings;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteStartV2Matchmaking>(AccelByteSubsystem, CurrentMatchmakingSearchHandle.ToSharedRef(), SessionName, MatchPool, CompletionDelegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);
	ensure(PlayerId.IsValid());

	return CancelMatchmaking(PlayerId.ToSharedRef().Get(), SessionName);
}

bool FOnlineSessionV2AccelByte::CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; SessionName: %s"), *SearchingPlayerId.ToString(), *SessionName.ToString());

	if (!CurrentMatchmakingSearchHandle.IsValid() || CurrentMatchmakingSearchHandle->SearchState != EOnlineAsyncTaskState::InProgress)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking as we are not currently running a matchmaking query!"));
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
		});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCancelV2Matchmaking>(AccelByteSubsystem, CurrentMatchmakingSearchHandle.ToSharedRef(), SessionName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);
	ensure(PlayerId.IsValid());

	return FindSessions(PlayerId.ToSharedRef().Get(), SearchSettings);
}

bool FOnlineSessionV2AccelByte::FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s"), *SearchingPlayerId.ToDebugString());

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindGameSessionsV2>(AccelByteSubsystem, SearchingPlayerId, SearchSettings);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; SessionId: %s"), *SearchingUserId.ToDebugString(), *SessionId.ToDebugString());

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystem, SearchingUserId, SessionId, CompletionDelegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off async task to find session with ID '%s'!"), *SessionId.ToDebugString());
	return true;
}

bool FOnlineSessionV2AccelByte::CancelFindSessions()
{
	return false;
}

bool FOnlineSessionV2AccelByte::PingSearchResults(const FOnlineSessionSearchResult& SearchResult)
{
	return false;
}

bool FOnlineSessionV2AccelByte::JoinSession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	return JoinSession(PlayerId.ToSharedRef().Get(), SessionName, DesiredSession);
}

bool FOnlineSessionV2AccelByte::JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	if (GetNamedSession(SessionName) != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session with name '%s' as a session with that name already exists!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
		});
		return false;
	}

	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, DesiredSession.Session);
	NewSession->SessionState = EOnlineSessionState::Creating;
	NewSession->LocalOwnerId = LocalUserId.AsShared();

	EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(NewSession->SessionSettings);
	if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJoinV2GameSession>(AccelByteSubsystem, LocalUserId, SessionName);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Spawning async task to join game session on backend!"));
		return true;
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJoinV2Party>(AccelByteSubsystem, LocalUserId, SessionName);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Spawning async task to join party session on backend!"));
		return true;
	}

	// #TODO #SESSIONv2 Not sure if for non-game and non-party sessions we also want to send a join request...
	AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
		SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::Success);
	});

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	return FindFriendSession(PlayerId.ToSharedRef().Get(), Friend);
}

bool FOnlineSessionV2AccelByte::FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend)
{
	return false;
}

bool FOnlineSessionV2AccelByte::FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& FriendList)
{
	return false;
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	return SendSessionInviteToFriend(PlayerId.ToSharedRef().Get(), SessionName, Friend);
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; FriendId: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *Friend.ToDebugString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to invite player to session as the session does not exist locally!"));
		return false;
	}

	EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendV2GameSessionInvite>(AccelByteSubsystem, LocalUserId, SessionName, Friend);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending invite to player for game session!"));
		return true;
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendV2PartyInvite>(AccelByteSubsystem, LocalUserId, SessionName, Friend);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending invite to player for party session!"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends)
{
	UE_LOG_AB(Warning, TEXT("FOnlineSessionV2AccelByte::SendSessionInviteToFriends is not implemented! Please send invites one at a time through FOnlineSessionV2AccelByte::SendSessionInviteToFriend!"));
	return false;
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends)
{
	UE_LOG_AB(Warning, TEXT("FOnlineSessionV2AccelByte::SendSessionInviteToFriends is not implemented! Please send invites one at a time through FOnlineSessionV2AccelByte::SendSessionInviteToFriend!"));
	return false;
}

bool FOnlineSessionV2AccelByte::GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType /*= NAME_GamePort*/)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		ConnectInfo = TEXT("");
		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	ensure(SessionInfo.IsValid());

	if (!SessionInfo->HasConnectionInfo())
	{
		ConnectInfo = TEXT("");
		return false;
	}

	// #TODO #SESSIONv2 Since we can change the desired connection port from the PortType arg, we may want to
	// allow developers to get other ports than the main game port from the backend server definition?
	ConnectInfo = SessionInfo->GetConnectionString();
	return true;
}

bool FOnlineSessionV2AccelByte::GetResolvedConnectString(const FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo)
{
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SearchResult.Session.SessionInfo);
	if (!SessionInfo.IsValid())
	{
		ConnectInfo = TEXT("");
		return false;
	}

	if (!SessionInfo->HasConnectionInfo())
	{
		ConnectInfo = TEXT("");
		return false;
	}

	// #TODO #SESSIONv2 Since we can change the desired connection port from the PortType arg, we may want to
	// allow developers to get other ports than the main game port from the backend server definition?
	ConnectInfo = SessionInfo->GetConnectionString();
	return true;
}

FOnlineSessionSettings* FOnlineSessionV2AccelByte::GetSessionSettings(FName SessionName)
{
	FScopeLock ScopeLock(&SessionLock);

	TSharedPtr<FNamedOnlineSession>* FoundNamedSession = Sessions.Find(SessionName);
	if (FoundNamedSession)
	{
		return &(*FoundNamedSession)->SessionSettings;
	}

	return nullptr;
}

bool FOnlineSessionV2AccelByte::RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited)
{
	TArray<TSharedRef<const FUniqueNetId>> PlayerArray;
	PlayerArray.Emplace(PlayerId.AsShared());

	return RegisterPlayers(SessionName, PlayerArray, bWasInvited);
}

bool FOnlineSessionV2AccelByte::RegisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players, bool bWasInvited /*= false*/)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; bWasInvited: %s"), *SessionName.ToString(), LOG_BOOL_FORMAT(bWasInvited));

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		if (!IsRunningDedicatedServer())
		{
			// If we do not have a session with this name, and we are not running a DS, just fail the call
			AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register players to session as a session with name '%s' does not exist!"), *SessionName.ToString());

			AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
				SessionInterface->TriggerOnRegisterPlayersCompleteDelegates(SessionName, Players, false);
			});
		}
		else
		{
			// Otherwise, queue this call up for later so that once we get session info we can make it with proper data
			SessionCallsAwaitingServerSession.Emplace([SessionInterface = SharedThis(this), SessionName, Players, bWasInvited]() {
				SessionInterface->RegisterPlayers(SessionName, Players, bWasInvited);
			});

			if (!bIsGettingServerAssociatedSession)
			{
				GetServerAssociatedSession(SessionName);
			}

			// #NOTE Deliberately not calling delegate here so that it gets called once server session is grabbed
		}
		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		return false;
	}

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!ensure(SessionData.IsValid()))
	{
		return false;
	}

	for (const TSharedRef<const FUniqueNetId>& PlayerToAdd : Players)
	{
		FUniqueNetIdMatcher PlayerToAddMatcher(PlayerToAdd.Get());
		int32 FoundPlayerIndex = Session->RegisteredPlayers.IndexOfByPredicate(PlayerToAddMatcher);
		if (FoundPlayerIndex == INDEX_NONE)
		{
			Session->RegisteredPlayers.Emplace(PlayerToAdd);

			// Update session player counts based on join type
			const bool bClosedSession = SessionData->Configuration.Joinability == EAccelByteV2SessionJoinability::INVITE_ONLY || SessionData->Configuration.Joinability == EAccelByteV2SessionJoinability::CLOSED;
			if (bClosedSession)
			{
				Session->NumOpenPrivateConnections--;
				if (Session->NumOpenPrivateConnections < 0)
				{
					Session->NumOpenPrivateConnections = 0;
				}
			}
			else
			{
				Session->NumOpenPublicConnections--;
				if (Session->NumOpenPublicConnections < 0)
				{
					Session->NumOpenPublicConnections = 0;
				}
			}
		}
	}

	AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
		SessionInterface->TriggerOnRegisterPlayersCompleteDelegates(SessionName, Players, true);
	});

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId)
{
	TArray<TSharedRef<const FUniqueNetId>> PlayerArray;
	PlayerArray.Emplace(PlayerId.AsShared());

	return UnregisterPlayers(SessionName, PlayerArray);
}

bool FOnlineSessionV2AccelByte::UnregisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		if (!IsRunningDedicatedServer())
		{
			// If we are not running a server instance and there is no session, then just fail as the player needs to have one to register
			AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister players from session as a session with name '%s' does not exist!"), *SessionName.ToString());

			AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
				SessionInterface->TriggerOnUnregisterPlayersCompleteDelegates(SessionName, Players, false);
			});
		}
		else
		{
			// Otherwise, queue this call up for later so that once we get session info we can make it with proper data
			SessionCallsAwaitingServerSession.Emplace([SessionInterface = SharedThis(this), SessionName, Players]() {
				SessionInterface->UnregisterPlayers(SessionName, Players);
			});

			if (!bIsGettingServerAssociatedSession)
			{
				GetServerAssociatedSession(SessionName);
			}

			// #NOTE Deliberately not calling delegate here so that it gets called once server session is grabbed
		}
		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		return false;
	}

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!ensure(SessionData.IsValid()))
	{
		return false;
	}

	for (const TSharedRef<const FUniqueNetId>& PlayerToRemove : Players)
	{
		FUniqueNetIdMatcher PlayerToRemoveMatcher(PlayerToRemove.Get());
		int32 FoundPlayerIndex = Session->RegisteredPlayers.IndexOfByPredicate(PlayerToRemoveMatcher);
		if (FoundPlayerIndex != INDEX_NONE)
		{
			Session->RegisteredPlayers.RemoveAt(FoundPlayerIndex);

			// Update session player counts based on join type
			const bool bClosedSession = SessionData->Configuration.Joinability == EAccelByteV2SessionJoinability::INVITE_ONLY || SessionData->Configuration.Joinability == EAccelByteV2SessionJoinability::CLOSED;
			if (bClosedSession)
			{
				Session->NumOpenPrivateConnections++;
			}
			else
			{
				Session->NumOpenPublicConnections++;
			}
		}
	}

	AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
		SessionInterface->TriggerOnUnregisterPlayersCompleteDelegates(SessionName, Players, true);
	});

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate)
{
	// Intentionally unimplemented
}

void FOnlineSessionV2AccelByte::UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate)
{
	// Intentionally unimplemented
}

int32 FOnlineSessionV2AccelByte::GetNumSessions()
{
	return Sessions.Num();
}

void FOnlineSessionV2AccelByte::DumpSessionState()
{
}

bool FOnlineSessionV2AccelByte::QueryAllInvites(const FUniqueNetId& PlayerId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s"), *PlayerId.ToDebugString());

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites>(AccelByteSubsystem, PlayerId);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

TArray<FOnlineSessionInviteAccelByte> FOnlineSessionV2AccelByte::GetAllInvites() const
{
	return SessionInvites;
}

TArray<FOnlineSessionInviteAccelByte> FOnlineSessionV2AccelByte::GetAllGameInvites() const
{
	return SessionInvites.FilterByPredicate([](const FOnlineSessionInviteAccelByte& Invite) {
		return Invite.SessionType == EOnlineSessionTypeAccelByte::GameSession;
	});
}

TArray<FOnlineSessionInviteAccelByte> FOnlineSessionV2AccelByte::GetAllPartyInvites() const
{
	return SessionInvites.FilterByPredicate([](const FOnlineSessionInviteAccelByte& Invite) {
		return Invite.SessionType == EOnlineSessionTypeAccelByte::PartySession;
	});
}

bool FOnlineSessionV2AccelByte::RejectInvite(const FUniqueNetId& PlayerId, const FOnlineSessionInviteAccelByte& InvitedSession, const FOnRejectSessionInviteComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s; InvitedSessionId: %s"), *PlayerId.ToDebugString(), *InvitedSession.Session.GetSessionIdStr());

	EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(InvitedSession.Session.Session.SessionSettings);
	if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRejectV2GameSessionInvite>(AccelByteSubsystem, PlayerId, InvitedSession.Session, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending rejection for game session invite!"));
		return true;
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRejectV2PartyInvite>(AccelByteSubsystem, PlayerId, InvitedSession.Session, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending rejection for party session invite!"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

bool FOnlineSessionV2AccelByte::RestoreActiveSessions(const FUniqueNetId& LocalUserId, const FOnRestoreActiveSessionsComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s"), *LocalUserId.ToDebugString());
	
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRestoreAllV2Sessions>(AccelByteSubsystem, LocalUserId, Delegate);
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

TArray<FOnlineRestoredSessionAccelByte> FOnlineSessionV2AccelByte::GetAllRestoredSessions() const
{
	return RestoredSessions;
}

TArray<FOnlineRestoredSessionAccelByte> FOnlineSessionV2AccelByte::GetAllRestoredPartySessions() const
{
	return RestoredSessions.FilterByPredicate([](const FOnlineRestoredSessionAccelByte& RestoredSession) {
		return RestoredSession.SessionType == EOnlineSessionTypeAccelByte::PartySession;
	});
}

TArray<FOnlineRestoredSessionAccelByte> FOnlineSessionV2AccelByte::GetAllRestoredGameSessions() const
{
	return RestoredSessions.FilterByPredicate([](const FOnlineRestoredSessionAccelByte& RestoredSession) {
		return RestoredSession.SessionType == EOnlineSessionTypeAccelByte::GameSession;
	});
}

bool FOnlineSessionV2AccelByte::LeaveRestoredSession(const FUniqueNetId& LocalUserId, const FOnlineRestoredSessionAccelByte& SessionToLeave, const FOnLeaveSessionComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionToLeaveId: %s"), *LocalUserId.ToDebugString(), *SessionToLeave.Session.GetSessionIdStr());
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return LeaveSession(LocalUserId, SessionToLeave.SessionType, SessionToLeave.Session.GetSessionIdStr(), Delegate);
}

FNamedOnlineSession* FOnlineSessionV2AccelByte::GetNamedSessionById(const FString& SessionIdString)
{
	FScopeLock ScopeLock(&SessionLock);
	for (const TPair<FName, TSharedPtr<FNamedOnlineSession>>& SessionPair : Sessions)
	{
		if (SessionPair.Value->GetSessionIdStr().Equals(SessionIdString))
		{
			return SessionPair.Value.Get();
		}
	}

	return nullptr;
}

void FOnlineSessionV2AccelByte::RegisterServer(FName SessionName, const FOnRegisterServerComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));
	
	// Only dedicated servers should be able to register to Armada
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register server to Armada as the current game instance is not a dedicated server!"));
		return;
	}

	// For an Armada-spawned server pod, there will always be a POD_NAME environment variable set. For local servers this
	// will most likely never be the case. With this, if we have the pod name variable, then register as a non-local server
	// otherwise, register as a local server.
	const FString PodName = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME"));
	if (!PodName.IsEmpty())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRegisterRemoteServerV2>(AccelByteSubsystem, SessionName, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Registering cloud server to Armada!"));
	}
	else
	{
		// #NOTE Deliberately leaving out session name from the local task, as nothing in that task relies on the name
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRegisterLocalServerV2>(AccelByteSubsystem, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Registering locally hosted server to Armada!"));
	}
}

void FOnlineSessionV2AccelByte::UnregisterServer(FName SessionName, const FOnUnregisterServerComplete& Delegate /*= FOnUnregisterServerComplete()*/)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Only dedicated servers should be able to register to Armada
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register server to Armada as the current game instance is not a dedicated server!"));
		return;
	}

	// For an Armada-spawned server pod, there will always be a POD_NAME environment variable set. For local servers this
	// will most likely never be the case. With this, if we have the pod name variable, then register as a non-local server
	// otherwise, register as a local server.
	const FString PodName = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME"));
	if (!PodName.IsEmpty())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2>(AccelByteSubsystem, SessionName, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Unregistering cloud server to Armada!"));
	}
	else
	{
		// #NOTE Deliberately leaving out session name from the local task, as nothing in that task relies on the name
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUnregisterLocalServerV2>(AccelByteSubsystem, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Unregistering locally hosted server to Armada!"));
	}
}

EOnlineSessionTypeAccelByte FOnlineSessionV2AccelByte::GetSessionTypeByName(const FName& SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		return EOnlineSessionTypeAccelByte::Unknown;
	}

	return GetSessionTypeFromSettings(Session->SessionSettings);
}

EOnlineSessionTypeAccelByte FOnlineSessionV2AccelByte::GetSessionTypeFromSettings(const FOnlineSessionSettings& Settings) const
{
	FString SessionTypeString = TEXT("");
	if (!Settings.Get(SETTING_SESSION_TYPE, SessionTypeString))
	{
		return EOnlineSessionTypeAccelByte::Unknown;
	}

	return GetSessionTypeFromString(SessionTypeString);
}

FUniqueNetIdPtr FOnlineSessionV2AccelByte::GetSessionLeaderId(const FNamedOnlineSession* Session) const
{
	const EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EOnlineSessionTypeAccelByte::PartySession)
	{
		return nullptr;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		return nullptr;
	}

	return SessionInfo->GetLeaderId();
}

bool FOnlineSessionV2AccelByte::KickPlayer(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToKick)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToKick: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PlayerIdToKick.ToDebugString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot kick player from session as the session does not exist locally!"));
		return false;
	}

	EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		// #TODO Revisit - not sure if game sessions will have kick functionality, but for player owned sessions they *probably* should
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteKickV2Party>(AccelByteSubsystem, LocalUserId, SessionName, PlayerIdToKick);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to kick player from party session!"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

FString FOnlineSessionV2AccelByte::ConvertToRegionListString(const TArray<FString>& Regions) const
{
	FString OutString;
	
	for (int32 Index = 0; Index < Regions.Num(); Index++)
	{
		OutString.Append(Regions[Index]);

		if (Index != Regions.Num() - 1)
		{
			OutString.AppendChar(',');
		}
	}

	return OutString;
}

TArray<FString> FOnlineSessionV2AccelByte::ConvertToRegionArray(const FString& RegionListString) const
{
	TArray<FString> OutList;
	RegionListString.ParseIntoArray(OutList, TEXT(","), 1);
	return OutList;
}

TArray<FString> FOnlineSessionV2AccelByte::GetRegionList(const FUniqueNetId& LocalPlayerId) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s"), *LocalPlayerId.ToDebugString());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalPlayerId);
	if (!ApiClient.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not get list of regions as we could not get an API client for the user specified!"));
		return TArray<FString>();
	}

	// Get latencies to QOS regions, and sort by lowest latency
	TArray<TPair<FString, float>> Latencies = ApiClient->Qos.GetCachedLatencies();
	Latencies.Sort([](const TPair<FString, float>& LeftHandLatency, const TPair<FString, float>& RightHandLatency) {
		return LeftHandLatency.Value < RightHandLatency.Value;
	});

	// Now add each region to the output array
	TArray<FString> OutRegions;
	for (const TPair<FString, float>& Latency : Latencies)
	{
		OutRegions.Emplace(Latency.Key);
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return OutRegions;
}

TSharedPtr<FOnlineSessionSearchAccelByte> FOnlineSessionV2AccelByte::GetCurrentMatchmakingSearchHandle() const
{
	return CurrentMatchmakingSearchHandle;
}

int32 FOnlineSessionV2AccelByte::GetSessionPlayerCount(const FName& SessionName) const
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		return -1;
	}

	return GetSessionPlayerCount(*Session);
}

int32 FOnlineSessionV2AccelByte::GetSessionPlayerCount(const FOnlineSession& Session) const
{
	bool bHasJoinability = false;
	bool bIsOpenSession = false;

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session.SessionInfo);
	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = (SessionInfo.IsValid()) ? SessionInfo->GetBackendSessionData() : nullptr;
	if (SessionData.IsValid())
	{
		bIsOpenSession = SessionData->Configuration.Joinability == EAccelByteV2SessionJoinability::OPEN;
		bHasJoinability = true;
	}
	else
	{
		// Otherwise, try and get joinability from session settings
		const EAccelByteV2SessionJoinability Joinability = GetJoinabiltyFromSessionSettings(Session.SessionSettings);
		if (Joinability != EAccelByteV2SessionJoinability::EMPTY)
		{
			bIsOpenSession = Joinability == EAccelByteV2SessionJoinability::OPEN;
			bHasJoinability = true;
		}
	}

	if (bHasJoinability)
	{
		return (bIsOpenSession) ? Session.SessionSettings.NumPublicConnections - Session.NumOpenPublicConnections : Session.SessionSettings.NumPrivateConnections - Session.NumOpenPrivateConnections;
	}

	// Otherwise, we're still probably creating the session and have no join type set, so just get the maximum of either
	return std::max(Session.SessionSettings.NumPublicConnections - Session.NumOpenPublicConnections, Session.SessionSettings.NumPrivateConnections - Session.NumOpenPrivateConnections);
}

int32 FOnlineSessionV2AccelByte::GetSessionMaxPlayerCount(const FName& SessionName) const
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		return -1;
	}

	return GetSessionMaxPlayerCount(*Session);
}

int32 FOnlineSessionV2AccelByte::GetSessionMaxPlayerCount(const FOnlineSession& Session) const
{
	bool bHasJoinability = false;
	bool bIsOpenSession = false;

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session.SessionInfo);
	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = (SessionInfo.IsValid()) ? SessionInfo->GetBackendSessionData() : nullptr;
	if (SessionData.IsValid())
	{
		bIsOpenSession = SessionData->Configuration.Joinability == EAccelByteV2SessionJoinability::OPEN;
		bHasJoinability = true;
	}
	else
	{
		const EAccelByteV2SessionJoinability Joinability = GetJoinabiltyFromSessionSettings(Session.SessionSettings);
		if (Joinability != EAccelByteV2SessionJoinability::EMPTY)
		{
			bIsOpenSession = Joinability == EAccelByteV2SessionJoinability::OPEN;
			bHasJoinability = true;
		}
	}

	if (bHasJoinability)
	{
		return (bIsOpenSession) ? Session.SessionSettings.NumPublicConnections : Session.SessionSettings.NumPrivateConnections;
	}

	// Otherwise, we're probably still creating this session without a join type, so just use the maximum of either of the values
	return std::max(Session.SessionSettings.NumPublicConnections, Session.SessionSettings.NumPrivateConnections);
}

bool FOnlineSessionV2AccelByte::SetSessionMaxPlayerCount(const FName& SessionName, int32 NewMaxPlayerCount) const
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		return false;
	}

	return SetSessionMaxPlayerCount(Session, NewMaxPlayerCount);
}

bool FOnlineSessionV2AccelByte::SetSessionMaxPlayerCount(FOnlineSession* Session, int32 NewMaxPlayerCount) const
{
	bool bHasJoinability = false;
	bool bIsOpenSession = false;

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = (SessionInfo.IsValid()) ? SessionInfo->GetBackendSessionData() : nullptr;
	if (SessionData.IsValid())
	{
		// If we have session data, use it to determine the joinability of this session, and thus what number of connections to use
		bIsOpenSession = SessionData->Configuration.Joinability == EAccelByteV2SessionJoinability::OPEN;
		bHasJoinability = true;
	}
	else
	{
		// Otherwise, try and get joinability from session settings
		const EAccelByteV2SessionJoinability Joinability = GetJoinabiltyFromSessionSettings(Session->SessionSettings);
		if (Joinability != EAccelByteV2SessionJoinability::EMPTY)
		{
			bIsOpenSession = Joinability == EAccelByteV2SessionJoinability::OPEN;
			bHasJoinability = true;
		}
	}

	if (!bHasJoinability)
	{
		// We were unable to find any indication of the joinability of this session. If you are just creating the session, you
		// can just set either NumPublicConnections or NumPrivateConnections. These fields will update after creation on backend
		// to reflect correct values. From then on, you will want to use the getters and setters from this method for convenience.
		return false;
	}

	if (bIsOpenSession)
	{
		Session->SessionSettings.NumPublicConnections = NewMaxPlayerCount;
	}
	else
	{
		Session->SessionSettings.NumPrivateConnections = NewMaxPlayerCount;
	}
	return true;
}

FString FOnlineSessionV2AccelByte::FormatToInNotInSearchParameter(const TArray<FString>& QueryStrings) const
{
	FString OutParam = TEXT("");
	for (int32 Index = 0; Index < QueryStrings.Num(); Index++)
	{
		OutParam.Append(QueryStrings[Index]);
		
		if (Index < QueryStrings.Num() - 1)
		{
			// If this is not the end of the array, then we want to add a special string of characters that will denote that we
			// are separating between two values.
			OutParam.Append(TEXT("%absep%"));
		}
	}

	return OutParam;
}

FString FOnlineSessionV2AccelByte::FormatToInNotInSearchParameter(const TArray<double>& QueryNumbers) const
{
	FString OutParam = TEXT("");
	for (int32 Index = 0; Index < QueryNumbers.Num(); Index++)
	{
		OutParam.Append(FString::SanitizeFloat(QueryNumbers[Index]));
		if (Index < QueryNumbers.Num() - 1)
		{
			// If this is not the end of the array, then we want to add a special string of characters that will denote that we
			// are separating between two values.
			OutParam.Append(TEXT("%absep%"));
		}
	}

	return OutParam;
}

bool FOnlineSessionV2AccelByte::RefreshSession(const FName& SessionName, const FOnRefreshSessionComplete& Delegate)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot refresh session as the session does not exist locally!"));
		return false;
	}

	EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EOnlineSessionTypeAccelByte::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshV2GameSession>(AccelByteSubsystem, SessionName, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to refresh local game session with backend data!"));
		return true;
	}
	else if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshV2PartySession>(AccelByteSubsystem, SessionName, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to refresh local party session with backend data!"));
		return true;
	}

	return false;
}

void FOnlineSessionV2AccelByte::OnInvitedToGameSessionNotification(FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *InviteEvent.SessionID);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InviteEvent.SessionID);
	ensure(SessionUniqueId.IsValid());

	const FOnSingleSessionResultCompleteDelegate OnFindGameSessionForInviteCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindGameSessionForInviteComplete);
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystem, PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindGameSessionForInviteCompleteDelegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnFindGameSessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; bWasSuccessful: %s; SessionId: %s"), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful), *Result.GetSessionIdStr());

	if (!bWasSuccessful)
	{
		AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to notify player about invite to session as we failed to query data about session from backend!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	FOnlineSessionInviteAccelByte NewInvite;
	NewInvite.SessionType = EOnlineSessionTypeAccelByte::GameSession;
	NewInvite.Session = Result;
	NewInvite.SenderId = nullptr; // #NOTE Null for now until we get sender ID for game session invites
	SessionInvites.Emplace(NewInvite);

	// #NOTE #SESSIONv2 Currently game session invites do not contain sender IDs, I assume because these are currently
	// meant to be system invites. Such as if matchmaking creates you a session and thus invites you to it. However,
	// if we do support player to player invites, then we'd need a sender ID here.
	const FUniqueNetIdRef FromId = FUniqueNetIdAccelByteUser::Invalid();
	TriggerOnSessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), FromId.Get(), AccelByteSubsystem->GetAppId(), Result);
	TriggerOnV2SessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), FromId.Get(), NewInvite);
	TriggerOnInviteListUpdatedDelegates();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGameSessionMembersChangedNotification(FAccelByteModelsV2GameSessionMembersChangedEvent MembersChangedEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s; JoinerId: %s"), *MembersChangedEvent.SessionID, *MembersChangedEvent.JoinerID);

	HandleSessionMembersChangedNotification(MembersChangedEvent.SessionID, MembersChangedEvent.Members, MembersChangedEvent.JoinerID);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGameSessionUpdatedNotification(FAccelByteModelsV2GameSession UpdatedGameSession, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *UpdatedGameSession.ID);

	FNamedOnlineSession* Session = GetNamedSessionById(UpdatedGameSession.ID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with with new attributes as we do not have the session stored locally!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with new attributes as session does not have a valid session info instance!"));
		return;
	}

	UpdateInternalGameSession(Session->SessionName, UpdatedGameSession);
	TriggerOnUpdateSessionCompleteDelegates(Session->SessionName, true);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnDsStatusChangedNotification(FAccelByteModelsV2DSStatusChangedNotif DsStatusChangeEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *DsStatusChangeEvent.SessionID);

	if (!DsStatusChangeEvent.Error.IsEmpty())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("DS status changed with an error message attached! Error message: %s"), *DsStatusChangeEvent.Error);
		return;
	}

	// If the new status is not ready, then we cannot do anything, so just bail
	if (!DsStatusChangeEvent.Game_server.Status.Equals(TEXT("READY"), ESearchCase::IgnoreCase) && !DsStatusChangeEvent.Game_server.Status.Equals(TEXT("BUSY"), ESearchCase::IgnoreCase))
	{
		// #NOTE No warning here as technically nothing bad happened
		return;
	}

	FNamedOnlineSession* Session = GetNamedSessionById(DsStatusChangeEvent.SessionID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with new DS status as session does not exist locally!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with new DS status as session does not have a valid session info instance!"));
		return;
	}

	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2GameSession>(SessionInfo->GetBackendSessionData());
	if (!GameSessionBackendData.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with new DS status as session does not have valid game session data!"));
		return;
	}

	// Just update the server object on the backend data and make call to update connection info
	GameSessionBackendData->DSInformation.Server = DsStatusChangeEvent.Game_server;
	GameSessionBackendData->Version++; // Increment version of the backend model for future updates
	SessionInfo->UpdateConnectionInfo();

	TriggerOnSessionServerUpdateDelegates(Session->SessionName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineSessionV2AccelByte::IsInPartySession() const
{
	return GetPartySession() != nullptr;
}

FNamedOnlineSession* FOnlineSessionV2AccelByte::GetPartySession() const
{
	FScopeLock ScopeLock(&SessionLock);
	for (const TPair<FName, TSharedPtr<FNamedOnlineSession>>& SessionPair : Sessions)
	{
		const TSharedPtr<FNamedOnlineSession> Session = SessionPair.Value;
		ensure(Session.IsValid());

		const EOnlineSessionTypeAccelByte SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
		if (SessionType == EOnlineSessionTypeAccelByte::PartySession)
		{
			return Session.Get();
		}
	}

	return nullptr;
}

void FOnlineSessionV2AccelByte::OnInvitedToPartySessionNotification(FAccelByteModelsV2PartyInvitedEvent InviteEvent, int32 LocalUserNum)
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InviteEvent.PartyID);
	ensure(SessionUniqueId.IsValid());

	const FOnSingleSessionResultCompleteDelegate OnFindPartySessionForInviteCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindPartySessionForInviteComplete, InviteEvent);
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2PartyById>(AccelByteSubsystem, PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindPartySessionForInviteCompleteDelegate);
}

void FOnlineSessionV2AccelByte::OnFindPartySessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2PartyInvitedEvent InviteEvent)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; bWasSuccessful: %s; SessionId: %s; SenderId: %s"), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful), *Result.GetSessionIdStr(), *InviteEvent.SenderID);

	if (!bWasSuccessful)
	{
		AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to notify player about invite to session as we failed to query data about session from backend!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	// Find sender ID in the backend session data so that we can get their platform info for the ID
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Result.Session.SessionInfo);
	ensure(SessionInfo.IsValid());

	const TSharedPtr<FAccelByteModelsV2PartySession> PartySessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	ensure(PartySessionData.IsValid());

	FAccelByteModelsV2SessionUser* FoundSender = PartySessionData->Members.FindByPredicate([&InviteEvent](const FAccelByteModelsV2SessionUser& User) {
		return User.ID == InviteEvent.SenderID;
	});

	if (FoundSender != nullptr)
	{
		// Create a new unique ID instance for the leaving member, complete with their platform info
		FAccelByteUniqueIdComposite IdComponents;
		IdComponents.Id = FoundSender->ID;
		IdComponents.PlatformType = FoundSender->PlatformID;
		IdComponents.PlatformId = FoundSender->PlatformUserID;

		TSharedPtr<const FUniqueNetIdAccelByteUser> SenderId = FUniqueNetIdAccelByteUser::Create(IdComponents);
		ensure(SenderId.IsValid());
	
		FOnlineSessionInviteAccelByte NewInvite;
		NewInvite.SessionType = EOnlineSessionTypeAccelByte::PartySession;
		NewInvite.Session = Result;
		NewInvite.SenderId = SenderId;
		SessionInvites.Emplace(NewInvite);

		TriggerOnSessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), AccelByteSubsystem->GetAppId(), Result);
		TriggerOnV2SessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), NewInvite);
		TriggerOnInviteListUpdatedDelegates();
	}
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnPartySessionMembersChangedNotification(FAccelByteModelsV2PartyMembersChangedEvent MemberChangeEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s; JoinerId: %s"), *MemberChangeEvent.PartyID, *MemberChangeEvent.JoinerID);

	HandleSessionMembersChangedNotification(MemberChangeEvent.PartyID, MemberChangeEvent.Members, MemberChangeEvent.JoinerID);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnPartySessionUpdatedNotification(FAccelByteModelsV2PartySession UpdatedPartySession, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *UpdatedPartySession.ID);

	FNamedOnlineSession* Session = GetNamedSessionById(UpdatedPartySession.ID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with with new attributes as we do not have the session stored locally!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with new attributes as session does not have a valid session info instance!"));
		return;
	}

	UpdateInternalPartySession(Session->SessionName, UpdatedPartySession);
	TriggerOnUpdateSessionCompleteDelegates(Session->SessionName, true);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnPartySessionInviteRejectedNotification(FAccelByteModelsV2PartyUserRejectedEvent RejectEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionId: %s"), LocalUserNum, *RejectEvent.PartyID);

	FNamedOnlineSession* Session = GetNamedSessionById(RejectEvent.PartyID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as we do not have that session stored locally!"), *RejectEvent.PartyID);
		return;
	}

	// Grab session data to update members there
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	ensure(SessionInfo.IsValid());

	TSharedPtr<FAccelByteModelsV2PartySession> SessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	ensure(SessionData.IsValid());

	SessionData->Members = RejectEvent.Members;
	SessionInfo->UpdatePlayerLists();
	SessionInfo->UpdateLeaderId();

	TriggerOnUpdateSessionCompleteDelegates(Session->SessionName, true);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::HandleSessionMembersChangedNotification(const FString& SessionId, const TArray<FAccelByteModelsV2SessionUser>& NewMembers, const FString& JoinerId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s; JoinerId: %s"), *SessionId, *JoinerId);

	FNamedOnlineSession* Session = GetNamedSessionById(SessionId);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update members for session '%s' as we do not have that session stored locally!"), *SessionId);
		return;
	}

	// Grab session data to update members there
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	ensure(SessionInfo.IsValid());

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	ensure(SessionData.IsValid());

	// Grab the old members array in case we need to diff it
	const TArray<FAccelByteModelsV2SessionUser> PreviousMembers = SessionData->Members;

	// Set new members array and update the invite list on session info
	SessionData->Members = NewMembers;
	SessionData->Version++;
	SessionInfo->UpdatePlayerLists();
	SessionInfo->UpdateLeaderId();

	// If the JoinerId for the event is not blank, then we just have to register this player and update session info
	if (!JoinerId.IsEmpty())
	{
		// Find the member object for the player so that we can construct a proper ID for them, with platform information
		const FAccelByteModelsV2SessionUser* FoundUser = NewMembers.FindByPredicate([&JoinerId](const FAccelByteModelsV2SessionUser& User) {
			return User.ID == JoinerId;
		});
		
		if (ensure(FoundUser != nullptr))
		{
			RegisterJoinedSessionMember(Session, *FoundUser);
		}

		return;
	}

	// If we do not have an ID for a user that has joined this session, then we need to diff the previous members array
	// and the new members array to figure out what changed. If the status changes to Leave or Disconnect, we need to
	// unregister that player. If it changes to join or connect we need to register them.
	for (const FAccelByteModelsV2SessionUser& NewMember : NewMembers)
	{
		const FAccelByteModelsV2SessionUser* PreviousMember = PreviousMembers.FindByPredicate([&NewMember](const FAccelByteModelsV2SessionUser& Member) {
			return Member.ID == NewMember.ID;
		});

		// If this user's status hasn't changed, then just move on to next member
		if (PreviousMember != nullptr && PreviousMember->Status == NewMember.Status)
		{
			continue;
		}

		const bool bIsJoinStatus = NewMember.Status == EAccelByteV2SessionMemberStatus::JOINED || NewMember.Status == EAccelByteV2SessionMemberStatus::CONNECTED;
		const bool bIsLeaveStatus = NewMember.Status == EAccelByteV2SessionMemberStatus::LEFT || NewMember.Status == EAccelByteV2SessionMemberStatus::KICKED || NewMember.Status == EAccelByteV2SessionMemberStatus::DROPPED;

		// If this player is joining the session, then we want to register them and fire delegates
		if (bIsJoinStatus)
		{
			RegisterJoinedSessionMember(Session, NewMember);
		}
		else
		{
			UnregisterLeftSessionMember(Session, NewMember);
		}
	}
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::RegisterJoinedSessionMember(FNamedOnlineSession* Session, const FAccelByteModelsV2SessionUser& JoinedMember)
{
	FAccelByteUniqueIdComposite IdComponents;
	IdComponents.Id = JoinedMember.ID;
	IdComponents.PlatformType = JoinedMember.PlatformID;
	IdComponents.PlatformId = JoinedMember.PlatformUserID;

	TSharedPtr<const FUniqueNetIdAccelByteUser> JoinedUserId = FUniqueNetIdAccelByteUser::Create(IdComponents);
	if (ensure(JoinedUserId.IsValid()))
	{
		RegisterPlayer(Session->SessionName, JoinedUserId.ToSharedRef().Get(), false);
		TriggerOnSessionParticipantsChangeDelegates(Session->SessionName, JoinedUserId.ToSharedRef().Get(), true);
	}
}

void FOnlineSessionV2AccelByte::UnregisterLeftSessionMember(FNamedOnlineSession* Session, const FAccelByteModelsV2SessionUser& LeftMember)
{
	FAccelByteUniqueIdComposite IdComponents;
	IdComponents.Id = LeftMember.ID;
	IdComponents.PlatformType = LeftMember.PlatformID;
	IdComponents.PlatformId = LeftMember.PlatformUserID;

	TSharedPtr<const FUniqueNetIdAccelByteUser> LeftUserId = FUniqueNetIdAccelByteUser::Create(IdComponents);
	if (!ensure(LeftUserId.IsValid()))
	{
		return;
	}

	UnregisterPlayer(Session->SessionName, LeftUserId.ToSharedRef().Get());

	// Participant removed delegate seems to be only in 4.27+, guarding it so we don't fail to compile on other engine versions.
#if ENGINE_MAJOR_VERSION >= 5 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27)
	TriggerOnSessionParticipantRemovedDelegates(Session->SessionName, LeftUserId.ToSharedRef().Get());
#endif
	TriggerOnSessionParticipantsChangeDelegates(Session->SessionName, LeftUserId.ToSharedRef().Get(), false);

	// If the user that has left is ourselves, then we want to destroy this session as well. Developers will need to listen
	// to OnDestroySessionComplete to ensure that they are catching being booted out of a session from the backend.
	if (LeftUserId.ToSharedRef().Get() == Session->LocalOwnerId.ToSharedRef().Get())
	{
		DestroySession(Session->SessionName);
	}
}

void FOnlineSessionV2AccelByte::OnMatchmakingStartedNotification(FAccelByteModelsV2StartMatchmakingNotif MatchmakingStartedNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (CurrentMatchmakingSearchHandle.IsValid())
	{
		// Since we already have a matchmaking search handle, we don't need to do anything else here and can bail
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	ensure(IdentityInterface.IsValid());

	const FUniqueNetIdPtr LocalPlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(LocalPlayerId.IsValid());

	// If we don't have a valid session search instance stored, then the party leader has started matchmaking, in which we
	// need to create our own search handle to get notifications from matchmaking to join later
	CurrentMatchmakingSearchHandle = MakeShared<FOnlineSessionSearchAccelByte>();
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;
	CurrentMatchmakingSearchHandle->SearchingPlayerId = LocalPlayerId;
	CurrentMatchmakingSearchHandle->TicketId = MatchmakingStartedNotif.Ticket_id;

	// #TODO (Maxwell) Revisit this to somehow give developers a way to choose what session name they are matching for if a party
	// member receives a matchmaking notification. Since matchmaking shouldn't just be limited to game sessions.
	CurrentMatchmakingSearchHandle->SearchingSessionName = NAME_GameSession;

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnMatchmakingMatchFoundNotification(FAccelByteModelsV2MatchFoundNotif MatchFoundEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *MatchFoundEvent.Id);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	ensure(PlayerId.IsValid());

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(MatchFoundEvent.Id);
	ensure(SessionUniqueId.IsValid());

	const FOnSingleSessionResultCompleteDelegate OnFindMatchmakingGameSessionByIdCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindMatchmakingGameSessionByIdComplete);
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystem, PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindMatchmakingGameSessionByIdCompleteDelegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnMatchmakingExpiredNotification(FAccelByteModelsV2MatchmakingExpiredNotif MatchmakingExpiredNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Ticket id %s"), *MatchmakingExpiredNotif.TicketId);

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("no matchmaking search handle found"));
		return;
	}
	
	if(CurrentMatchmakingSearchHandle->TicketId != MatchmakingExpiredNotif.TicketId)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Ticket Id did not match with current matchmaking process"));
		return;
	}

	FName SessionName = CurrentMatchmakingSearchHandle->SearchingSessionName;
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
	CurrentMatchmakingSearchHandle.Reset();
	CurrentMatchmakingSessionSettings = {};

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Matchmaking ticket expired!"));
	TriggerOnMatchmakingCompleteDelegates(SessionName, false);
}

void FOnlineSessionV2AccelByte::OnFindMatchmakingGameSessionByIdComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!bWasSuccessful)
	{
		FName SessionName = CurrentMatchmakingSearchHandle->SearchingSessionName;
		CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
		CurrentMatchmakingSearchHandle.Reset();
		CurrentMatchmakingSessionSettings = {};
		
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session information for match found from matchmaking! Considering matchmaking failed from here!"));
		TriggerOnMatchmakingCompleteDelegates(SessionName, false);

		return;
	}

	if (!ensure(CurrentMatchmakingSearchHandle.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finish matchmaking as we do not have a valid search handle instance!"));
		
		// #TODO Since we don't have a way to get the session name we were matching for, we just have to default to game
		// session. Figure out a way to make this more flexible.
		TriggerOnMatchmakingCompleteDelegates(NAME_GameSession, false);

		return;
	}

	CurrentMatchmakingSearchHandle->SearchResults.Emplace(Result);
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Done;

	TriggerOnMatchmakingCompleteDelegates(CurrentMatchmakingSearchHandle->SearchingSessionName, true);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGetServerAssociatedSessionComplete_Internal(FName SessionName)
{
	// Flag that we are done getting session associated with this server
	bIsGettingServerAssociatedSession = false;

	// Execute any calls that were waiting on the session to become available
	for (const TFunction<void()>& SessionCall : SessionCallsAwaitingServerSession)
	{
		SessionCall();
	}
	SessionCallsAwaitingServerSession.Empty();
}

void FOnlineSessionV2AccelByte::SetupAccelByteP2PConnection(const FUniqueNetId& LocalPlayerId)
{
	// Start by registering the AccelByte P2P socket subsystem as the default
	FAccelByteNetworkUtilitiesModule::Get().RegisterDefaultSocketSubsystem();

	// Now, grab the API client for the user that is setting up P2P and run set up in the module
	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	ensure(IdentityInterface.IsValid());

	const AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalPlayerId);
	ensure(ApiClient.IsValid());

	FAccelByteNetworkUtilitiesModule::Get().Setup(ApiClient);
}

void FOnlineSessionV2AccelByte::TeardownAccelByteP2PConnection()
{
	FAccelByteNetworkUtilitiesModule::Get().UnregisterDefaultSocketSubsystem();
}

void FOnlineSessionV2AccelByte::OnICEConnectionComplete(const FString& PeerId, bool bStatus, FName SessionName)
{
	TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::Success);
}

#undef ONLINE_ERROR_NAMESPACE
