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
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteUpdateMemberStatus.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteInitializePlayerAttributes.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteUpdatePlayerAttributes.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteCreateV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteLeaveV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteJoinV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteSendV2PartyInvite.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteUpdatePartyV2.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteKickV2Party.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelBytePromoteV2PartyLeader.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteRejectV2PartyInvite.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteRefreshV2PartySession.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteFindV2PartyById.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteJoinV2PartyByCode.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteGenerateNewV2PartyCode.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteRevokeV2PartyCode.h"
#include "AsyncTasks/Matchmaking/OnlineAsyncTaskAccelByteStartV2Matchmaking.h"
#include "AsyncTasks/Matchmaking/OnlineAsyncTaskAccelByteCancelV2Matchmaking.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteGetServerClaimedV2Session.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteRegisterRemoteServerV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteRegisterLocalServerV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteUnregisterRemoteServerV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteUnregisterLocalServerV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteAcceptBackfillProposal.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteRejectBackfillProposal.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteCreateBackfillTicket.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteDeleteBackfillTicket.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteServerQueryGameSessionsV2.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteServerQueryPartySessionsV2.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineVoiceInterfaceAccelByte.h"
#include "Api/AccelByteLobbyApi.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "AccelByteNetworkUtilities.h"
#include "OnlineSessionSettingsAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteUtilities.h"
#include <algorithm>

#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelBytePromoteV2GameSessionLeader.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineSessionV2AccelByte"
#define ACCELBYTE_P2P_TRAVEL_URL_FORMAT TEXT("accelbyte.%s:%d")
const FString ClientIdPrefix = FString(TEXT("client-"));

FOnlineSessionInfoAccelByteV2::FOnlineSessionInfoAccelByteV2(const FString& SessionIdStr)
	: SessionId(FUniqueNetIdAccelByteResource::Create(SessionIdStr))
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
	// 
	// #NOTE Deliberately not using either of these bool return values as we only call this method during create, join, or
	// refresh. Since these are just used to fire notification delegates, we won't need those for these manual actions.
	bool bWasJoinedMembersChanged = false;
	bool bWasInvitedPlayersChanged = false;
	UpdatePlayerLists(bWasJoinedMembersChanged, bWasInvitedPlayersChanged);

	UpdateLeaderId();

	// Finally, try and update connection information for game sessions
	UpdateConnectionInfo();
}

void FOnlineSessionInfoAccelByteV2::SetLatestBackendSessionDataUpdate(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBackendSessionData)
{
	LatestBackendSessionDataUpdate = InBackendSessionData;
}

void FOnlineSessionInfoAccelByteV2::SetBackendSessionData(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBackendSessionData, bool& bWasInvitedPlayersChanged)
{
	BackendSessionData = InBackendSessionData;

	// We don't care about whether joined members changed in this case, so this out param is unused
	bool bWasJoinedMembersChanged = false;
	UpdatePlayerLists(bWasJoinedMembersChanged, bWasInvitedPlayersChanged);

	UpdateLeaderId();
	UpdateConnectionInfo();
}

TSharedPtr<FAccelByteModelsV2BaseSession> FOnlineSessionInfoAccelByteV2::GetBackendSessionData() const
{
	return BackendSessionData;
}

TSharedPtr<FAccelByteModelsV2GameSession> FOnlineSessionInfoAccelByteV2::GetBackendSessionDataAsGameSession() const
{
	return StaticCastAsGameSession(BackendSessionData);
}

TSharedPtr<FAccelByteModelsV2PartySession> FOnlineSessionInfoAccelByteV2::GetBackendSessionDataAsPartySession() const
{
	return StaticCastAsPartySession(BackendSessionData);
}

TSharedPtr<FAccelByteModelsV2BaseSession> FOnlineSessionInfoAccelByteV2::GetLatestBackendSessionDataUpdate() const
{
	return LatestBackendSessionDataUpdate;
}

TSharedPtr<FAccelByteModelsV2GameSession> FOnlineSessionInfoAccelByteV2::GetLatestBackendSessionDataUpdateAsGameSession() const
{
	return StaticCastAsGameSession(LatestBackendSessionDataUpdate);
}

void FOnlineSessionInfoAccelByteV2::SetDSReadyUpdateReceived(const bool& bInIsDSReady)
{
	bDSReadyUpdateReceived = bInIsDSReady;
}

bool FOnlineSessionInfoAccelByteV2::GetDSReadyUpdateReceived() const
{
	return bDSReadyUpdateReceived;
}

bool FOnlineSessionInfoAccelByteV2::FindMember(const FUniqueNetId& MemberId, FAccelByteModelsV2SessionUser*& OutMember)
{
	// Check backend session data validity before trying to find anything inside of that data
	if (!BackendSessionData.IsValid())
	{
		return false;
	}

	// Convert member ID to an AccelByte composite ID structure
	FUniqueNetIdAccelByteUserPtr MemberCompositeId = FUniqueNetIdAccelByteUser::CastChecked(MemberId);
	
	// Iterate through each member in the session data, try and find a match by AccelByte ID
	for (FAccelByteModelsV2SessionUser& Member : BackendSessionData->Members)
	{
		if (Member.ID == MemberCompositeId->GetAccelByteId())
		{
			// Match found - return early with the match
			OutMember = &Member;
			return true;
		}
	}

	// Failed to find a match - bail
	return false;
}

bool FOnlineSessionInfoAccelByteV2::ContainsMember(const FUniqueNetId& MemberId)
{
	FAccelByteModelsV2SessionUser* TempFoundMember = nullptr;
	return FindMember(MemberId, TempFoundMember);
}

void FOnlineSessionInfoAccelByteV2::SetP2PChannel(int32 InChannel)
{
	P2PChannel = InChannel;
}

bool FOnlineSessionInfoAccelByteV2::IsP2PMatchmaking()
{
	// currently if the creator is client IAM, then this is a MM match
	return GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P && GetBackendSessionDataAsGameSession()->CreatedBy.StartsWith(TEXT("client"));
}

TArray<FAccelByteModelsV2GameSessionTeam> FOnlineSessionInfoAccelByteV2::GetTeamAssignments() const
{
	return Teams;
}

void FOnlineSessionInfoAccelByteV2::SetTeamAssignments(const TArray<FAccelByteModelsV2GameSessionTeam>& InTeams)
{
	Teams = InTeams;

	// Reset member-party mapping
	MemberParties.Empty();
	
	for(const auto& Team : Teams)
	{
		for(const auto& Party : Team.Parties)
		{
			// no need to store if partyId is empty
			if(Party.PartyID.IsEmpty())
			{
				continue;
			}

			for(const auto& Member : Party.UserIDs)
			{
				MemberParties.Emplace(Member, Party.PartyID);
			}
		}
	}
}

bool FOnlineSessionInfoAccelByteV2::HasConnectionInfo() const
{
	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastAsGameSession(BackendSessionData);
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
	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastAsGameSession(BackendSessionData);
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
		return FString::Printf(ACCELBYTE_P2P_TRAVEL_URL_FORMAT, *PeerId, P2PChannel);
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
	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionBackendData = StaticCastAsGameSession(BackendSessionData);
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

TArray<FUniqueNetIdRef> FOnlineSessionInfoAccelByteV2::GetInvitedPlayers() const
{
	return InvitedPlayers;
}

FString FOnlineSessionInfoAccelByteV2::GetMemberPartyId(const FUniqueNetIdRef& UserId) const
{
	auto ABUserId = FUniqueNetIdAccelByteUser::TryCast(UserId);
	if(!ABUserId->IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Could not check member's Party ID since UserId is invalid!"));
		return "";
	}

	if(!MemberParties.Contains(ABUserId->GetAccelByteId()))
	{
		return "";
	}

	return MemberParties[ABUserId->GetAccelByteId()];
}

void FOnlineSessionInfoAccelByteV2::UpdatePlayerLists(bool& bOutJoinedMembersChanged, bool& bOutInvitedPlayersChanged)
{
	if (!BackendSessionData.IsValid())
	{
		return;
	}

	// Save previous lengths of invited and joined player arrays to determine if either have changed
	const int32 PreviousInvitedPlayersNum = InvitedPlayers.Num();
	const int32 PreviousJoinedMembersNum = JoinedMembers.Num();

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

	if (PreviousInvitedPlayersNum != InvitedPlayers.Num())
	{
		bOutInvitedPlayersChanged = true;
	}

	if (PreviousJoinedMembersNum != JoinedMembers.Num())
	{
		bOutJoinedMembersChanged = true;
	}
}

void FOnlineSessionInfoAccelByteV2::UpdateLeaderId()
{
	if (!BackendSessionData.IsValid())
	{
		return;
	}

	FAccelByteModelsV2SessionUser* FoundLeaderMember = BackendSessionData->Members.FindByPredicate([this](const FAccelByteModelsV2SessionUser& Member) {
		return Member.ID.Equals(BackendSessionData->LeaderID);
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
	TSharedPtr<FAccelByteModelsV2GameSession> GameBackendSessionData = StaticCastAsGameSession(BackendSessionData);
	if (!GameBackendSessionData.IsValid())
	{
		return;
	}

	const bool bIsDsJoinable = GameBackendSessionData->DSInformation.Server.Status.Equals(TEXT("READY"), ESearchCase::IgnoreCase) || GameBackendSessionData->DSInformation.Server.Status.Equals(TEXT("BUSY"), ESearchCase::IgnoreCase);
	if (GameBackendSessionData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::DS && bIsDsJoinable)
	{
		// Grab socket subsystem to create a new FInternetAddr instance
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (ensure(SocketSubsystem != nullptr))
		{
			HostAddress = SocketSubsystem->CreateInternetAddr();

			// Set the IP of the address to be the string we get from backend, error out if the string is invalid
			bool bIsIpValid = true;
			HostAddress->SetIp(*GameBackendSessionData->DSInformation.Server.Ip, bIsIpValid);
			HostAddress->SetPort(GameBackendSessionData->DSInformation.Server.Port);
			ensure(bIsIpValid);
		}
	}
	else if (GameBackendSessionData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::P2P)
	{
		// If this is a P2P session, then we don't want to set the HostAddress, but rather set the remote ID to the user ID
		// of the player that created the session.
		//
		// #TODO This doesn't account for if a player drops and we need a new ID for them, may need to figure something out for this...
		if(IsP2PMatchmaking())
		{
			PeerId = GameBackendSessionData->LeaderID;
		}
		else
		{
			PeerId = GameBackendSessionData->CreatedBy;
		}		
	}
}

TSharedPtr<FAccelByteModelsV2GameSession> FOnlineSessionInfoAccelByteV2::StaticCastAsGameSession(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBaseSession) const
{
	if (InBaseSession->SessionType != EAccelByteV2SessionType::GameSession)
	{
		return nullptr;
	}
	return StaticCastSharedPtr<FAccelByteModelsV2GameSession>(InBaseSession);
}

TSharedPtr<FAccelByteModelsV2PartySession> FOnlineSessionInfoAccelByteV2::StaticCastAsPartySession(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBaseSession) const
{
	if (InBaseSession->SessionType != EAccelByteV2SessionType::PartySession)
	{
		return nullptr;
	}
	return StaticCastSharedPtr<FAccelByteModelsV2PartySession>(InBaseSession);
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

const FString FOnlineSessionV2AccelByte::ServerSessionIdEnvironmentVariable = TEXT("NOMAD_META_session_id");

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

void FOnlineSessionV2AccelByte::Init()
{
	if (IsRunningDedicatedServer())
	{
		const FOnServerReceivedSessionDelegate OnServerReceivedSessionDelegate = FOnServerReceivedSessionDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnServerReceivedSessionComplete_Internal);
		AddOnServerReceivedSessionDelegate_Handle(OnServerReceivedSessionDelegate);
	}
}

void FOnlineSessionV2AccelByte::Tick(float DeltaTime)
{
	// If this flag has not been set, we don't need to do anything
	if (!bReceivedSessionUpdate)
	{
		return;
	}

	FScopeLock ScopeLock(&SessionLock);

	// Check for updates and apply them to each stored session
	for (TPair<FName, TSharedPtr<FNamedOnlineSession>>& SessionEntry : Sessions)
	{
		if (!ensure(SessionEntry.Value.IsValid()))
		{
			UE_LOG_AB(Warning, TEXT("Could not check session for updates as the session is invalid!"));
			continue;
		}

		if (SessionEntry.Value->SessionState == EOnlineSessionState::Creating || SessionEntry.Value->SessionState == EOnlineSessionState::Destroying)
		{
			// Do not attempt to update a session that is creating or destroying, as those states will not have valid session
			// info or backend data
			UE_LOG_AB(VeryVerbose, TEXT("Ignoring updating session named %s as it is still in the %s state."), *SessionEntry.Key.ToString(), EOnlineSessionState::ToString(SessionEntry.Value->SessionState));
			continue;
		}

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionEntry.Value->SessionInfo);
		if (!ensure(SessionInfo.IsValid()))
		{
			UE_LOG_AB(Warning, TEXT("Could not check session for updates as the session doesn't have a valid session info object!"));
			continue;
		}

		const TSharedPtr<FAccelByteModelsV2BaseSession> ExistingBackendData = SessionInfo->GetBackendSessionData();
		if (!ensure(ExistingBackendData.IsValid()))
		{
			UE_LOG_AB(Warning, TEXT("Could not check session for updates as the session info doesn't have a valid backend session data object!"));
			continue;
		}

		// If there is no latest update, or the latest update's version is not greater than the current data's version, we skip this session
		const TSharedPtr<FAccelByteModelsV2BaseSession> LatestUpdate = SessionInfo->GetLatestBackendSessionDataUpdate();
		const bool bHasUpdateToApply = LatestUpdate.IsValid() && LatestUpdate->Version > ExistingBackendData->Version;
		if (!bHasUpdateToApply)
		{
			SessionInfo->SetLatestBackendSessionDataUpdate(nullptr);
			continue;
		}

		bool bIsConnectingToP2P = false;

		const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(SessionEntry.Value->SessionSettings);
		if (SessionType == EAccelByteV2SessionType::PartySession)
		{
			const TSharedPtr<FAccelByteModelsV2PartySession> PartySessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(LatestUpdate);
			if (!ensure(PartySessionData.IsValid()))
			{
				UE_LOG_AB(Warning, TEXT("Could not update session as the new session data is invalid!"));
				continue;
			}

			UpdateInternalPartySession(SessionEntry.Key, PartySessionData.ToSharedRef().Get());
		}
		else if (SessionType == EAccelByteV2SessionType::GameSession)
		{
			const TSharedPtr<FAccelByteModelsV2GameSession> GameSessionData = SessionInfo->GetLatestBackendSessionDataUpdateAsGameSession();
			if (!ensure(GameSessionData.IsValid()))
			{
				UE_LOG_AB(Warning, TEXT("Could not update session as the new session data is invalid!"));
				continue;
			}

			UpdateInternalGameSession(SessionEntry.Key, GameSessionData.ToSharedRef().Get(), bIsConnectingToP2P);
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Could not update session as the session's type is neither Game nor Party!"));
			continue;
		}

		if (SessionInfo->GetDSReadyUpdateReceived())
		{
			SessionInfo->SetDSReadyUpdateReceived(false);
			TriggerOnSessionServerUpdateDelegates(SessionEntry.Value->SessionName);
		}

		SessionInfo->SetLatestBackendSessionDataUpdate(nullptr);

		// If the client is connecting to P2P, the connecting finished delegate will fire the session update complete delegate
		if (!bIsConnectingToP2P)
		{
			TriggerOnUpdateSessionCompleteDelegates(SessionEntry.Value->SessionName, true);
			TriggerOnSessionUpdateReceivedDelegates(SessionEntry.Value->SessionName);
		}
	}

	bReceivedSessionUpdate = false;
}

void FOnlineSessionV2AccelByte::RegisterSessionNotificationDelegates(const FUniqueNetId& PlayerId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s"), *PlayerId.ToDebugString());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as our identity interface is invalid!"));
		return;
	}

	int32 LocalUserNum = 0;
	if (!ensure(IdentityInterface->GetLocalUserNum(PlayerId, LocalUserNum)))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as we could not get a local user index for player with ID '%s'!"), *PlayerId.ToDebugString());
		return;
	}

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(PlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as player '%s' has an invalid API client!"), *PlayerId.ToDebugString());
		return;
	}

	#define BIND_LOBBY_NOTIFICATION(NotificationDelegateName, Verb) \
		typedef AccelByte::Api::Lobby::FV2##NotificationDelegateName##Notif F##Verb##NotificationDelegate; \
		const F##Verb##NotificationDelegate On##Verb##NotificationDelegate = F##Verb##NotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::On##Verb##Notification, LocalUserNum); \
		ApiClient->Lobby.SetV2##NotificationDelegateName##NotifDelegate(On##Verb##NotificationDelegate); \

	// Begin Game Session Notifications
	BIND_LOBBY_NOTIFICATION(GameSessionInvited, InvitedToGameSession);
	BIND_LOBBY_NOTIFICATION(GameSessionMembersChanged, GameSessionMembersChanged);
	BIND_LOBBY_NOTIFICATION(GameSessionUpdated, GameSessionUpdated);
	BIND_LOBBY_NOTIFICATION(GameSessionKicked, KickedFromGameSession);
	BIND_LOBBY_NOTIFICATION(DSStatusChanged, DsStatusChanged);
	//~ End Game Session Notifications

	// Begin Party Session Notifications
	BIND_LOBBY_NOTIFICATION(PartyInvited, InvitedToPartySession);
	BIND_LOBBY_NOTIFICATION(PartyMembersChanged, PartySessionMembersChanged);
	BIND_LOBBY_NOTIFICATION(PartyUpdated, PartySessionUpdated);
	BIND_LOBBY_NOTIFICATION(PartyKicked, KickedFromPartySession);
	BIND_LOBBY_NOTIFICATION(PartyRejected, PartySessionInviteRejected);
	//~ End Party Session Notifications

	// Begin Matchmaking Notifications
	BIND_LOBBY_NOTIFICATION(MatchmakingStart, MatchmakingStarted);
	BIND_LOBBY_NOTIFICATION(MatchmakingMatchFound, MatchmakingMatchFound);
	BIND_LOBBY_NOTIFICATION(MatchmakingExpired, MatchmakingExpired);
	BIND_LOBBY_NOTIFICATION(MatchmakingCanceled, MatchmakingCanceled)
	//~ End Matchmaking Notifications

	#undef BIND_LOBBY_NOTIFICATION

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

EAccelByteV2SessionJoinability FOnlineSessionV2AccelByte::GetJoinabilityFromString(const FString& JoinabilityStr) const
{
	UEnum* JoinabilityEnum = StaticEnum<EAccelByteV2SessionJoinability>();
	if (!ensure(JoinabilityEnum != nullptr))
	{
		return EAccelByteV2SessionJoinability::EMPTY;
	}

	int64 FoundJoinTypeEnumValue = JoinabilityEnum->GetValueByNameString(JoinabilityStr);
	if (FoundJoinTypeEnumValue != INDEX_NONE)
	{
		return static_cast<EAccelByteV2SessionJoinability>(FoundJoinTypeEnumValue);
	}

	// By default, return joinabilty as empty in case the creator didn't specify a join type
	return EAccelByteV2SessionJoinability::EMPTY;
}

EAccelByteV2SessionType FOnlineSessionV2AccelByte::GetSessionTypeFromString(const FString& SessionTypeStr) const
{
	UEnum* SessionTypeEnum = StaticEnum<EAccelByteV2SessionType>();
	if (!ensure(SessionTypeEnum != nullptr))
	{
		return EAccelByteV2SessionType::Unknown;
	}

	int64 FoundSessionTypeEnumValue = SessionTypeEnum->GetValueByNameString(SessionTypeStr);
	if (FoundSessionTypeEnumValue != INDEX_NONE)
	{
		return static_cast<EAccelByteV2SessionType>(FoundSessionTypeEnumValue);
	}

	// By default, return session type as unknown if not found
	return EAccelByteV2SessionType::Unknown;
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
	if (!ensure(JoinabilityEnum != nullptr))
	{
		return TEXT("");
	}

	return JoinabilityEnum->GetAuthoredNameStringByValue(static_cast<int64>(Joinability)).ToUpper();
}

FString FOnlineSessionV2AccelByte::GetServerTypeAsString(const EAccelByteV2SessionConfigurationServerType& ServerType) const
{
	UEnum* ServerTypeEnum = StaticEnum<EAccelByteV2SessionConfigurationServerType>();
	if (!ensure(ServerTypeEnum != nullptr))
	{
		return TEXT("");
	}

	return ServerTypeEnum->GetAuthoredNameStringByValue(static_cast<int64>(ServerType)).ToUpper();
}

EAccelByteV2SessionConfigurationServerType FOnlineSessionV2AccelByte::GetServerTypeFromString(const FString& ServerType) const
{
	UEnum* ServerTypeEnum = StaticEnum<EAccelByteV2SessionConfigurationServerType>();
	if (!ensure(ServerTypeEnum != nullptr))
	{
		return EAccelByteV2SessionConfigurationServerType::EMPTY;
	}

	int64 FoundServerTypeEnumValue = ServerTypeEnum->GetValueByNameString(ServerType);
	if (FoundServerTypeEnumValue != INDEX_NONE)
	{
		return static_cast<EAccelByteV2SessionConfigurationServerType>(FoundServerTypeEnumValue);
	}

	// By default, return server type as empty if not found
	return EAccelByteV2SessionConfigurationServerType::EMPTY;
}

TSharedPtr<const FUniqueNetId> FOnlineSessionV2AccelByte::CreateSessionIdFromString(const FString& SessionIdStr)
{
	return FUniqueNetIdAccelByteResource::Create(SessionIdStr);
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

	if (IsRunningDedicatedServer())
	{
		// If we are trying to create a new session as a server, just pass an invalid user ID and continue. Server clients
		// do not have user IDs.
		AB_OSS_INTERFACE_TRACE_END(TEXT("Passing to create session!"));
		return CreateSession(FUniqueNetIdAccelByteUser::Invalid().Get(), SessionName, NewSessionSettings);
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session as our identity interface is invalid!"));
		return false;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(HostingPlayerNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session as we could not find a valid player ID for index %d!"), HostingPlayerNum);
		return false;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Passing to create session with player '%s'!"), *PlayerId->ToDebugString());
	return CreateSession(PlayerId.ToSharedRef().Get(), SessionName, NewSessionSettings);
}

bool FOnlineSessionV2AccelByte::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToString(), *SessionName.ToString());
	}
	else
	{
		AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	}

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

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(NewSessionSettings);
	if (SessionType == EAccelByteV2SessionType::Unknown)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create new session as the SETTING_SESSION_TYPE was blank or set to an unsupported value! Needs to be either SETTING_SESSION_TYPE_PARTY_SESSION or SETTING_SESSION_TYPE_GAME_SESSION!"));
		return false;
	}

	if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Creating a new party session!"));
		return CreatePartySession(HostingPlayerId, SessionName, NewSessionSettings);
	}
	else if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Creating a new game session!"));
		return CreateGameSession(HostingPlayerId, SessionName, NewSessionSettings);
	}

	return false;
}

bool FOnlineSessionV2AccelByte::CreatePartySession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToDebugString(), *SessionName.ToString());
	}
	else
	{
		AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	}

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
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create party session as our identity interface is invalid!"));
		return false;
	}

	// Only set local and remote owner information if we are not running a dedicated server. Servers do not have a user
	// associated with them, so we have no owning user ID, local or otherwise.
	if (!IsRunningDedicatedServer())
	{
		NewSession->OwningUserId = HostingPlayerId.AsShared();
		NewSession->OwningUserName = IdentityInterface->GetPlayerNickname(HostingPlayerId);
		NewSession->LocalOwnerId = HostingPlayerId.AsShared();
	}
	
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
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToDebugString(), *SessionName.ToString());
	}
	else
	{
		AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	}

	// Create new session and update state
	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, NewSessionSettings);
	NewSession->SessionState = EOnlineSessionState::Creating;

	// Set number of open connections to the requested connection count from settings
	NewSession->NumOpenPublicConnections = NewSessionSettings.NumPublicConnections;
	NewSession->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
	
	// If we are sending a create request for this session, then that means that we know who is hosting already.
	// For servers, we want to disable this functionality even if we are sending a create request. This is because a server
	// client does not have a user ID, and thus cannot be set as the local owner or remote owner.
	if (bSendCreateRequest && !IsRunningDedicatedServer())
	{
		const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
		if (!ensure(IdentityInterface.IsValid()))
		{
			AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game session as our identity interface is invalid!"));
			return false;
		}

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
	if (!ensure(NewSession != nullptr))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating game session as we do not have a session created for name '%s'!"), *SessionName.ToString());
		return;
	}

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
		NewSession->NumOpenPublicConnections = NewSession->SessionSettings.NumPublicConnections;

		NewSession->SessionSettings.NumPrivateConnections = 0;
		NewSession->NumOpenPrivateConnections = 0;
	}

	// Register ourselves to the session and attempt to set up P2P connection if we are not running a dedicated server
	if (!IsRunningDedicatedServer())
	{
		if (!ensure(NewSession->LocalOwnerId.IsValid()))
		{
			AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating game session as the local owner of the session is invalid!"));
			DestroySession(SessionName);
			return;
		}

		const FUniqueNetIdRef LocalOwnerIdRef = NewSession->LocalOwnerId.ToSharedRef();
		RegisterPlayer(SessionName, LocalOwnerIdRef.Get(), false);
		if (SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P)
		{
			SetupAccelByteP2PConnection(LocalOwnerIdRef.Get());
		}
	}

	NewSession->SessionState = EOnlineSessionState::Pending;

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::FinalizeCreatePartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& BackendSessionInfo)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *BackendSessionInfo.ID);

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!ensure(Session != nullptr))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating party session as we do not have a session created for name '%s'!"), *SessionName.ToString());
		return;
	}

	// Create new session info based off of the created session, set by filling session ID
	TSharedRef<FOnlineSessionInfoAccelByteV2> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV2>(BackendSessionInfo.ID);
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(BackendSessionInfo));
	Session->SessionInfo = SessionInfo;

	// Parties are always invite only, so we just want to update the private connection num
	Session->SessionSettings.NumPrivateConnections = BackendSessionInfo.Configuration.MaxPlayers;
	Session->NumOpenPrivateConnections = Session->SessionSettings.NumPrivateConnections;
	Session->SessionSettings.Set(SETTING_PARTYSESSION_CODE, BackendSessionInfo.Code);

	// Make sure to also register ourselves to the session
	if (!ensure(Session->LocalOwnerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating party session as the local owner of the session is invalid!"));
		DestroySession(SessionName);
		return;
	}

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
	AddBuiltInGameSessionSettingsToSessionSettings(OutResult.SessionSettings, BackendSession);

	if (!BackendSession.CreatedBy.IsEmpty())
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = BackendSession.CreatedBy;

		TSharedPtr<const FUniqueNetIdAccelByteUser> OwnerId = FUniqueNetIdAccelByteUser::Create(CompositeId);
		if (ensure(OwnerId.IsValid()))
		{
			OutResult.OwningUserId = OwnerId;
		}
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
	AddBuiltInPartySessionSettingsToSessionSettings(OutResult.SessionSettings, BackendSession);

	if (!BackendSession.CreatedBy.IsEmpty())
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = BackendSession.CreatedBy;

		TSharedPtr<const FUniqueNetIdAccelByteUser> OwnerId = FUniqueNetIdAccelByteUser::Create(CompositeId);
		if (ensure(OwnerId.IsValid()))
		{
			OutResult.OwningUserId = OwnerId;
		}
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

void FOnlineSessionV2AccelByte::AddBuiltInGameSessionSettingsToSessionSettings(FOnlineSessionSettings& OutSettings, const FAccelByteModelsV2GameSession& GameSession)
{
	OutSettings.Set(SETTING_SESSION_TYPE, SETTING_SESSION_TYPE_GAME_SESSION);
	OutSettings.Set(SETTING_SESSION_MATCHPOOL, GameSession.MatchPool);
	OutSettings.Set(SETTING_SESSION_SERVER_TYPE, GetServerTypeAsString(GameSession.Configuration.Type));
	OutSettings.Set(SETTING_SESSION_JOIN_TYPE, GetJoinabilityAsString(GameSession.Configuration.Joinability));
	OutSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, GameSession.Configuration.MinPlayers);
	OutSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, GameSession.Configuration.InviteTimeout);
	OutSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, GameSession.Configuration.InactiveTimeout);
	OutSettings.Set(SETTING_GAMESESSION_CLIENTVERSION, GameSession.Configuration.ClientVersion);
	OutSettings.Set(SETTING_GAMESESSION_DEPLOYMENT, GameSession.Configuration.Deployment);

	// If we have an ID for an active backfill ticket set on this session, then set it here
	if (!GameSession.BackfillTicketID.IsEmpty())
	{
		OutSettings.Set(SETTING_MATCHMAKING_BACKFILL_TICKET_ID, GameSession.BackfillTicketID);
	}

	FOnlineSessionSettingsAccelByte::Set(OutSettings, SETTING_GAMESESSION_REQUESTEDREGIONS, GameSession.Configuration.RequestedRegions);
}

void FOnlineSessionV2AccelByte::AddBuiltInPartySessionSettingsToSessionSettings(FOnlineSessionSettings& OutSettings, const FAccelByteModelsV2PartySession& PartySession)
{
	OutSettings.Set(SETTING_SESSION_TYPE, SETTING_SESSION_TYPE_PARTY_SESSION);
	OutSettings.Set(SETTING_SESSION_JOIN_TYPE, GetJoinabilityAsString(PartySession.Configuration.Joinability));
	OutSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, PartySession.Configuration.MinPlayers);
	OutSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, PartySession.Configuration.InactiveTimeout);
	OutSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, PartySession.Configuration.InactiveTimeout);
	OutSettings.Set(SETTING_PARTYSESSION_CODE, PartySession.Code);
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
		FieldName == SETTING_GAMESESSION_SERVERNAME ||
		FieldName == SETTING_SESSION_SERVER_TYPE ||
		FieldName == SETTING_PARTYSESSION_CODE ||
		FieldName == SETTING_MATCHMAKING_BACKFILL_TICKET_ID;
}

bool FOnlineSessionV2AccelByte::GetServerLocalIp(FString& OutIp) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// First, check if we have an override IP set on the interface, if so always use that.
	if (!LocalServerIpOverride.IsEmpty())
	{
		OutIp = LocalServerIpOverride;
		AB_OSS_INTERFACE_TRACE_END(TEXT("OutIp: %s"), *OutIp);
		return true;
	}

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

	// To handle a specified serverIP provided through localds commandline argument
	// i.e. "-serverip=192.168.x.x" 
	// If args not found, then the value won't be modified
	FAccelByteUtilities::GetValueFromCommandLineSwitch(ACCELBYTE_ARGS_SERVERIP, OutIp);

	AB_OSS_INTERFACE_TRACE_END(TEXT("OutIp: %s"), *OutIp);
	return true;
}

bool FOnlineSessionV2AccelByte::GetServerPort(int32& OutPort, bool bIsLocal) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// First, check if we have an override port set on the interface, if so always use that.
	if (bIsLocal && LocalServerPortOverride > 0)
	{
		OutPort = LocalServerPortOverride;
		AB_OSS_INTERFACE_TRACE_END(TEXT("OutPort: %d"), OutPort);
		return true;
	}

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
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
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

	// First, check if we have an override name set on the interface, if so always use that.
	if (!LocalServerNameOverride.IsEmpty())
	{
		OutServerName = LocalServerNameOverride;
		AB_OSS_INTERFACE_TRACE_END(TEXT("OutServerName: %s"), *OutServerName);
		return true;
	}

	// Then, try to find server name as an environment variable
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

bool FOnlineSessionV2AccelByte::GetServerClaimedSession(FName SessionName, const FString& SessionId)
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
	bIsGettingServerClaimedSession = true;

	// Create a new game session instance that will be further filled out by the subsequent async task.
	// #NOTE Intentionally passing an invalid ID as we don't know who is hosting the session currently.
	CreateGameSession(FUniqueNetIdAccelByteUser::Invalid().Get(), SessionName, NewSessionSettings, false);

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetServerClaimedV2Session>(AccelByteSubsystem, SessionName, SessionId);

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

bool FOnlineSessionV2AccelByte::LeaveSession(const FUniqueNetId& LocalUserId, const EAccelByteV2SessionType& SessionType, const FString& SessionId, const FOnLeaveSessionComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionId: %s"), *LocalUserId.ToDebugString(), *SessionId);

	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLeaveV2GameSession>(AccelByteSubsystem, LocalUserId, SessionId, Delegate);
	
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending request to leave game session!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLeaveV2Party>(AccelByteSubsystem, LocalUserId, SessionId, Delegate);
		
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending request to leave party session!"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

TArray<TSharedPtr<FJsonValue>> FOnlineSessionV2AccelByte::ConvertSessionSettingArrayToJson(const TArray<FString>& Array) const
{
	TArray<TSharedPtr<FJsonValue>> OutArray;
	for(const FString& Item : Array)
	{
		OutArray.Add(MakeShared<FJsonValueString>(Item));
	}
	return OutArray;
}

bool FOnlineSessionV2AccelByte::ConvertSessionSettingJsonToArray(const TArray<TSharedPtr<FJsonValue>>* JsonArray, TArray<FString>& OutArray) const
{
	for(const auto& Item : *JsonArray)
	{
		FString ItemString;
		if(!Item->TryGetString(ItemString))
		{
			return false;
		}

		OutArray.Add(ItemString);
	}

	return true;
}

bool FOnlineSessionV2AccelByte::ConvertSessionSettingJsonToArray(const TArray<TSharedPtr<FJsonValue>>* JsonArray, TArray<double>& OutArray) const
{
	for(const auto& Item : *JsonArray)
	{
		double ItemNumber;
		if(!Item->TryGetNumber(ItemNumber))
		{
			return false;
		}

		OutArray.Add(ItemNumber);
	}

	return true;
}

TArray<TSharedPtr<FJsonValue>> FOnlineSessionV2AccelByte::ConvertSessionSettingArrayToJson(const TArray<double>& Array) const
{
	TArray<TSharedPtr<FJsonValue>> OutArray;
	for(const double& Item : Array)
	{
		OutArray.Add(MakeShared<FJsonValueNumber>(Item));
	}
	return OutArray;
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

		// If the setting value is a blob, we assume that it represents a serialized array of strings or doubles
		if(Setting.Value.Data.GetType() == EOnlineKeyValuePairDataType::Blob)
		{
			const auto ArrayType = FOnlineSessionSettingsAccelByte::GetArrayFieldType(Settings, Setting.Key);

			if(ArrayType == ESessionSettingsAccelByteArrayFieldType::STRINGS)
			{
				TArray<FString> Array;
				FOnlineSessionSettingsAccelByte::Get(Settings, Setting.Key, Array);
				OutObject->SetArrayField(Setting.Key.ToString().ToUpper(), ConvertSessionSettingArrayToJson(Array));
			}
			else if(ArrayType == ESessionSettingsAccelByteArrayFieldType::DOUBLES)
			{
				TArray<double> Array;
				FOnlineSessionSettingsAccelByte::Get(Settings, Setting.Key, Array);
				OutObject->SetArrayField(Setting.Key.ToString().ToUpper(), ConvertSessionSettingArrayToJson(Array));
			}

			continue;
		}

		// Add the setting to the attributes object. With the setting key as the field name, 
		// converted to all uppercase to avoid FName weirdness with casing.
		Setting.Value.Data.AddToJsonObject(OutObject, Setting.Key.ToString().ToUpper(), false);
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

		// Check JSON field type to determine type on read
		switch (Attribute.Value->Type)
		{
			case EJson::String:
			{
				FString StringValue;
				if (!Object->TryGetStringField(Attribute.Key, StringValue))
				{
					UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as a string, skipping!"), *Attribute.Key);
					continue;
				}

				OutSettings.Set(FName(Attribute.Key), StringValue);
				break;
			}
			
			case EJson::Boolean:
			{
				bool BoolValue;
				if (!Object->TryGetBoolField(Attribute.Key, BoolValue))
				{
					UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as a bool, skipping!"), *Attribute.Key);
					continue;
				}

				OutSettings.Set(FName(Attribute.Key), BoolValue);
				break;
			}

			case EJson::Number: 
			{
				double NumberValue;
				if (!Object->TryGetNumberField(Attribute.Key, NumberValue))
				{
					UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as a number, skipping!"), *Attribute.Key);
					continue;
				}

				OutSettings.Set(FName(Attribute.Key), NumberValue);
				break;
			}

			case EJson::Array:
			{
				const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
				if(!Object->TryGetArrayField(Attribute.Key, ArrayValue))
				{
					UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as an array, skipping!"), *Attribute.Key);
					continue;
				}

				if (ArrayValue->Num() == 0)
				{
					UE_LOG_AB(Warning, TEXT("Session attribute '%s' is an empty array, skipping!"), *Attribute.Key);
					continue;
				}

				// Using the type of the first field in the array to either continue reading fields as strings or doubles
				const EJson FirstFieldType = (*ArrayValue)[0].ToSharedRef().Get().Type;
				if (FirstFieldType == EJson::String)
				{
					TArray<FString> AttributeValue;
					if(!ConvertSessionSettingJsonToArray(ArrayValue, AttributeValue))
					{
						UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as an array of strings, skipping!"), *Attribute.Key);
						continue;
					}

					FOnlineSessionSettingsAccelByte::Set(OutSettings, FName(Attribute.Key), AttributeValue);
					continue;
				}

				if (FirstFieldType == EJson::Number)
				{
					TArray<double> AttributeValue;
					if (!ConvertSessionSettingJsonToArray(ArrayValue, AttributeValue))
					{
						UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as an array of numbers, skipping!"), *Attribute.Key);
						continue;
					}

					FOnlineSessionSettingsAccelByte::Set(OutSettings, FName(Attribute.Key), AttributeValue);
					continue;
				}

				UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' (an array) as either an array of numbers or an array of strings, skipping!"), *Attribute.Key);

				break;
			}

			case EJson::Object:
			{
				const TSharedPtr<FJsonObject>* JsonObjectValue;
				if (!Object->TryGetObjectField(Attribute.Key, JsonObjectValue))
				{
					UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as an object, skipping!"), *Attribute.Key);
					continue;
				}

				OutSettings.Set(FName(Attribute.Key), (*JsonObjectValue).ToSharedRef());
				break;
			}

			default:
			{
				UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as variant data, skipping!"), *Attribute.Key);
				continue;
			}
		}
	}

	return OutSettings;
}

TSharedRef<FJsonObject> FOnlineSessionV2AccelByte::ConvertSearchParamsToJsonObject(const FOnlineSearchSettings& Params) const
{
	TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
	for (const TPair<FName, FOnlineSessionSearchParam>& Param : Params.SearchParams)
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

		// If the setting value is a blob, we assume that it represents a serialized array of strings or doubles
		if (Param.Value.Data.GetType() == EOnlineKeyValuePairDataType::Blob)
		{
			const ESessionSettingsAccelByteArrayFieldType ArrayType = FOnlineSearchSettingsAccelByte::GetArrayFieldType(Params, Param.Key);
			if (ArrayType == ESessionSettingsAccelByteArrayFieldType::STRINGS)
			{
				TArray<FString> Array;
				FOnlineSearchSettingsAccelByte::Get(Params, Param.Key, Array);
				OutObject->SetArrayField(Param.Key.GetPlainNameString(), ConvertSessionSettingArrayToJson(Array));
			}
			else if (ArrayType == ESessionSettingsAccelByteArrayFieldType::DOUBLES)
			{
				TArray<double> Array;
				FOnlineSearchSettingsAccelByte::Get(Params, Param.Key, Array);
				OutObject->SetArrayField(Param.Key.GetPlainNameString(), ConvertSessionSettingArrayToJson(Array));
			}

			continue;
		}

		// Add the setting to the attributes object. With the setting key as the field name, converted to all uppercase to avoid
		// FName weirdness with casing.
		// Removing type suffix because key need to be same as what is configured in ruleset.
		Param.Value.Data.AddToJsonObject(OutObject, Param.Key.GetPlainNameString(), false);
	}

	return OutObject;
}

void FOnlineSessionV2AccelByte::ConnectToJoinedP2PSession(FName SessionName, EOnlineSessionP2PConnectedAction Action)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!ensure(Session != nullptr))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as our local session instance is invalid!"));
		return;
	}

	// Attempt to register our socket subsystem as default
	FAccelByteNetworkUtilitiesModule::Get().RegisterDefaultSocketSubsystem();

	// Now, grab the API client for the user that is setting up P2P and run set up in the module
	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as our identity interface is invalid!"));
		return;
	}

	if (!ensure(Session->LocalOwnerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as the session's local owner is invalid!"));
		return;
	}

	const AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(Session->LocalOwnerId.ToSharedRef().Get());
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as the session's local owner is invalid!"));
		return;
	}

	FAccelByteNetworkUtilitiesModule::Get().Setup(ApiClient);

	// Register a delegate so that we can fire our join session complete delegate when connection finishes
	const FAccelByteNetworkUtilitiesModule::OnICERequestConnectFinished OnICEConnectionCompleteDelegate =
		FAccelByteNetworkUtilitiesModule::OnICERequestConnectFinished::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnICEConnectionComplete, SessionName, Action);
	FAccelByteNetworkUtilitiesModule::Get().RegisterICERequestConnectFinishedDelegate(OnICEConnectionCompleteDelegate);

	// Request connection to the peer
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as the session's information instance is invalid!"));
		return;
	}

	const int32 P2PChannel = FMath::RandRange(1024, 4096);
	FAccelByteNetworkUtilitiesModule::Get().RequestConnect(FString::Printf (TEXT("%s:%d"), *SessionInfo->GetPeerId(), P2PChannel));

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::UpdateInternalGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& UpdatedGameSession, bool& bIsConnectingToP2P, bool bIsFirstJoin)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' does not exist locally!"), *SessionName.ToString());
		return;
	}

	const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::GameSession)
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

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!SessionData.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally does not have valid session backend data!"), *SessionName.ToString());
		return;
	}

	const TArray<FAccelByteModelsV2SessionUser> OldMembers = SessionData->Members;	
	const EAccelByteV2SessionConfigurationServerType OldServerType = SessionInfo->GetServerType();

	// First update the session data associated with the session info structure
	// TODO: Potentially unnecessary memory allocation
	bool bHasInvitedPlayersChanged = false;
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(UpdatedGameSession), bHasInvitedPlayersChanged);
	SessionInfo->SetTeamAssignments(UpdatedGameSession.Teams);

	UpdateSessionMembers(Session, OldMembers, bHasInvitedPlayersChanged);

	const EAccelByteV2SessionConfigurationServerType NewServerType = SessionInfo->GetServerType();	

	bIsConnectingToP2P = false;
	
	// If we have a local owner ID for this session, then check if we are switching to or from P2P
	if (!IsRunningDedicatedServer() && Session->LocalOwnerId.IsValid())
	{
		const bool bSwitchingToP2P = OldServerType != EAccelByteV2SessionConfigurationServerType::P2P && NewServerType == EAccelByteV2SessionConfigurationServerType::P2P;
		const bool bSwitchingFromP2P = OldServerType == EAccelByteV2SessionConfigurationServerType::P2P && NewServerType != EAccelByteV2SessionConfigurationServerType::P2P;
		const bool bIsFirstP2PJoin = NewServerType == EAccelByteV2SessionConfigurationServerType::P2P && bIsFirstJoin;
		if (bSwitchingToP2P || bIsFirstP2PJoin)
		{
			// If the session's local owner is also its leader, they'll be the P2P host
			if (Session->LocalOwnerId.IsValid() && Session->LocalOwnerId.ToSharedRef().Get() == SessionInfo->GetLeaderId().ToSharedRef().Get())
			{
				SetupAccelByteP2PConnection(Session->LocalOwnerId.ToSharedRef().Get());
			}
			else
			{
				bIsConnectingToP2P = true;
				ConnectToJoinedP2PSession(SessionName, (bIsFirstP2PJoin) ? EOnlineSessionP2PConnectedAction::Join : EOnlineSessionP2PConnectedAction::Update);
			}
		}
		else if (bSwitchingFromP2P)
		{
			TeardownAccelByteP2PConnection();
		}
	}

	// First, read all attributes from the session, as that will overwrite all of the reserved/built-in settings
	if (UpdatedGameSession.Attributes.JsonObject.IsValid())
	{
		Session->SessionSettings = ReadSessionSettingsFromJsonObject(UpdatedGameSession.Attributes.JsonObject.ToSharedRef());
	}

	// After loading in custom attributes, load in reserved/built-in settings
	AddBuiltInGameSessionSettingsToSessionSettings(Session->SessionSettings, UpdatedGameSession);
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

	const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
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

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!SessionData.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally does not have valid session backend data!"), *SessionName.ToString());
		return;
	}

	const TArray<FAccelByteModelsV2SessionUser> OldMembers = SessionData->Members;

	// First update the session data associated with the session info structure
	bool bHasInvitedPlayersChanged = false;
	// TODO: Potentially unnecessary memory allocation
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(UpdatedPartySession), bHasInvitedPlayersChanged);

	UpdateSessionMembers(Session, OldMembers, bHasInvitedPlayersChanged);

	// First, read all attributes from the session, as that will overwrite all of the reserved/built-in settings
	if (UpdatedPartySession.Attributes.JsonObject.IsValid())
	{
		Session->SessionSettings = ReadSessionSettingsFromJsonObject(UpdatedPartySession.Attributes.JsonObject.ToSharedRef());
	}

	// After reading attributes, reload reserved/built-in settings for the session
	AddBuiltInPartySessionSettingsToSessionSettings(Session->SessionSettings, UpdatedPartySession);
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

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystem->GetVoiceInterface());
		if (VoiceInterface.IsValid())
		{
			VoiceInterface->RemoveAllTalkers();
		}
	}
	
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
			SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, false);
		});
		return false;
	}

	Session->SessionSettings = UpdatedSessionSettings;
	
	// If we don't need to refresh our session data on the backend, just bail here
	if (!bShouldRefreshOnlineData)
	{
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
			SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, true);
		});
		AB_OSS_INTERFACE_TRACE_END(TEXT("Skipping updating session settings on backend as bShouldRefreshOnlineData is marked false!"));
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateGameSessionV2>(AccelByteSubsystem, SessionName, UpdatedSessionSettings);
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		if (IsRunningDedicatedServer())
		{
			AccelByteSubsystem->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
				SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
				SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, true);
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
	if (!ensure(SessionInfo.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session as our session info instance is invalid!"));
		return false;
	}

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
		if (!ensure(Session->LocalOwnerId.IsValid()))
		{
			AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session as our session has no local owner ID!"));
			return false;
		}

		const FUniqueNetIdRef SessionOwnerId = Session->LocalOwnerId.ToSharedRef();

		// Fire off call to leave session. Appropriate delegates will be fired by this method, along with clean up of sessions.
		EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);

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

	FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystem->GetVoiceInterface());
	if (VoiceInterface.IsValid())
	{
		VoiceInterface->RemoveAllTalkers();
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
	CurrentMatchmakingSearchHandle->MatchPool = MatchPool;

	// #NOTE Previously, we would set search state to InProgress in the FOnlineAsyncTaskAccelByteStartV2Matchmaking::Initialize
	// method. However, this runs the risk of allowing a second StartMatchmaking call to come in before that method runs
	// and cause issues. Instead, set to InProgress here to prevent duplicate matchmaking start calls.
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;

	SearchSettings = CurrentMatchmakingSearchHandle.ToSharedRef(); // Make sure that the caller also gets the new subclassed search handle
	CurrentMatchmakingSessionSettings = NewSessionSettings;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteStartV2Matchmaking>(AccelByteSubsystem, CurrentMatchmakingSearchHandle.ToSharedRef(), SessionName, MatchPool, CompletionDelegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking as our identity interface is invalid!"));
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
		});
		return false;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking as we could not get a unique ID for player at index %d!"), SearchingPlayerNum);
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
		});
		return false;
	}

	return CancelMatchmaking(PlayerId.ToSharedRef().Get(), SessionName);
}

bool FOnlineSessionV2AccelByte::CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; SessionName: %s"), *SearchingPlayerId.ToString(), *SessionName.ToString());

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("No matchmaking search handle attached to the session interface at this time. Treating the cancel request as a success to allow game client to clean up local state if stuck."));
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, true);
		});
		return true;
	}

	if (CurrentMatchmakingSearchHandle->SearchState != EOnlineAsyncTaskState::InProgress)
	{
		CurrentMatchmakingSearchHandle.Reset();
		AB_OSS_INTERFACE_TRACE_END(TEXT("Matchmaking search handle attached to session interface is not marked as in progress. Treating the cancel request as a success to allow game client to clean up local state if stuck."));
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, true);
		});
		return true;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCancelV2Matchmaking>(AccelByteSubsystem, CurrentMatchmakingSearchHandle.ToSharedRef(), SessionName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find sessions as our identity interface is invalid!"));
		return false;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find sessions as we could not find a unique ID for player at index %d!"), SearchingPlayerNum);
		return false;
	}

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
	if (!ensure(IdentityInterface.IsValid()))
	{
		UE_LOG_AB(Warning, TEXT("Failed to join session as our identity interface is invalid!"));
		return false;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		UE_LOG_AB(Warning, TEXT("Failed to join session as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return false;
	}

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

	// Check whether or not we are just restoring this session. Will impact the call made to the backend on join.
	bool bIsRestoreSession = false;
	for (const FOnlineRestoredSessionAccelByte& Session : RestoredSessions)
	{
		if (Session.Session.GetSessionIdStr().Equals(DesiredSession.GetSessionIdStr()))
		{
			bIsRestoreSession = true;
			break;
		}
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(NewSession->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJoinV2GameSession>(AccelByteSubsystem, LocalUserId, SessionName, bIsRestoreSession);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Spawning async task to join game session on backend!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJoinV2Party>(AccelByteSubsystem, LocalUserId, SessionName, bIsRestoreSession);
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

bool FOnlineSessionV2AccelByte::JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FString& PartyCode)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PartyCode: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PartyCode);

	// First, ensure that we are not going to clobber an existing session with the given name
	if (GetNamedSession(SessionName) != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session with name '%s' as a session with that name already exists!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
		});
		return false;
	}

	// Next, create a dummy session that we will use to occupy the session name while joining by code. Since join by party
	// code doesn't match the flow of a typical join session, we have to fudge it a bit. So the general steps will be:
	// 1. Create dummy session under the given name, and fire off async task to join by code
	// 2. Make the call to join the party by code
	// 3. Store the party data from the join call
	// 4. Convert the party data to a session result, and then call the proper JoinSession method to turn into proper
	//    FNamedOnlineSession.
	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, FOnlineSessionSettings());
	NewSession->SessionState = EOnlineSessionState::Creating;
	NewSession->LocalOwnerId = LocalUserId.AsShared();

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJoinV2PartyByCode>(AccelByteSubsystem, LocalUserId, SessionName, PartyCode);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::GenerateNewPartyCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnGenerateNewPartyCodeComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate new code for party with session name '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false, TEXT(""));
		});
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate new party code for session with name '%s' as the session is not a party session!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false, TEXT(""));
		});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateNewV2PartyCode>(AccelByteSubsystem, LocalUserId, SessionName, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::RevokePartyCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnRevokePartyCodeComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to revoke code for party with session name '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to revoke party code for session with name '%s' as the session is not a party session!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRevokeV2PartyCode>(AccelByteSubsystem, LocalUserId, SessionName, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::SetLocalServerNameOverride(const FString& InLocalServerNameOverride)
{
	LocalServerNameOverride = InLocalServerNameOverride;
}

void FOnlineSessionV2AccelByte::SetLocalServerIpOverride(const FString& InLocalServerIpOverride)
{
	LocalServerIpOverride = InLocalServerIpOverride;
}

void FOnlineSessionV2AccelByte::SetLocalServerPortOverride(int32 InLocalServerPortOverride)
{
	LocalServerPortOverride = InLocalServerPortOverride;
}

FOnlineSessionV2AccelBytePlayerAttributes FOnlineSessionV2AccelByte::GetPlayerAttributes(const FUniqueNetId& LocalPlayerId)
{
	FOnlineSessionV2AccelBytePlayerAttributes OutAttributes{};
	FAccelByteModelsV2PlayerAttributes* FoundAttributes = GetInternalPlayerAttributes(LocalPlayerId);
	if (FoundAttributes != nullptr)
	{
		OutAttributes.bEnableCrossplay = FoundAttributes->CrossplayEnabled;
		OutAttributes.Data = FoundAttributes->Data.JsonObject;
	}

	return OutAttributes;
}

bool FOnlineSessionV2AccelByte::UpdatePlayerAttributes(const FUniqueNetId& LocalPlayerId, const FOnlineSessionV2AccelBytePlayerAttributes& NewAttributes, const FOnUpdatePlayerAttributesComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s"), *LocalPlayerId.ToDebugString());

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdatePlayerAttributes>(AccelByteSubsystem, LocalPlayerId, NewAttributes, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::IsPlayerP2PHost(const FUniqueNetId& LocalUserId, FName SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		return false;
	}

	const bool bIsP2PHost = (LocalUserId == SessionInfo->GetLeaderId().ToSharedRef().Get());
	return SessionInfo->IsP2PMatchmaking() && bIsP2PHost;
}

bool FOnlineSessionV2AccelByte::FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend)
{
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as our identity interface is invalid!"));
		return false;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as we could not find a unique ID for player at index %d!"), LocalUserNum);
		return false;
	}

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
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionName: %s; Friend: %s"), LocalUserNum, *SessionName.ToString(), *Friend.ToDebugString());

	if (IsRunningDedicatedServer())
	{
		// If we are running a dedicated server, then we just want to continue sending the invite with an invalid local user ID.
		// Servers do not have user IDs for the authenticated client, so just fake it with the invalid ID.
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending invite to user '%s'!"), *Friend.ToDebugString());
		return SendSessionInviteToFriend(FUniqueNetIdAccelByteUser::Invalid().Get(), SessionName, Friend);
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as our identity interface is invalid!"));
		return false;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as we could not get a unique ID from player index %d!"), LocalUserNum);
		return false;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Sending invite to user '%s'!"), *Friend.ToDebugString());
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

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendV2GameSessionInvite>(AccelByteSubsystem, LocalUserId, SessionName, Friend);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending invite to player for game session!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
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
	if (!ensure(SessionInfo.IsValid()))
	{
		UE_LOG_AB(Warning, TEXT("Failed to get resolved connect string for session '%s' as the local session info is invalid!"), *SessionName.ToString());
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

			const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
			if (SessionType == EAccelByteV2SessionType::GameSession && !IsRunningDedicatedServer())
			{
				FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystem->GetVoiceInterface());
				if (VoiceInterface.IsValid())
				{
					VoiceInterface->RegisterTalker(PlayerToAdd, *Session);
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
	FScopeLock ScopeLock(&SessionLock);

	for (const TPair<FName, TSharedPtr<FNamedOnlineSession>>& Pair : Sessions)
	{
		if (!ensure(Pair.Value.IsValid()))
		{
			continue;
		}

		FNamedOnlineSession* Session = Pair.Value.Get();

		const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
		if (!ensure(SessionInfo.IsValid()))
		{
			continue;
		}

		DumpNamedSession(Session);

		FString BackendDataJsonString;
		EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);

		if (SessionType == EAccelByteV2SessionType::GameSession)
		{
			FJsonObjectConverter::UStructToJsonObjectString(SessionInfo->GetBackendSessionDataAsGameSession().ToSharedRef().Get(), BackendDataJsonString);
		}
		else if (SessionType == EAccelByteV2SessionType::PartySession)
		{
			FJsonObjectConverter::UStructToJsonObjectString(SessionInfo->GetBackendSessionDataAsPartySession().ToSharedRef().Get(), BackendDataJsonString);
		}

		LOG_SCOPE_VERBOSITY_OVERRIDE(LogAccelByteOSS, ELogVerbosity::VeryVerbose);
		UE_LOG_AB(Verbose, TEXT("dumping session backend data as JSON: %s"), *BackendDataJsonString);
	}
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27)
void FOnlineSessionV2AccelByte::RemovePlayerFromSession(int32 LocalUserNum, FName SessionName, const FUniqueNetId& TargetPlayerId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionName: %s; TargetPlayerId: %s"), LocalUserNum, *SessionName.ToString(), *TargetPlayerId.ToDebugString());

	// Grab a user ID for the local user index provided through the identity interface
	FOnlineIdentityAccelBytePtr IdentityInterface = nullptr;
	if (!FOnlineIdentityAccelByte::GetFromSubsystem(AccelByteSubsystem, IdentityInterface))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove player '%s' from session '%s' as a unique ID for local player %d was not found!"), *TargetPlayerId.ToDebugString(), *SessionName.ToString(), LocalUserNum);
		return;
	}

	FUniqueNetIdPtr LocalPlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalPlayerId.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove player '%s' from session '%s' as a unique ID for local player %d was not found!"), *TargetPlayerId.ToDebugString(), *SessionName.ToString(), LocalUserNum);
		return;
	}

	KickPlayer(LocalPlayerId.ToSharedRef().Get(), SessionName, TargetPlayerId);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}
#endif

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
		return Invite.SessionType == EAccelByteV2SessionType::GameSession;
	});
}

TArray<FOnlineSessionInviteAccelByte> FOnlineSessionV2AccelByte::GetAllPartyInvites() const
{
	return SessionInvites.FilterByPredicate([](const FOnlineSessionInviteAccelByte& Invite) {
		return Invite.SessionType == EAccelByteV2SessionType::PartySession;
	});
}

bool FOnlineSessionV2AccelByte::RejectInvite(const FUniqueNetId& PlayerId, const FOnlineSessionInviteAccelByte& InvitedSession, const FOnRejectSessionInviteComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s; InvitedSessionId: %s"), *PlayerId.ToDebugString(), *InvitedSession.Session.GetSessionIdStr());

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(InvitedSession.Session.Session.SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRejectV2GameSessionInvite>(AccelByteSubsystem, PlayerId, InvitedSession.Session, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sending rejection for game session invite!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
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
		return RestoredSession.SessionType == EAccelByteV2SessionType::PartySession;
	});
}

TArray<FOnlineRestoredSessionAccelByte> FOnlineSessionV2AccelByte::GetAllRestoredGameSessions() const
{
	return RestoredSessions.FilterByPredicate([](const FOnlineRestoredSessionAccelByte& RestoredSession) {
		return RestoredSession.SessionType == EAccelByteV2SessionType::GameSession;
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

	const AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived OnAMSDrainReceivedDelegate = AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnAMSDrain);
	FRegistry::ServerAMS.SetOnAMSDrainReceivedDelegate(OnAMSDrainReceivedDelegate);

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

EAccelByteV2SessionType FOnlineSessionV2AccelByte::GetSessionTypeByName(const FName& SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		return EAccelByteV2SessionType::Unknown;
	}

	return GetSessionTypeFromSettings(Session->SessionSettings);
}

EAccelByteV2SessionType FOnlineSessionV2AccelByte::GetSessionTypeFromSettings(const FOnlineSessionSettings& Settings) const
{
	FString SessionTypeString = TEXT("");
	if (!Settings.Get(SETTING_SESSION_TYPE, SessionTypeString))
	{
		return EAccelByteV2SessionType::Unknown;
	}

	return GetSessionTypeFromString(SessionTypeString);
}

FUniqueNetIdPtr FOnlineSessionV2AccelByte::GetSessionLeaderId(const FNamedOnlineSession* Session) const
{
	const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
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

bool FOnlineSessionV2AccelByte::KickPlayer(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToKick, const FOnKickPlayerComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToKick: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PlayerIdToKick.ToDebugString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot kick player from session as the session does not exist locally!"));
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		// #TODO Revisit - not sure if game sessions will have kick functionality, but for player owned sessions they *probably* should
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteKickV2Party>(AccelByteSubsystem, LocalUserId, SessionName, PlayerIdToKick, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to kick player from party session!"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

bool FOnlineSessionV2AccelByte::PromoteGameSessionLeader(const FUniqueNetId& LocalUserId, const FName& SessionName,
	const FUniqueNetId& PlayerIdToPromote)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToPromoteAsGameSessionLeader: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PlayerIdToPromote.ToDebugString());

	const FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot promote player from game session as the session does not exist locally!"));
		return false;
	}

	const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	switch (SessionType)
	{
		case EAccelByteV2SessionType::GameSession:
			AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelBytePromoteV2GameSessionLeader>(AccelByteSubsystem, LocalUserId, Session->GetSessionIdStr(), PlayerIdToPromote);
			AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to promote player to leader of game session!"));
			return true;

		case EAccelByteV2SessionType::PartySession:
		case EAccelByteV2SessionType::Unknown:
		default:
			AB_OSS_INTERFACE_TRACE_END(TEXT("Unable to promote game session leader of SessionName: %s as the session is of type %s!"), *SessionName.ToString(), *FAccelByteUtilities::GetUEnumValueAsString(SessionType));
			return false;
	}
}

bool FOnlineSessionV2AccelByte::PromotePartySessionLeader(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToPromote, const FOnPromotePartySessionLeaderComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToPromote: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PlayerIdToPromote.ToDebugString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot kick player from session as the session does not exist locally!"));
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Unable to promote party leader of SessionName: %s as the session is of type GameSession!"), *SessionName.ToString());
		return false;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelBytePromoteV2PartyLeader>(AccelByteSubsystem, LocalUserId, SessionName, Session->GetSessionIdStr(), PlayerIdToPromote, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to promote player to leader of party session!"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

TArray<FString> FOnlineSessionV2AccelByte::GetRegionList(const FUniqueNetId& LocalPlayerId) const
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s"), *LocalPlayerId.ToDebugString());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get list of regions as our identity interface is invalid!"));
		return TArray<FString>();
	}

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalPlayerId);
	if (!ensure(ApiClient.IsValid()))
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

bool FOnlineSessionV2AccelByte::RefreshSession(const FName& SessionName, const FOnRefreshSessionComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot refresh session as the session does not exist locally!"));
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshV2GameSession>(AccelByteSubsystem, SessionName, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to refresh local game session with backend data!"));
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		if (IsRunningDedicatedServer())
		{
			AB_OSS_INTERFACE_TRACE_END(TEXT("Game servers are not able to refresh party sessions!"));
			return false;
		}

		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshV2PartySession>(AccelByteSubsystem, SessionName, Delegate);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Sent off request to refresh local party session with backend data!"));
	}

	return true;
}

bool FOnlineSessionV2AccelByte::CreateBackfillTicket(const FName& SessionName, const FOnCreateBackfillTicketComplete& Delegate)
{
	return CreateBackfillTicket(SessionName, TEXT(""), Delegate);
}

bool FOnlineSessionV2AccelByte::CreateBackfillTicket(const FName& SessionName, const FString& MatchPool, const FOnCreateBackfillTicketComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot create backfill ticket for session as the session does not exist locally!"));
		AccelByteSubsystem->ExecuteNextTick([Delegate, SessionName]() {
			Delegate.ExecuteIfBound(false);
			});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateBackfillTicket>(AccelByteSubsystem, SessionName, MatchPool, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::DeleteBackfillTicket(const FName& SessionName, const FOnDeleteBackfillTicketComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot delete backfill ticket for session as the session does not exist locally!"));
		AccelByteSubsystem->ExecuteNextTick([Delegate, SessionName]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteBackfillTicket>(AccelByteSubsystem, SessionName, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::AcceptBackfillProposal(const FName& SessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Proposal, bool bStopBackfilling, const FOnAcceptBackfillProposalComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot accept backfill proposal for session as the session does not exist locally!"));
		AccelByteSubsystem->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteAcceptBackfillProposal>(AccelByteSubsystem, SessionName, Proposal, bStopBackfilling, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::RejectBackfillProposal(const FName& SessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Proposal, bool bStopBackfilling, const FOnRejectBackfillProposalComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot reject backfill proposal for session as the session does not exist locally!"));
		AccelByteSubsystem->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRejectBackfillProposal>(AccelByteSubsystem, SessionName, Proposal, bStopBackfilling, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::UpdateMemberStatus(FName SessionName, const FUniqueNetId& PlayerId, const EAccelByteV2SessionMemberStatus& Status, const FOnSessionMemberStatusUpdateComplete& Delegate)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; PlayerId: %s; Status: %s"), *SessionName.ToString(), *PlayerId.ToDebugString(), *StaticEnum<EAccelByteV2SessionMemberStatus>()->GetNameStringByValue(static_cast<int64>(Status)));

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not member status for session named '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystem->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateMemberStatus>(AccelByteSubsystem, SessionName, PlayerId, Status, Delegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::EnqueueBackendDataUpdate(const FName& SessionName, const TSharedPtr<FAccelByteModelsV2BaseSession>& NewSessionData, const bool bIsDSReadyUpdate/*=false*/)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not enqueue session backend data update as there is no session with the provided name!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not enqueue session backend data update as session does not have a valid session info instance!"));
		return;
	}

	const TSharedPtr<FAccelByteModelsV2BaseSession> ExistingSessionData = SessionInfo->GetBackendSessionData();
	if (!NewSessionData.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not enqueue session backend data update as session does not have a valid existing backend data instance!"));
		return;
	}

	const TSharedPtr<FAccelByteModelsV2BaseSession> LastUpdate = SessionInfo->GetLatestBackendSessionDataUpdate();

	const bool bIsUpdateNewerThanLast = !LastUpdate.IsValid() || NewSessionData->Version > LastUpdate->Version;
	const bool bShouldUpdate = ExistingSessionData->Version < NewSessionData->Version && bIsUpdateNewerThanLast;

	if (bShouldUpdate)
	{
		SessionInfo->SetLatestBackendSessionDataUpdate(NewSessionData);
	}

	if (bIsDSReadyUpdate)
	{
		SessionInfo->SetDSReadyUpdateReceived(true);
	}

	if (!bReceivedSessionUpdate)
	{
		bReceivedSessionUpdate = bShouldUpdate || bIsDSReadyUpdate;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnInvitedToGameSessionNotification(FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *InviteEvent.SessionID);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InviteEvent.SessionID);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as we could not create a valid session unique ID from the ID in the notification!"));
		return;
	}

	const FOnSingleSessionResultCompleteDelegate OnFindGameSessionForInviteCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindGameSessionForInviteComplete, InviteEvent);
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystem, PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindGameSessionForInviteCompleteDelegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnFindGameSessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; bWasSuccessful: %s; SessionId: %s"), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful), *Result.GetSessionIdStr());

	if (!bWasSuccessful)
	{
		AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to notify player about invite to session as we failed to query data about session from backend!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// Find sender ID in the backend session data so that we can get their platform info for the ID
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Result.Session.SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our session's information instance is invalid!"));
		return;
	}

	const TSharedPtr<FAccelByteModelsV2GameSession> GameSessionData = SessionInfo->GetBackendSessionDataAsGameSession();
	if (!ensure(GameSessionData.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our local copy of the backend session data is invalid!"));
		return;
	}

	FUniqueNetIdPtr SenderId = nullptr;
	if (InviteEvent.SenderID.StartsWith(ClientIdPrefix))
	{
		// If the sender ID from the notification starts with a 'client-' prefix, then we just want to convert the sender ID
		// to a FUniqueNetIdAccelByteResource and pass it through the delegate.
		const FString ClientSenderId = InviteEvent.SenderID.Mid(ClientIdPrefix.Len(), InviteEvent.SenderID.Len() - ClientIdPrefix.Len());
		SenderId = FUniqueNetIdAccelByteResource::Create(ClientSenderId);
	}
	else
	{
		// Otherwise, the sender is a user and we should get their platform identifiers from the members array to construct
		// a FUniqueNetIdAccelByteUser ID instance.
		FAccelByteModelsV2SessionUser* FoundSender = GameSessionData->Members.FindByPredicate([&InviteEvent](const FAccelByteModelsV2SessionUser& User) {
			return User.ID == InviteEvent.SenderID;
		});

		if (FoundSender != nullptr)
		{
			// Create a new unique ID instance for the user that sent the invite, complete with their platform info
			FAccelByteUniqueIdComposite IdComponents;
			IdComponents.Id = FoundSender->ID;
			IdComponents.PlatformType = FoundSender->PlatformID;
			IdComponents.PlatformId = FoundSender->PlatformUserID;

			SenderId = FUniqueNetIdAccelByteUser::Create(IdComponents);
		}
	}

	if (!ensure(SenderId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as the ID of the sender of the invite is invalid!"));
		return;
	}

	FOnlineSessionInviteAccelByte NewInvite;
	NewInvite.SessionType = EAccelByteV2SessionType::GameSession;
	NewInvite.Session = Result;
	NewInvite.SenderId = SenderId;
	SessionInvites.Emplace(NewInvite);

	TriggerOnSessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), AccelByteSubsystem->GetAppId(), Result);
	TriggerOnV2SessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), NewInvite);
	TriggerOnInviteListUpdatedDelegates();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGameSessionMembersChangedNotification(FAccelByteModelsV2GameSessionMembersChangedEvent MembersChangedEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s; JoinerId: %s"), *MembersChangedEvent.Session.ID, *MembersChangedEvent.JoinerID);

	FNamedOnlineSession* Session = GetNamedSessionById(MembersChangedEvent.Session.ID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with members changed event as we do not have the session stored locally!"));
		return;
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(MembersChangedEvent.Session));

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

	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(UpdatedGameSession));

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnKickedFromGameSessionNotification(FAccelByteModelsV2GameSessionUserKickedEvent KickedEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *KickedEvent.SessionID);

	// First check if the inbound session ID matches one stored locally
	FNamedOnlineSession* Session = GetNamedSessionById(*KickedEvent.SessionID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session kicked notification as we do not have the session stored locally!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session kicked notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session kicked notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// If the game session we're kicked from matches the local session found, destroy the session, which will also
	// trigger leave if needed
	TriggerOnKickedFromSessionDelegates(Session->SessionName);
	DestroySession(Session->SessionName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnDsStatusChangedNotification(FAccelByteModelsV2DSStatusChangedNotif DsStatusChangeEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *DsStatusChangeEvent.SessionID);

	FNamedOnlineSession* Session = GetNamedSessionById(DsStatusChangeEvent.SessionID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with new DS status as session does not exist locally!"));
		return;
	}
	
	if (!DsStatusChangeEvent.Error.IsEmpty())
	{
		TriggerOnSessionServerErrorDelegates(Session->SessionName, DsStatusChangeEvent.Error);
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("DS status changed with an error message attached! Error message: %s"), *DsStatusChangeEvent.Error);
		return;
	}

	const FString ServerStatus = DsStatusChangeEvent.Session.DSInformation.Server.Status;
	const bool bStatusIsReady = ServerStatus.Equals(TEXT("READY"), ESearchCase::IgnoreCase) || ServerStatus.Equals(TEXT("BUSY"), ESearchCase::IgnoreCase);

	// If the new status is not ready, then we cannot do anything, so just bail
	if (!bStatusIsReady)
	{
		// #NOTE No warning here as technically nothing bad happened
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(DsStatusChangeEvent.Session), true);

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
		if (!ensure(Session.IsValid()))
		{
			continue;
		}

		const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
		if (SessionType == EAccelByteV2SessionType::PartySession)
		{
			return Session.Get();
		}
	}

	return nullptr;
}

void FOnlineSessionV2AccelByte::OnInvitedToPartySessionNotification(FAccelByteModelsV2PartyInvitedEvent InviteEvent, int32 LocalUserNum)
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InviteEvent.PartyID);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as we could not create a valid session unique ID from the ID in the notification!"));
		return;
	}

	const FOnSingleSessionResultCompleteDelegate OnFindPartySessionForInviteCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindPartySessionForInviteComplete, InviteEvent);
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2PartyById>(AccelByteSubsystem, PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindPartySessionForInviteCompleteDelegate);
}

void FOnlineSessionV2AccelByte::OnKickedFromPartySessionNotification(FAccelByteModelsV2PartyUserKickedEvent KickedEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *KickedEvent.PartyID);

	// First check if the inbound party ID matches one stored locally
	FNamedOnlineSession* Session = GetNamedSessionById(*KickedEvent.PartyID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session kicked notification as we do not have the session stored locally!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session kicked notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session kicked notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// If the party session we're kicked from matches the local session found, destroy the session, which will also
	// trigger leave if needed
	TriggerOnKickedFromSessionDelegates(Session->SessionName);
	DestroySession(Session->SessionName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
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
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// Find sender ID in the backend session data so that we can get their platform info for the ID
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Result.Session.SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our session's information instance is invalid!"));
		return;
	}

	const TSharedPtr<FAccelByteModelsV2PartySession> PartySessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	if (!ensure(PartySessionData.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our local copy of the backend session data is invalid!"));
		return;
	}

	FUniqueNetIdPtr SenderId = nullptr;
	if (InviteEvent.SenderID.StartsWith(ClientIdPrefix))
	{
		// If the sender ID from the notification starts with a 'client-' prefix, then we just want to convert the sender ID
		// to a FUniqueNetIdAccelByteResource and pass it through the delegate.
		const FString ClientSenderId = InviteEvent.SenderID.Mid(ClientIdPrefix.Len(), InviteEvent.SenderID.Len() - ClientIdPrefix.Len());
		SenderId = FUniqueNetIdAccelByteResource::Create(ClientSenderId);
	}
	else
	{
		// Otherwise, the sender is a user and we should get their platform identifiers from the members array to construct
		// a FUniqueNetIdAccelByteUser ID instance.
		FAccelByteModelsV2SessionUser* FoundSender = PartySessionData->Members.FindByPredicate([&InviteEvent](const FAccelByteModelsV2SessionUser& User) {
			return User.ID == InviteEvent.SenderID;
			});

		if (FoundSender != nullptr)
		{
			// Create a new unique ID instance for the user that sent the invite, complete with their platform info
			FAccelByteUniqueIdComposite IdComponents;
			IdComponents.Id = FoundSender->ID;
			IdComponents.PlatformType = FoundSender->PlatformID;
			IdComponents.PlatformId = FoundSender->PlatformUserID;

			SenderId = FUniqueNetIdAccelByteUser::Create(IdComponents);
		}
	}

	if (!ensure(SenderId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as the ID of the sender of the invite is invalid!"));
		return;
	}

	FOnlineSessionInviteAccelByte NewInvite;
	NewInvite.SessionType = EAccelByteV2SessionType::PartySession;
	NewInvite.Session = Result;
	NewInvite.SenderId = SenderId;
	SessionInvites.Emplace(NewInvite);

	TriggerOnSessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), AccelByteSubsystem->GetAppId(), Result);
	TriggerOnV2SessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), NewInvite);
	TriggerOnInviteListUpdatedDelegates();
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnPartySessionMembersChangedNotification(FAccelByteModelsV2PartyMembersChangedEvent MembersChangedEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s; JoinerId: %s"), *MembersChangedEvent.Session.ID, *MembersChangedEvent.JoinerID);

	FNamedOnlineSession* Session = GetNamedSessionById(MembersChangedEvent.Session.ID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with with members changed event as we do not have the session stored locally!"));
		return;
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2PartySession>(MembersChangedEvent.Session));

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

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2PartySession>(UpdatedPartySession));

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
	if (!ensure(SessionInfo.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as the locally stored session has invalid session info!"), *RejectEvent.PartyID);
		return;
	}

	TSharedPtr<FAccelByteModelsV2PartySession> SessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	if (!ensure(SessionData.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as the locally stored session has invalid backend data!"), *RejectEvent.PartyID);
		return;
	}

	SessionData->Members = RejectEvent.Members;

	bool bHasInvitedPlayersChanged = false;
	bool bHasJoinedMembersChanged = false;
	SessionInfo->UpdatePlayerLists(bHasJoinedMembersChanged, bHasInvitedPlayersChanged);
	
	if (bHasInvitedPlayersChanged)
	{
		FUniqueNetIdPtr UniqueRejectID = nullptr;
		FAccelByteModelsV2SessionUser* FoundSender = RejectEvent.Members.FindByPredicate([&RejectEvent](const FAccelByteModelsV2SessionUser& User) {
			return User.ID == RejectEvent.RejectedID;
		});

		if (FoundSender != nullptr)
		{
			FAccelByteUniqueIdComposite IdComponents;
			IdComponents.Id = FoundSender->ID;
			IdComponents.PlatformType = FoundSender->PlatformID;
			IdComponents.PlatformId = FoundSender->PlatformUserID;

			UniqueRejectID = FUniqueNetIdAccelByteUser::Create(IdComponents);
			TriggerOnSessionInviteRejectedDelegates(Session->SessionName, UniqueRejectID.ToSharedRef().Get());
		}

		TriggerOnSessionInvitesChangedDelegates(Session->SessionName);
	}

	SessionInfo->UpdateLeaderId();

	TriggerOnUpdateSessionCompleteDelegates(Session->SessionName, true);
	TriggerOnSessionUpdateReceivedDelegates(Session->SessionName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::UpdateSessionMembers(FNamedOnlineSession* Session, const TArray<FAccelByteModelsV2SessionUser>& PreviousMembers, const bool bHasInvitedPlayersChanged)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Session->GetSessionIdStr());

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not diff members for session '%s' as the local session has invalid session info!"), *Session->GetSessionIdStr());
		return;
	}

	const TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!ensure(SessionData.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not diff members for session '%s' as its session info has invalid backend data!"), *Session->GetSessionIdStr());
		return;
	}

	if (bHasInvitedPlayersChanged)
	{
		TriggerOnSessionInvitesChangedDelegates(Session->SessionName);
	}

	// We need to diff the previous members array and the new members array to figure out what changed.
	// If the status changes to Leave or Disconnect, we need to unregister that player. If it changes
	// to join or connect we need to register them.
	for (const FAccelByteModelsV2SessionUser& NewMember : SessionData->Members)
	{
		const FAccelByteModelsV2SessionUser* PreviousMember = PreviousMembers.FindByPredicate([&NewMember](const FAccelByteModelsV2SessionUser& Member) {
			return Member.ID == NewMember.ID;
		});

		// If this user's status hasn't changed, then we want to ensure that we have this player in the RegisteredPlayers
		// array. If they are already in the array, then skip. Otherwise, register them.
		if (PreviousMember != nullptr && PreviousMember->Status == NewMember.Status)
		{
			const bool bIsJoined = (PreviousMember->Status == EAccelByteV2SessionMemberStatus::JOINED || PreviousMember->Status == EAccelByteV2SessionMemberStatus::CONNECTED);
			const bool bNeedsRegistration = bIsJoined && !Session->RegisteredPlayers.ContainsByPredicate([&PreviousMember](const FUniqueNetIdRef& PlayerId) {
				FUniqueNetIdAccelByteUserRef AccelBytePlayerId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);
				return AccelBytePlayerId->GetAccelByteId() == PreviousMember->ID;
			});

			if (bNeedsRegistration)
			{
				RegisterJoinedSessionMember(Session, *PreviousMember);
			}

			continue;
		}

		const bool bIsJoinStatus = NewMember.Status == EAccelByteV2SessionMemberStatus::JOINED || NewMember.Status == EAccelByteV2SessionMemberStatus::CONNECTED;
		const bool bIsLeaveStatus = NewMember.Status == EAccelByteV2SessionMemberStatus::LEFT || NewMember.Status == EAccelByteV2SessionMemberStatus::KICKED || NewMember.Status == EAccelByteV2SessionMemberStatus::DROPPED;

		if (bIsJoinStatus)
		{
			RegisterJoinedSessionMember(Session, NewMember);
		}
		else if (bIsLeaveStatus)
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
	if (Session->LocalOwnerId.IsValid() && LeftUserId.ToSharedRef().Get() == Session->LocalOwnerId.ToSharedRef().Get())
	{
		DestroySession(Session->SessionName);
	}
}

void FOnlineSessionV2AccelByte::OnMatchmakingStartedNotification(FAccelByteModelsV2StartMatchmakingNotif MatchmakingStartedNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; TicketID: %s; PartyID: %s; Namespace: %s; MatchPool: %s"),
		LocalUserNum, *MatchmakingStartedNotif.TicketID, *MatchmakingStartedNotif.PartyID, *MatchmakingStartedNotif.Namespace, *MatchmakingStartedNotif.MatchPool);

	if (CurrentMatchmakingSearchHandle.IsValid())
	{
		// Since we already have a matchmaking search handle, we don't need to do anything else here and can bail
		TriggerOnMatchmakingStartedDelegates();
		AB_OSS_INTERFACE_TRACE_END(TEXT("CurrentMatchmakingSearchHandle is valid"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle matchmaking start notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr LocalPlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(LocalPlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle matchmaking start notification as a unique ID could not be found for player at index %d!"), LocalUserNum);
		return;
	}

	// If we don't have a valid session search instance stored, then the party leader has started matchmaking, in which we
	// need to create our own search handle to get notifications from matchmaking to join later
	CurrentMatchmakingSearchHandle = MakeShared<FOnlineSessionSearchAccelByte>();
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;
	CurrentMatchmakingSearchHandle->SearchingPlayerId = LocalPlayerId;
	CurrentMatchmakingSearchHandle->TicketId = MatchmakingStartedNotif.TicketID;
	CurrentMatchmakingSearchHandle->MatchPool = MatchmakingStartedNotif.MatchPool;

	// #TODO (Maxwell) Revisit this to somehow give developers a way to choose what session name they are matching for if a party
	// member receives a matchmaking notification. Since matchmaking shouldn't just be limited to game sessions.
	CurrentMatchmakingSearchHandle->SearchingSessionName = NAME_GameSession;

	TriggerOnMatchmakingStartedDelegates();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnMatchmakingMatchFoundNotification(FAccelByteModelsV2MatchFoundNotif MatchFoundEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *MatchFoundEvent.Id);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as a unique ID could not be found for player at index %d!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(MatchFoundEvent.Id);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as a unique ID could not be created for the match's session ID!"));
		return;
	}

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as we do not have a valid search handle to save results to!"));
		return;
	}

	const FOnSingleSessionResultCompleteDelegate OnFindMatchmakingGameSessionByIdCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindMatchmakingGameSessionByIdComplete);
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystem, PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindMatchmakingGameSessionByIdCompleteDelegate);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnMatchmakingExpiredNotification(FAccelByteModelsV2MatchmakingExpiredNotif MatchmakingExpiredNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Ticket id %s"), *MatchmakingExpiredNotif.TicketID);

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("no matchmaking search handle found"));
		return;
	}
	
	if(CurrentMatchmakingSearchHandle->TicketId != MatchmakingExpiredNotif.TicketID)
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

void FOnlineSessionV2AccelByte::OnMatchmakingCanceledNotification(FAccelByteModelsV2MatchmakingCanceledNotif MatchmakingCanceledNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Make sure we're matchmaking first
	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to handle matchmaking canceled notification as no matchmaking search handle was found!"));
		return;
	}

	// Clear current matchmaking handle, and mark as failed
	FName SessionName = CurrentMatchmakingSearchHandle->SearchingSessionName;
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
	CurrentMatchmakingSearchHandle.Reset();
	CurrentMatchmakingSessionSettings = {};

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Matchmaking ticket canceled!"));
	TriggerOnMatchmakingCompleteDelegates(SessionName, false);
	TriggerOnMatchmakingCanceledDelegates();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnFindMatchmakingGameSessionByIdComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finish matchmaking as we do not have a valid search handle instance!"));

		// #TODO Since we don't have a way to get the session name we were matching for, we just have to default to game
		// session. Figure out a way to make this more flexible.
		TriggerOnMatchmakingCompleteDelegates(NAME_GameSession, false);

		return;
	}

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

	CurrentMatchmakingSearchHandle->SearchResults.Emplace(Result);
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Done;

	TriggerOnMatchmakingCompleteDelegates(CurrentMatchmakingSearchHandle->SearchingSessionName, true);

	// After triggering this delegate, we will want to reset the current matchmaking search handle so that we're able to re-enter matchmaking at a later time
	CurrentMatchmakingSearchHandle.Reset();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnServerReceivedSessionComplete_Internal(FName SessionName)
{
	// Flag that we are done getting session associated with this server
	bIsGettingServerClaimedSession = false;

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
	if (!ensure(IdentityInterface.IsValid()))
	{
		UE_LOG_AB(Warning, TEXT("Failed to setup AccelByte P2P connection as our identity interface is invalid!"));
		return;
	}

	const AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalPlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		UE_LOG_AB(Warning, TEXT("Failed to setup AccelByte P2P connection as an API client could not be found for player with ID '%s'!"), *LocalPlayerId.ToDebugString());
		return;
	}

	FAccelByteNetworkUtilitiesModule::Get().Setup(ApiClient);

	FAccelByteNetworkUtilitiesModule::Get().EnableHosting();
}

void FOnlineSessionV2AccelByte::TeardownAccelByteP2PConnection()
{
	FAccelByteNetworkUtilitiesModule::Get().UnregisterDefaultSocketSubsystem();
	FAccelByteNetworkUtilitiesModule::Get().DisableHosting();
}

void FOnlineSessionV2AccelByte::OnICEConnectionComplete(const FString& PeerId,
	const EAccelByteP2PConnectionStatus& Status, FName SessionName, EOnlineSessionP2PConnectedAction Action)
{
	UE_LOG_AB(Log, TEXT("FOnlineSessionV2AccelByte::OnICEConnectionComplete"))
	EOnJoinSessionCompleteResult::Type Result;
	
	switch (Status)
	{
	case EAccelByteP2PConnectionStatus::Success:
		Result = EOnJoinSessionCompleteResult::Success;
		break;

	case EAccelByteP2PConnectionStatus::SignalingServerDisconnected:
	case EAccelByteP2PConnectionStatus::HostResponseTimeout:
	case EAccelByteP2PConnectionStatus::PeerIsNotHosting:
		Result = EOnJoinSessionCompleteResult::SessionDoesNotExist;
		break;

	case EAccelByteP2PConnectionStatus::JuiceGatherFailed:
	case EAccelByteP2PConnectionStatus::JuiceGetLocalDescriptionFailed:
	case EAccelByteP2PConnectionStatus::JuiceConnectionFailed:
	case EAccelByteP2PConnectionStatus::FailedGettingTurnServer:
	case EAccelByteP2PConnectionStatus::FailedGettingTurnServerCredential:
		Result = EOnJoinSessionCompleteResult::CouldNotRetrieveAddress;
		break;

	default:
		Result = EOnJoinSessionCompleteResult::UnknownError;
	}

	FAccelByteNetworkUtilitiesModule::Get().RegisterICERequestConnectFinishedDelegate(nullptr);

	AsyncTask(ENamedThreads::GameThread, [this, Action, SessionName, Result, PeerId] {
		if(Action == EOnlineSessionP2PConnectedAction::Join)
        {
			FNamedOnlineSession* Session = GetNamedSession(SessionName);
			check(Session != nullptr);

			TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
			check(SessionInfo.IsValid());
			const TTuple<FString, int32> PeerChannel = FAccelByteNetworkUtilitiesModule::ExtractPeerAndChannel(PeerId);
			SessionInfo->SetP2PChannel(PeerChannel.Value);
			
			TriggerOnJoinSessionCompleteDelegates(SessionName, Result);
        }
		else if (Action == EOnlineSessionP2PConnectedAction::Update)
		{
			TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
			TriggerOnSessionUpdateReceivedDelegates(SessionName);
		}
	});
}

void FOnlineSessionV2AccelByte::ConnectToDSHub(const FString& ServerName)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("ServerName: %s"), *ServerName);

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// First, register any delegates that we want to listen to from DS hub
	const AccelByte::GameServerApi::FOnServerClaimedNotification OnServerClaimedNotificationDelegate = AccelByte::GameServerApi::FOnServerClaimedNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnServerClaimedNotification);
	FRegistry::ServerDSHub.SetOnServerClaimedNotificationDelegate(OnServerClaimedNotificationDelegate);

	const AccelByte::GameServerApi::FOnV2BackfillProposalNotification OnV2BackfillProposalNotificationDelegate = AccelByte::GameServerApi::FOnV2BackfillProposalNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnV2BackfillProposalNotification);
	FRegistry::ServerDSHub.SetOnV2BackfillProposalNotificationDelegate(OnV2BackfillProposalNotificationDelegate);

	const AccelByte::GameServerApi::FOnV2SessionMemberChangedNotification OnV2SessionMemberChangedNotification = AccelByte::GameServerApi::FOnV2SessionMemberChangedNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnV2DsSessionMemberChangedNotification);
	FRegistry::ServerDSHub.SetOnV2SessionMemberChangedNotificationDelegate(OnV2SessionMemberChangedNotification);

	// Finally, connect to the DS hub websocket
	FRegistry::ServerDSHub.Connect(ServerName);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::DisconnectFromDSHub()
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// Unbind any delegates that we want to stop listening to DS hub for
	FRegistry::ServerDSHub.SetOnServerClaimedNotificationDelegate(AccelByte::GameServerApi::FOnServerClaimedNotification());
	FRegistry::ServerDSHub.SetOnV2BackfillProposalNotificationDelegate(AccelByte::GameServerApi::FOnV2BackfillProposalNotification());
	FRegistry::ServerDSHub.SetOnV2SessionMemberChangedNotificationDelegate(AccelByte::GameServerApi::FOnV2SessionMemberChangedNotification());
	
	// Finally, disconnect the DS hub websocket
	FRegistry::ServerDSHub.Disconnect();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnServerClaimedNotification(const FAccelByteModelsServerClaimedNotification& Notification)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Notification.Session_id)

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// Also bail if we already have a game session set up
	if (GetNamedSession(NAME_GameSession) != nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// Take the session ID we got for the server claim and retrieve session data for it
	GetServerClaimedSession(NAME_GameSession, Notification.Session_id);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnV2BackfillProposalNotification(const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Notification)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("BackfillTicketId: %s; ProposalId: %s"), *Notification.BackfillTicketID, *Notification.ProposalID);

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	TriggerOnBackfillProposalReceivedDelegates(Notification);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnV2DsSessionMemberChangedNotification(const FAccelByteModelsV2GameSession& Notification)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Notification.ID);

	FNamedOnlineSession* Session = GetNamedSessionById(Notification.ID);
	if (Session == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with members changed event as we do not have the session stored locally!"));
		return;
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(Notification));
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SendReadyToAMS()
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Send Ready Message to AMS"));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}
	FRegistry::ServerAMS.SendReadyMessage();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::DisconnectFromAMS()
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// Unbind any delegates that we want to stop listening to AMS for
	FRegistry::ServerAMS.SetOnAMSDrainReceivedDelegate(AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived());

	// Finally, disconnect the DS AMS websocket
	FRegistry::ServerAMS.Disconnect();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::InitMetricExporter(const FString& Address, uint16 Port, uint16 IntervalSeconds)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FRegistry::ServerMetricExporter.Initialize(Address, Port, IntervalSeconds);
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetMetricLabel(const FString& Key, const FString& Value)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FRegistry::ServerMetricExporter.SetLabel(Key, Value);
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::EnqueueMetric(const FString& Key, double Value)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FRegistry::ServerMetricExporter.EnqueueMetric(Key, Value);
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::EnqueueMetric(const FString& Key, int32 Value)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FRegistry::ServerMetricExporter.EnqueueMetric(Key, Value);
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::EnqueueMetric(const FString& Key, const FString& Value)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FRegistry::ServerMetricExporter.EnqueueMetric(Key, Value);
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetOptionalMetricsEnabled(bool Enable)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FRegistry::ServerMetricExporter.SetOptionalMetricsEnabled(Enable);
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetMetricCollector(const TSharedPtr<IAccelByteStatsDMetricCollector>& Collector)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FRegistry::ServerMetricExporter.SetStatsDMetricCollector(Collector);
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::InitializePlayerAttributes(const FUniqueNetId& LocalPlayerId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s"), *LocalPlayerId.ToDebugString());

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteInitializePlayerAttributes>(AccelByteSubsystem, LocalPlayerId);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

FAccelByteModelsV2PlayerAttributes* FOnlineSessionV2AccelByte::GetInternalPlayerAttributes(const FUniqueNetId& LocalPlayerId)
{
	return UserIdToPlayerAttributesMap.Find(LocalPlayerId.AsShared());
}

void FOnlineSessionV2AccelByte::StorePlayerAttributes(const FUniqueNetId& LocalPlayerId, FAccelByteModelsV2PlayerAttributes&& Attributes)
{
	// #NOTE: Usage of .Find is usually incorrect since the hash of the ID will be different depending on if platform
	// type/ID is missing, however since this will only be used for local players this should be fine as we should always
	// have a local player's platform information.
	FAccelByteModelsV2PlayerAttributes* FoundAttributes = UserIdToPlayerAttributesMap.Find(LocalPlayerId.AsShared());
	if (FoundAttributes != nullptr)
	{
		*FoundAttributes = MoveTempIfPossible(Attributes);
	}
	else
	{
		UserIdToPlayerAttributesMap.Add(LocalPlayerId.AsShared(), MoveTempIfPossible(Attributes));
	}
}

bool FOnlineSessionV2AccelByte::ServerQueryGameSessions(const FAccelByteModelsV2ServerQueryGameSessionsRequest& Request, int64 Offset, int64 Limit)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("ServerQueryGameSessions only works for Dedicated Server"));
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2>(AccelByteSubsystem, Request, Offset, Limit);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::ServerQueryPartySessions(const FAccelByteModelsV2QueryPartiesRequest& Request, int64 Offset, int64 Limit)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("ServerQueryPartySessions only works for Dedicated Server"));
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2>(AccelByteSubsystem, Request, Offset, Limit);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::OnAMSDrain()
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	TriggerOnAMSDrainReceivedDelegates();

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
