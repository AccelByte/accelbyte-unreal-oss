// Copyright (c) 2022 - 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteConnectLobby.h"
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
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteUpdateReservedPartyStorage.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteGetPartySessionStorage.h"
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
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteSendReadyToAMS.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteUpdateDSInformation.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineVoiceInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Misc/Timespan.h"
#include "Api/AccelByteLobbyApi.h"
#include "AccelByteNetworkUtilities.h"
#include "OnlineSessionSettingsAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteError.h"
#include "Core/AccelByteUtilities.h"

#include "OnlinePresenceErrors.h"
#include "AsyncTasks/Matchmaking/OnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets.h"
#include "AsyncTasks/Matchmaking/OnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails.h"
#include "AsyncTasks/PartyV2/OnlineAsyncTaskAccelByteCancelV2PartyInvite.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteSendDSSessionReady.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteCancelV2GameSessionInvite.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteGenerateNewV2GameCode.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteJoinV2GameSessionByCode.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelBytePromoteV2GameSessionLeader.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage.h"
#include "AsyncTasks/SessionV2/OnlineTaskAccelByteRevokeV2GameCode.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteKickV2GameSession.h"
#include "AsyncTasks/Server/OnlineAsyncTaskAccelByteServerKickV2GameSession.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteRefreshV2ActiveSessions.h"
#include "AsyncTasks/SessionV2/OnlineAsyncTaskAccelByteRefreshActiveSessions.h"
#include "OnlineSubsystemAccelByteConfig.h"
#include "Misc/Optional.h"

// STL
#include <algorithm>

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineSessionV2AccelByte"
#define ACCELBYTE_P2P_TRAVEL_URL_FORMAT TEXT("accelbyte.%s:%d")

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

void FOnlineSessionInfoAccelByteV2::SetSessionLeaderStorage(const FJsonObjectWrapper& Data)
{
	SessionLeaderStorage = Data;
}

void FOnlineSessionInfoAccelByteV2::SetSessionMemberStorage(const FUniqueNetIdRef& UserId,
	const FJsonObjectWrapper& Data)
{
	SessionMembersStorages.Add(UserId, Data);
}

bool FOnlineSessionInfoAccelByteV2::GetSessionLeaderStorage(FJsonObjectWrapper& OutStorage) const
{
	if (SessionLeaderStorage.JsonObject.IsValid())
	{
		OutStorage = SessionLeaderStorage;
		return true;
	}

	return false;
}

bool FOnlineSessionInfoAccelByteV2::GetSessionMemberStorage(const FUniqueNetIdRef& UserId, FJsonObjectWrapper& OutStorage) const
{
	if (SessionMembersStorages.Contains(UserId) && SessionMembersStorages[UserId].JsonObject.IsValid())
	{
		OutStorage = SessionMembersStorages[UserId];
		return true;
	}

	return false;
}

bool FOnlineSessionInfoAccelByteV2::GetAllSessionMemberStorage(TUniqueNetIdMap<FJsonObjectWrapper>& OutStorage) const
{
	if (SessionMembersStorages.Num() > 0)
	{
		OutStorage = SessionMembersStorages;
		return true;
	}

	return false;
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
		const bool bIsInviteStatus = Member.StatusV2 == EAccelByteV2SessionMemberStatus::INVITED;
		const bool bIsJoinedStatus = Member.StatusV2 == EAccelByteV2SessionMemberStatus::JOINED 
			|| Member.StatusV2 == EAccelByteV2SessionMemberStatus::CONNECTED
			|| Member.StatusV2 == EAccelByteV2SessionMemberStatus::DISCONNECTED;
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
	if (GameBackendSessionData->Configuration.Type == EAccelByteV2SessionConfigurationServerType::DS &&
		bIsDsJoinable &&
		!GameBackendSessionData->DSInformation.Server.Ip.IsEmpty()) // Address crashing on PS5 if IP is empty (in case user is not member of session yet)
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

TSharedPtr<FJsonObject>& FOnlineSessionSearchAccelByte::GetSearchStorage()
{
	return SearchStorage;
}

bool FOnlineSessionSearchAccelByte::GetIsP2PMatchmaking() const
{
	return bIsP2PMatchmaking;
}

void FOnlineSessionSearchAccelByte::SetIsP2PMatchmaking(const bool IsP2PMatchmaking)
{
	bIsP2PMatchmaking = IsP2PMatchmaking;
}

void FOnlineSessionSearchAccelByte::SetSearchStorage(TSharedPtr<FJsonObject> const& JsonObject)
{
	SearchStorage = JsonObject;
}

FOnlineSessionInviteAccelByte::FOnlineSessionInviteAccelByte()
	: TimeManager(nullptr)
{
}

FOnlineSessionInviteAccelByte::FOnlineSessionInviteAccelByte(FAccelByteTimeManagerWPtr InTimeManager)
	: TimeManager(InTimeManager)
{}

bool FOnlineSessionInviteAccelByte::IsExpired()
{
	auto SynchedServerTime = FDateTime::MinValue();
	
	const FAccelByteTimeManagerPtr TimeManagerPtr = TimeManager.Pin();
	if(TimeManagerPtr.IsValid())
	{
		SynchedServerTime = TimeManagerPtr->GetCurrentServerTime();
	}
	
	// If we are unable to sync with the server time
	FDateTime CurrentTime = FDateTime::UtcNow();
	if (SynchedServerTime > FDateTime::MinValue())
	{
		CurrentTime = SynchedServerTime;
	}

	FTimespan DeltaTime = ExpiredAt - CurrentTime;
	return DeltaTime.GetTotalSeconds() < 1.0f;
}

TCHAR const* FOnlineSessionV2AccelByte::ServerSessionIdEnvironmentVariable = TEXT("NOMAD_META_session_id");

FOnlineSessionV2AccelByte::FOnlineSessionV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem->AsShared())
{
}

FOnlineSessionV2AccelByte::~FOnlineSessionV2AccelByte()
{
	OnSessionServerCheckGetSessionDelegate.Unbind();
	OnSessionInviteCheckGetSessionDelegate.Unbind();

	DisconnectFromAMS();
	DisconnectFromDSHub();
}

bool FOnlineSessionV2AccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineSessionV2AccelBytePtr& OutInterfaceInstance)
{
#if !AB_USE_V2_SESSIONS
	OutInterfaceInstance = nullptr;
	return false;
#else
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	return OutInterfaceInstance.IsValid();
#endif
}

bool FOnlineSessionV2AccelByte::GetFromSubsystem(const FOnlineSubsystemAccelByte* Subsystem, TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	if (Subsystem == nullptr)
	{
		return false;
	}

#if !AB_USE_V2_SESSIONS
	OutInterfaceInstance = nullptr;
	return false;
#else
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	return OutInterfaceInstance.IsValid();
#endif
}

bool FOnlineSessionV2AccelByte::GetFromWorld(const UWorld* World, FOnlineSessionV2AccelBytePtr& OutInterfaceInstance)
{
#if !AB_USE_V2_SESSIONS
	OutInterfaceInstance = nullptr;
	return false;
#else
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}
	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
#endif
}

void FOnlineSessionV2AccelByte::Init()
{
	if (IsRunningDedicatedServer())
	{
		const FOnServerReceivedSessionDelegate OnServerReceivedSessionDelegate = FOnServerReceivedSessionDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnServerReceivedSessionComplete_Internal);
		AddOnServerReceivedSessionDelegate_Handle(OnServerReceivedSessionDelegate);

		AddAccelByteOnConnectAMSCompleteDelegate_Handle(FAccelByteOnConnectAMSCompleteDelegate::CreateThreadSafeSP(AsShared(), &FOnlineSessionV2AccelByte::OnAMSConnectedCallback));
	}

	OnSessionServerCheckGetSessionDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(AsShared(), &FOnlineSessionV2AccelByte::OnSessionServerCheckGetSession);
	OnSessionInviteCheckGetSessionDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(AsShared(), &FOnlineSessionV2AccelByte::OnSessionInviteCheckGetSession);
}

void FOnlineSessionV2AccelByte::UpdateSessionEntries()
{
	// If we have no sessions with a pending update, then bail out
	if (SessionsWithPendingQueuedUpdates.Num() <= 0)
	{
		return;
	}
	
	FScopeLock ScopeLock(&SessionLock);

	for (TPair<FName, TSharedPtr<FNamedOnlineSession>>& SessionEntry : Sessions)
	{
		int32 PendingUpdateIndex = SessionsWithPendingQueuedUpdates.IndexOfByPredicate([SessionName = SessionEntry.Key](const FName& PendingUpdateSessionName) {
			return PendingUpdateSessionName.IsEqual(SessionName);
		});

		if (PendingUpdateIndex == INDEX_NONE)
		{
			// Session does not have an update pending, bail out
			continue;
		}

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

			// #NOTE Intentionally not removing pending update here to account for a race condition between websocket notification and join/create completion
			continue;
		}

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionEntry.Value->SessionInfo);
		if (!SessionInfo.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Could not check session for updates as the session doesn't have a valid session info object!"));

			// Remove the pending update as we do not have a way to retrieve the latest update data
			SessionsWithPendingQueuedUpdates.RemoveAt(PendingUpdateIndex);
			continue;
		}

		const TSharedPtr<FAccelByteModelsV2BaseSession> ExistingBackendData = SessionInfo->GetBackendSessionData();
		if (!ensure(ExistingBackendData.IsValid()))
		{
			UE_LOG_AB(Warning, TEXT("Could not check session for updates as the session info doesn't have a valid backend session data object!"));

			// Remove the pending update as we do not have a way to compare existing data with latest update
			SessionsWithPendingQueuedUpdates.RemoveAt(PendingUpdateIndex);
			continue;
		}

		// If there is no latest update, or the latest update's version is not greater than the current data's version, we skip this session
		const TSharedPtr<FAccelByteModelsV2BaseSession> LatestUpdate = SessionInfo->GetLatestBackendSessionDataUpdate();
		const bool bHasUpdateToApply = LatestUpdate.IsValid() && LatestUpdate->Version > ExistingBackendData->Version;
		if (!bHasUpdateToApply)
		{
			SessionInfo->SetLatestBackendSessionDataUpdate(nullptr);
			SessionsWithPendingQueuedUpdates.RemoveAt(PendingUpdateIndex);
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

			// assumption we only update game session from notifications here
			// we don't update leader and member storages here since data from notifications might be reduced
			UpdateInternalGameSession(SessionEntry.Key, GameSessionData.ToSharedRef().Get(), bIsConnectingToP2P, false, false);
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

		// Clear the pending update entry to signal that the update is finished
		SessionsWithPendingQueuedUpdates.RemoveAt(PendingUpdateIndex);
	}
}

void FOnlineSessionV2AccelByte::OnMatchTicketCheckGetSessionInfoById(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SessionSearchResult)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	OnMatchTicketCheckGetMatchSessionDetailsDelegate.Unbind();

	// check if we already received actual match found notification but CurrentMatchmakingSearchHandle is still valid (still query resulting session)
	// but not actually stopping mitigation polling since it'll be stopped once we triggered match complete delegate
	if(bFindMatchmakingGameSessionByIdInProgress)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("already received match found notification, skipping matchmaking notif mitigation"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte subsystem is invalid, skipping matchmaking notif mitigation"));
		return;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Failed to check matchmaking progress as identity interface is invalid!"));
		return;
	}

	// already not matchmaking, either ticket expired or cancelled while waiting for endpoint response
	if(!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Failed to check matchmaking progress as current matchmaking search handler is invalid!"));
		StopMatchTicketCheckPoll();
		return;
	}
	const FUniqueNetId& SearchingPlayerId = CurrentMatchmakingSearchHandle->GetSearchingPlayerId().ToSharedRef().Get();

	const FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(SearchingPlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("failed to check matchmaking progress as ApiClient of the searching player %s is invalid!"), *SearchingPlayerId.ToDebugString());
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionSearchResult.Session.SessionInfo);
	if(!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("failed to check matchmaking progress as session info from search result is invalid!"), *SearchingPlayerId.ToDebugString());
		return;
	}

	TSharedPtr<FAccelByteModelsV2GameSession> BackendGameSession = SessionInfo->GetBackendSessionDataAsGameSession();
	
	FAccelByteModelsV2MatchFoundNotif MatchFoundNotif;
	MatchFoundNotif.Id = SessionSearchResult.GetSessionIdStr();
	MatchFoundNotif.Namespace = ApiClient->CredentialsRef->GetNamespace();
	MatchFoundNotif.Teams = BackendGameSession->Teams;
	for(const FString& TicketID : BackendGameSession->TicketIDs)
	{
		MatchFoundNotif.Tickets.Emplace(FAccelByteModelsV2Ticket{TicketID});
	}
	MatchFoundNotif.CreatedAt = FDateTime::UtcNow();
	MatchFoundNotif.MatchPool = CurrentMatchmakingSearchHandle->MatchPool;

	FString MessageContent;
	if(!FJsonObjectConverter::UStructToJsonObjectString(MatchFoundNotif, MessageContent))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("failed to spoof match found notif as converting notification to json string failed"), *SearchingPlayerId.ToDebugString());
		return;
	}

	FString LobbyNotif = FAccelByteNotificationSenderUtility::ComposeMMv2Notification("OnMatchFound", MessageContent);
	ApiClient->NotificationSender.SendLobbyNotification(LobbyNotif);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnMatchTicketCheckGetMatchTicketDetails(
	const FAccelByteModelsV2MatchmakingGetTicketDetailsResponse& Response, const FOnlineError& OnlineError)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	ClearOnGetMatchTicketDetailsCompleteDelegate_Handle(GetMatchTicketDetailsCompleteDelegateHandle);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to check matchmaking progress as AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Failed to check matchmaking progress as identity interface is invalid!"));
		return;
	}

	// check if we already received actual match found notification but CurrentMatchmakingSearchHandle is still valid (still query resulting session)
	// but not actually stopping mitigation polling since it'll be stopped once we triggered match complete delegate
	if(bFindMatchmakingGameSessionByIdInProgress)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("already received match found notification, skipping matchmaking notif mitigation"));
		return;
	}

	// already not matchmaking, either ticket expired or cancelled while waiting for endpoint response
	if(!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Failed to check matchmaking progress as our current matchmaking search handle is invalid"));
		StopMatchTicketCheckPoll();
		return;
	}
	const FUniqueNetId& SearchingPlayerId = CurrentMatchmakingSearchHandle->GetSearchingPlayerId().ToSharedRef().Get();

	const FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(SearchingPlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("failed to check matchmaking progress as ApiClient of the searching player %s is invalid!"), *SearchingPlayerId.ToDebugString());
		return;
	}

	int LocalUserNum;
	if(!ensure(IdentityInterface->GetLocalUserNum(SearchingPlayerId, LocalUserNum)))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("failed to check matchmaking progress as LocalUserNum of the searching player %s is not found!"), *SearchingPlayerId.ToDebugString());
		return;
	}

	if(!OnlineError.WasSuccessful())
	{
		const FString MMTicketNotFoundErrorCode = TEXT("FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails.matchmakingv2-failed-ticket-not-found");
		// current ticket not found. most likely the ticket already expired.
		if(OnlineError.ErrorCode ==  MMTicketNotFoundErrorCode)
		{
			FAccelByteModelsV2MatchmakingExpiredNotif ExpiredNotif;
			ExpiredNotif.Namespace = ApiClient->CredentialsRef->GetNamespace();
			ExpiredNotif.MatchPool = CurrentMatchmakingSearchHandle->MatchPool;
			ExpiredNotif.TicketID = CurrentMatchmakingSearchHandle->TicketId;
			ExpiredNotif.CreatedAt = FDateTime::UtcNow();

			FString ExpiredNotifJsonString;
			if(FJsonObjectConverter::UStructToJsonObjectString(ExpiredNotif, ExpiredNotifJsonString))
			{
				const FString LobbyMessage = FAccelByteNotificationSenderUtility::ComposeMMv2Notification("OnMatchmakingTicketExpired", ExpiredNotifJsonString);
				ApiClient->NotificationSender.SendLobbyNotification(LobbyMessage);
			}
			else
			{
				UE_LOG_AB(Log, TEXT("Failed to spoof ticket expire notification, failed to serialize expired notif"));
			}
		}
		else
		{
			// other unexpected error occurs set polling to next duration
			SetMatchTicketCheckPollToNextPollTime();
		}

		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Failed to spoof ticket expire notification, ticket info returned with error %s"), *OnlineError.ErrorCode);
		return;
	}

	// if the ticket is currently not active, not proposed, and with no match found means ticket is canceled
	if(!Response.IsActive && Response.ProposedProposal.Status.IsEmpty() && !Response.MatchFound)
	{
		FAccelByteModelsV2MatchmakingCanceledNotif CancelNotif;
		CancelNotif.Namespace = ApiClient->CredentialsRef->GetNamespace();
		CancelNotif.Reason = TEXT("ticket info polling responded with not active, no backfill proposed, and no match found");
		CancelNotif.TicketID = CurrentMatchmakingSearchHandle->TicketId;

		FString CancelNotifJsonString;
		if(FJsonObjectConverter::UStructToJsonObjectString(CancelNotif, CancelNotifJsonString))
		{
			const FString LobbyMessage = FAccelByteNotificationSenderUtility::ComposeMMv2Notification("OnMatchmakingTicketCanceled", CancelNotifJsonString);
			ApiClient->NotificationSender.SendLobbyNotification(LobbyMessage);
		}
		else
		{
			UE_LOG_AB(Log, TEXT("Failed to spoof ticket canceled notification, failed to serialize cancel notif"));
		}
		return;
	}

	// match not found yet, set next time to trigger check match ticket.
	if(!Response.MatchFound)
	{
		SetMatchTicketCheckPollToNextPollTime();
		return;
	}

	// Match found, check session info for stuffs we need to populate spoofing Match found notification
	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(Response.SessionId);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to spoof match found notification as a unique ID could not be created for the match's session ID!"));
		return;
	}
	
	OnMatchTicketCheckGetMatchSessionDetailsDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(AsShared(), &FOnlineSessionV2AccelByte::OnMatchTicketCheckGetSessionInfoById);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get(),
	CurrentMatchmakingSearchHandle->SearchingPlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnMatchTicketCheckGetMatchSessionDetailsDelegate);
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::FinalizeStartMatchmakingComplete()
{
	StartMatchTicketCheckPoll();
}

void FOnlineSessionV2AccelByte::StartMatchTicketCheckPoll()
{
	int64 InitialDelaySeconds { 30 };
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			InitialDelaySeconds = Config->GetMatchTicketPollStartDelaySeconds().GetValue();
		}
	}

	NextMatchmakingDetailPollTime = FDateTime::UtcNow() + FTimespan::FromSeconds(InitialDelaySeconds);

	UE_LOG_AB(VeryVerbose, TEXT("Start match ticket check poll, current time %s, next poll time %s"), *FDateTime::UtcNow().ToString(), *NextMatchmakingDetailPollTime.ToString());
}

void FOnlineSessionV2AccelByte::SetMatchTicketCheckPollToNextPollTime()
{
	int64 IntervalSeconds { 15 };
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			IntervalSeconds = Config->GetMatchTicketPollIntervalSeconds().GetValue();
		}
	}

	NextMatchmakingDetailPollTime = FDateTime::UtcNow() + FTimespan::FromSeconds(IntervalSeconds);
	UE_LOG_AB(VeryVerbose, TEXT("Set match ticket check next poll, current time %s, next poll time %s"), *FDateTime::UtcNow().ToString(), *NextMatchmakingDetailPollTime.ToString());
}

void FOnlineSessionV2AccelByte::StopMatchTicketCheckPoll()
{
	UE_LOG_AB(VeryVerbose, TEXT("stop match ticket check next poll"));
	NextMatchmakingDetailPollTime = FDateTime(0);
}

void FOnlineSessionV2AccelByte::SendDSStatusChangedNotif(const int32 LocalUserNum, const TSharedPtr<FAccelByteModelsV2GameSession>& SessionData)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to send session server notification AccelByte subsystem is invalid"));
		return;
	}

	const FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if(!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server notification as ApiClient for LocalUserNum %d is invalid!"), LocalUserNum);
		return;
	}

	// Need to make sure possible DS status from backend when receiving DSStatusChanged notif.
	const bool bShouldTriggerNotification = SessionData->DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::AVAILABLE ||
		SessionData->DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::FAILED_TO_REQUEST;

	if(!bShouldTriggerNotification)
	{
		return;
	}
	
	FAccelByteModelsV2DSStatusChangedNotif DSChangedNotif;
	DSChangedNotif.Session = *SessionData;
	DSChangedNotif.SessionID = SessionData->ID;
	DSChangedNotif.GameServer = SessionData->DSInformation.Server;
	if(SessionData->DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::FAILED_TO_REQUEST)
	{
		DSChangedNotif.Error = TEXT("Session failed claiming a DS");
	}

	FString DSStatusChangedJsonString;
	if(!FJsonObjectConverter::UStructToJsonObjectString(DSChangedNotif, DSStatusChangedJsonString))
	{
		UE_LOG_AB(VeryVerbose, TEXT("Failed sending server information for session id %s, unable to serialize notification"), *SessionData->ID);
		return;
	}
		
	const FString LobbyMessage = FAccelByteNotificationSenderUtility::ComposeSessionNotification("OnDSStatusChanged", DSStatusChangedJsonString);
	ApiClient->NotificationSender.SendLobbyNotification(LobbyMessage);
}

void FOnlineSessionV2AccelByte::CheckMatchmakingProgress()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte system is invalid, skip checking matchmaking progress"));
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte config is invalid, skip checking matchmaking progress"));
		return;
	}
	
	// matchmaking check is disabled, early exit
	if (!Config->GetEnableMatchTicketPolling().GetValue())
	{
		return;
	}
	
	// We are currently not matchmaking, early exit
	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		return;
	}

	// Not the time to trigger match detail polling, early exit
	if (FDateTime::UtcNow() < NextMatchmakingDetailPollTime || NextMatchmakingDetailPollTime == FDateTime(0))
	{
		return;
	}

	UE_LOG_AB(VeryVerbose, TEXT("Checking match ticket details from poll, current time %s, poll time %s"), *FDateTime::UtcNow().ToString(), *NextMatchmakingDetailPollTime.ToString());

	GetMatchTicketDetailsCompleteDelegateHandle = AddOnGetMatchTicketDetailsCompleteDelegate_Handle(
		FOnGetMatchTicketDetailsCompleteDelegate::CreateThreadSafeSP(AsShared(), &FOnlineSessionV2AccelByte::OnMatchTicketCheckGetMatchTicketDetails));
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails>(AccelByteSubsystemPtr.Get(),
		CurrentMatchmakingSearchHandle->SearchingPlayerId.ToSharedRef().Get(), CurrentMatchmakingSearchHandle->GetTicketId());

	StopMatchTicketCheckPoll();
}

void FOnlineSessionV2AccelByte::SendSessionInviteNotif(int32 LocalUserNum, const FString& SessionId) const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to send session invite notif AccelByte subsystem is invalid"));
		return;
	}

	const FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if(!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server notification as ApiClient for LocalUserNum %d is invalid!"), LocalUserNum);
		return;
	}

	SettingsPtr Settings = ApiClient->Settings;
	if(!Settings.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server notification as Settings for LocalUserNum %d is invalid!"), LocalUserNum);
		return;
	}
	
	FAccelByteModelsV2GameSessionUserInvitedEvent Invite;
	Invite.SessionID = SessionId;
	Invite.SenderID = ClientIdPrefix + Settings->ClientId;

	FString InviteJsonString;
	FJsonObjectConverter::UStructToJsonObjectString(Invite, InviteJsonString);

	const FString LobbyMessage = FAccelByteNotificationSenderUtility::ComposeSessionNotification("OnSessionInvited", InviteJsonString);
	ApiClient->NotificationSender.SendLobbyNotification(LobbyMessage);
}

void FOnlineSessionV2AccelByte::StartSessionInviteCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId,
	const FString& SessionId)
{
	int64 InitialDelaySeconds{30};

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			InitialDelaySeconds = Config->GetSessionInvitePollStartDelaySeconds().GetValue();
		}
	}

	FSessionInviteCheckPollItem PollItem{};
	PollItem.SessionId = SessionId;
	PollItem.SearchingPlayerId = SearchingPlayerId;
	PollItem.NextPollTime = FDateTime::UtcNow()+ FTimespan::FromSeconds(InitialDelaySeconds);
	
	{
		FScopeLock ScopeLock(&SessionInviteCheckTimesLock);
		SessionInviteCheckPollTimes.Emplace(PollItem);
	}

	UE_LOG_AB(VeryVerbose, TEXT("Start session invite check poll for session id %s, user %s, current time %s, next poll time %s"),
		*SessionId, *SearchingPlayerId->ToDebugString(), *FDateTime::UtcNow().ToString(), *PollItem.NextPollTime.ToString())
}

void FOnlineSessionV2AccelByte::SetSessionInviteCheckPollNextPollTime(const FUniqueNetIdPtr& SearchingPlayerId,
	const FString& SessionId)
{
	int64 IntervalSeconds{15};

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			IntervalSeconds = Config->GetSessionInvitePollIntervalSeconds().GetValue();
		}
	}

	FSessionInviteCheckPollItem PollItem{};
	PollItem.SessionId = SessionId;
	PollItem.SearchingPlayerId = SearchingPlayerId;
	PollItem.NextPollTime = FDateTime::UtcNow()+ FTimespan::FromSeconds(IntervalSeconds);
	
	{
		FScopeLock ScopeLock(&SessionInviteCheckTimesLock);
		SessionInviteCheckPollTimes.Emplace(PollItem);
	}

	UE_LOG_AB(VeryVerbose, TEXT("Set session invite check poll for session id %s, user %s, current time %s, next poll time %s"),
		*SessionId, *SearchingPlayerId->ToDebugString(), *FDateTime::UtcNow().ToString(), *PollItem.NextPollTime.ToString())
}

void FOnlineSessionV2AccelByte::StopSessionInviteCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId,
	const FString& SessionId)
{
	UE_LOG_AB(VeryVerbose, TEXT("Stopping session invite check poll. Session id %s, player %s"), *SessionId, *SearchingPlayerId->ToDebugString());
	FScopeLock ScopeLock(&SessionInviteCheckTimesLock);

	const int32 PollItemIndex = SessionInviteCheckPollTimes.IndexOfByPredicate([&](const FSessionInviteCheckPollItem& PollItem)
	{
		return SearchingPlayerId == PollItem.SearchingPlayerId && SessionId == PollItem.SessionId;
	});

	if(PollItemIndex != INDEX_NONE)
	{
		SessionInviteCheckPollTimes.RemoveAt(PollItemIndex);
	}
}

void FOnlineSessionV2AccelByte::OnSessionInviteCheckGetSession(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& OnlineSearchResult)
{
	// if invitation get info exist that means we received a notification while checking get session, no need to continue mitigation process
	if(SessionInvitationGetInfoInProgressExist(OnlineSearchResult.GetSessionIdStr()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Session invitation received while invite notif mitigation is in process, stopping session invite mitigation"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to send session invite as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session invite as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session invite as local user is invalid!"));
		return;
	}

	const FUniqueNetIdAccelByteUserPtr AccelByteUniqueUser = FUniqueNetIdAccelByteUser::TryCast(LocalUserId.ToSharedRef());
	if (!AccelByteUniqueUser.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session invite as local user is not accelbyte user!"));
		return;
	}

	FString AccelByteId = AccelByteUniqueUser->GetAccelByteId();

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(OnlineSearchResult.Session.SessionInfo);
	if(!SessionInfo.IsValid())
	{
		return;
	}

	// if the request fail we will recheck at next poll time
	if(!bWasSuccessful)
	{
		SetSessionInviteCheckPollNextPollTime(AccelByteUniqueUser, OnlineSearchResult.GetSessionIdStr());
		return;
	}

	const bool bCurrentUserInvited = SessionInfo->GetBackendSessionDataAsGameSession()->Members.ContainsByPredicate([&AccelByteId](const FAccelByteModelsV2SessionUser& Member)
	{
		return Member.ID == AccelByteId && Member.StatusV2 == EAccelByteV2SessionMemberStatus::INVITED;
	});

	// if current user is invited then send session invite notif to SDK
	if(bCurrentUserInvited)
	{
		SendSessionInviteNotif(LocalUserNum, OnlineSearchResult.GetSessionIdStr());
		return;
	}

	// if current user not found or not invited yet then set next poll time
	SetSessionInviteCheckPollNextPollTime(AccelByteUniqueUser, OnlineSearchResult.GetSessionIdStr());
}

void FOnlineSessionV2AccelByte::StartSessionServerCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId, const FName SessionName)
{
	int64 InitialDelaySeconds{30};

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			InitialDelaySeconds = Config->GetServerInfoPollStartDelaySeconds().GetValue();
		}
	}

	FSessionServerCheckPollItem PollItem{};
	PollItem.SessionName = SessionName;
	PollItem.SearchingPlayerId = SearchingPlayerId;
	PollItem.NextPollTime = FDateTime::UtcNow()+ FTimespan::FromSeconds(InitialDelaySeconds);
	
	{
		FScopeLock ScopeLock(&SessionServerCheckTimesLock);
		SessionServerCheckPollTimes.Emplace(PollItem);
	}
	
	UE_LOG_AB(VeryVerbose, TEXT("Start session server check poll for session name %s, user %s, current time %s, next poll time %s"),
		*SessionName.ToString(), *SearchingPlayerId->ToDebugString(), *FDateTime::UtcNow().ToString(), *PollItem.NextPollTime.ToString())
}

void FOnlineSessionV2AccelByte::SetSessionServerCheckPollNextPollTime(const FUniqueNetIdPtr& SearchingPlayerId, const FName SessionName)
{
	int64 IntervalSeconds{15};

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			IntervalSeconds = Config->GetServerInfoPollIntervalSeconds().GetValue();
		}
	}

	FSessionServerCheckPollItem PollItem{};
	PollItem.SessionName = SessionName;
	PollItem.SearchingPlayerId = SearchingPlayerId;
	PollItem.NextPollTime = FDateTime::UtcNow()+ FTimespan::FromSeconds(IntervalSeconds);
	
	{
		FScopeLock ScopeLock(&SessionServerCheckTimesLock);
		SessionServerCheckPollTimes.Emplace(PollItem);
	}

	UE_LOG_AB(VeryVerbose, TEXT("Set session server check poll for session name %s, user %s, current time %s, next poll time %s"),
		*SessionName.ToString(), *SearchingPlayerId->ToDebugString(), *FDateTime::UtcNow().ToString(), *PollItem.NextPollTime.ToString())
}

void FOnlineSessionV2AccelByte::StopSessionServerCheckPoll(const FUniqueNetIdPtr& SearchingPlayerId, const FName SessionName)
{
	UE_LOG_AB(VeryVerbose, TEXT("Stopping session server check poll. Session name %s, player %s"), *SessionName.ToString(), *SearchingPlayerId->ToDebugString());
	FScopeLock ScopeLock(&SessionServerCheckTimesLock);

	const int32 PollItemIndex = SessionServerCheckPollTimes.IndexOfByPredicate([&](const FSessionServerCheckPollItem& PollItem)
	{
		return SearchingPlayerId == PollItem.SearchingPlayerId && SessionName == PollItem.SessionName;
	});

	if(PollItemIndex != INDEX_NONE)
	{
		SessionServerCheckPollTimes.RemoveAt(PollItemIndex);
	}
}

void FOnlineSessionV2AccelByte::OnSessionServerCheckGetSession(int LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	if(!bWasSuccessful)
	{
		// todo something when request not success
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server as session info request failed!"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server as our identity interface is invalid!"));
		return;
	}

	const FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if(!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server as ApiClient for LocalUserNum %d is invalid!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!PlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session server as Unique player ID for LocalUserNum %d is invalid!"), LocalUserNum);
		return;
	}
	
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(OnlineSessionSearchResult.Session.SessionInfo);
	if(!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed checking session server information for session id %s as the session info is invalid"), *OnlineSessionSearchResult.GetSessionIdStr());
		return;
	}
	
	const TSharedPtr<FAccelByteModelsV2GameSession> BackendData = SessionInfo->GetBackendSessionDataAsGameSession();
	if(!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed checking session server information for session id %s as the backend session data is invalid"), *OnlineSessionSearchResult.GetSessionIdStr());
		return;
	}
	
	const FNamedOnlineSession* NamedSession = GetNamedSessionById(BackendData->ID);
	if(NamedSession == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed checking session server information for session id %s, no named session found"), *BackendData->ID);
		return;
	}

	const bool bSessionServerCheckComplete = BackendData->DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::FAILED_TO_REQUEST ||
		BackendData->DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::ENDED ||
		BackendData->DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::AVAILABLE;

	if(bSessionServerCheckComplete)
	{
		SendDSStatusChangedNotif(LocalUserNum, BackendData);
		
		return;
	}

	// other statuses we should wait for the next poll.
	SetSessionServerCheckPollNextPollTime(PlayerId, NamedSession->SessionName);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::CheckSessionServerProgress()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to send session server as our AccelByte subsystem is invalid"));
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to send session server as our AccelByte config is invalid"));
		return;
	}

	if (!Config->GetEnableServerInfoPolling().GetValue())
	{
		return;
	}

	FScopeLock ScopeLock(&SessionServerCheckTimesLock);

	TArray<int32> PollItemIndexesToRemove;
	for(int i = 0; i < SessionServerCheckPollTimes.Num(); i++)
	{
		FSessionServerCheckPollItem PollItem = SessionServerCheckPollTimes[i];
		if(FDateTime::UtcNow() < PollItem.NextPollTime)
		{
			continue;
		}

		const FNamedOnlineSession* NamedSession = GetNamedSession(PollItem.SessionName);
		if(NamedSession == nullptr)
		{
			UE_LOG_AB(Log, TEXT("Session with name %s doesn't exist to check session server progress for user %s, removing from poll list"), *PollItem.SessionName.ToString(), *PollItem.SearchingPlayerId->ToDebugString());
			PollItemIndexesToRemove.Insert(i, 0);
			continue;
		}

		UE_LOG_AB(VeryVerbose, TEXT("Checking session server progress, session name %s player %s polltime %s current time %s"), *PollItem.SessionName.ToString(), *PollItem.SearchingPlayerId->ToDebugString(), *PollItem.NextPollTime.ToString(), *FDateTime::UtcNow().ToString());

		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get(),
			PollItem.SearchingPlayerId.ToSharedRef().Get(), NamedSession->SessionInfo->GetSessionId(), OnSessionServerCheckGetSessionDelegate);

		PollItemIndexesToRemove.Insert(i, 0);
	}

	// remove already triggered check poll times
	for(const int32 Index : PollItemIndexesToRemove)
	{
		SessionServerCheckPollTimes.RemoveAt(Index);
	}
}

void FOnlineSessionV2AccelByte::CheckSessionInviteAfterMatchFound()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session invite as our AccelByte subsystem is invalid"));
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to check session invite as our AccelByte config is invalid"));
		return;
	}

	if (!Config->GetEnableSessionInvitePolling().GetValue())
	{
		return;
	}

	FScopeLock ScopeLock(&SessionInviteCheckTimesLock);

	TArray<int32> PollTimeIndexesToRemove;
	for(int i = 0; i < SessionInviteCheckPollTimes.Num(); i++)
	{
		FSessionInviteCheckPollItem PollItem = SessionInviteCheckPollTimes[i];

		// check time if the poll should trigger
		if(FDateTime::UtcNow() < PollItem.NextPollTime)
		{
			continue;
		}

		// if we already received an invite of this session or we already joined the session, remove this item from poll list.
		const bool bSessionJoined = GetNamedSessionById(PollItem.SessionId) != nullptr;
		const bool bInviteReceived = SessionInvites.ContainsByPredicate([&PollItem](const FOnlineSessionInviteAccelByte& Invite)
		{
			return Invite.Session.GetSessionIdStr() == PollItem.SessionId;
		});
		if(bInviteReceived || bSessionJoined)
		{
			UE_LOG_AB(VeryVerbose, TEXT("Checking session invite after match found, session id %s invite already received removing from poll"), *PollItem.SessionId);
			PollTimeIndexesToRemove.Insert(i, 0);
			continue;
		}

		FUniqueNetIdPtr SessionNetId = CreateSessionIdFromString(PollItem.SessionId);

		UE_LOG_AB(VeryVerbose, TEXT("Checking session invite after match found, session id %s player %s polltime %s current time %s"), *PollItem.SessionId, *PollItem.SearchingPlayerId->ToDebugString(), *PollItem.NextPollTime.ToString(), *FDateTime::UtcNow().ToString());
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get(),
			PollItem.SearchingPlayerId.ToSharedRef().Get(), SessionNetId.ToSharedRef().Get(), OnSessionInviteCheckGetSessionDelegate);

		PollTimeIndexesToRemove.Insert(i, 0);
	}

	// remove already triggered check poll times
	for(const int32 Index : PollTimeIndexesToRemove)
	{
		SessionInviteCheckPollTimes.RemoveAt(Index);
	}
}

void FOnlineSessionV2AccelByte::Tick(float DeltaTime)
{
	// Check if we have any canceled ticket IDs and if we have elapsed the time before clearing the tracked array, if so, clear it
	const double CurrentTimeSeconds = FPlatformTime::Seconds();
	if (CanceledTicketIds.Num() > 0 && CurrentTimeSeconds > LastCanceledTicketIdAddedTimeSeconds + ClearCanceledTicketIdsTimeInSeconds)
	{
		CanceledTicketIds.Empty();
	}
	
	UpdateSessionEntries();

	CheckMatchmakingProgress();

	CheckSessionServerProgress();

	CheckSessionInviteAfterMatchFound();
}

void FOnlineSessionV2AccelByte::RegisterSessionNotificationDelegates(const FUniqueNetId& PlayerId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s"), *PlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to register session notification as our AccelByte subsystem is invalid"));
		return;
	}

	InstanceIdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(InstanceIdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as our identity interface is invalid!"));
		return;
	}

	int32 LocalUserNum = 0;
	if (!ensure(InstanceIdentityInterface->GetLocalUserNum(PlayerId, LocalUserNum)))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as we could not get a local user index for player with ID '%s'!"), *PlayerId.ToDebugString());
		return;
	}

	AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(PlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as player '%s' has an invalid API client!"), *PlayerId.ToDebugString());
		return;
	}

	const auto Lobby = ApiClient->GetLobbyApi().Pin();
	if (!ensure(Lobby.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as player '%s' has an invalid Lobby API!"), *PlayerId.ToDebugString());
		return;
	}

	#define BIND_LOBBY_NOTIFICATION(NotificationDelegateName, Verb) \
		typedef AccelByte::Api::Lobby::FV2##NotificationDelegateName##Notif F##Verb##NotificationDelegate; \
		const F##Verb##NotificationDelegate On##Verb##NotificationDelegate = F##Verb##NotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::On##Verb##Notification, LocalUserNum); \
		Lobby->SetV2##NotificationDelegateName##NotifDelegate(On##Verb##NotificationDelegate); \

	// Begin Game Session Notifications
	BIND_LOBBY_NOTIFICATION(GameSessionInvited, InvitedToGameSession);
	BIND_LOBBY_NOTIFICATION(GameSessionInviteTimeout, GameSessionInvitationTimeout);
	BIND_LOBBY_NOTIFICATION(GameSessionMembersChanged, GameSessionMembersChanged);
	BIND_LOBBY_NOTIFICATION(GameSessionUpdated, GameSessionUpdated);
	BIND_LOBBY_NOTIFICATION(GameSessionKicked, KickedFromGameSession);
	BIND_LOBBY_NOTIFICATION(DSStatusChanged, DsStatusChanged);
	BIND_LOBBY_NOTIFICATION(GameSessionRejected, GameSessionInviteRejected);
	BIND_LOBBY_NOTIFICATION(GameSessionJoined, GameSessionJoined);
	BIND_LOBBY_NOTIFICATION(GameSessionEnded, GameSessionEnded);

	const THandler<FAccelByteModelsV2GameSessionInviteCanceledEvent> OnGameSessionInviteCanceled = THandler<FAccelByteModelsV2GameSessionInviteCanceledEvent>::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnGameSessionInviteCanceledNotification, LocalUserNum);
	OnGameSessionInviteCanceledHandle = Lobby->AddV2GameSessionInviteCanceledNotifDelegate(OnGameSessionInviteCanceled);
	//~ End Game Session Notifications

	// Begin Party Session Notifications
	BIND_LOBBY_NOTIFICATION(PartyInvited, InvitedToPartySession);
	BIND_LOBBY_NOTIFICATION(PartyInviteTimeout, PartySessionInvitationTimeout);
	BIND_LOBBY_NOTIFICATION(PartyMembersChanged, PartySessionMembersChanged);
	BIND_LOBBY_NOTIFICATION(PartyUpdated, PartySessionUpdated);
	BIND_LOBBY_NOTIFICATION(PartyKicked, KickedFromPartySession);
	BIND_LOBBY_NOTIFICATION(PartyRejected, PartySessionInviteRejected);
	const THandler<FAccelByteModelsV2PartyInviteCanceledEvent> OnPartyInviteCanceled = THandler<FAccelByteModelsV2PartyInviteCanceledEvent>::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnPartySessionInviteCanceledNotification, LocalUserNum);
	OnPartyInviteCanceledHandle = Lobby->AddV2PartyInviteCanceledNotifDelegate(OnPartyInviteCanceled);
	const THandler<FAccelByteModelsV2PartyCreatedEvent> OnPartyCreatedNotification = THandler<FAccelByteModelsV2PartyCreatedEvent>::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnPartySessionCreatedNotification, LocalUserNum);
	OnPartyCreatedHandle = Lobby->AddV2PartyCreatedNotifDelegate(OnPartyCreatedNotification);
	//~ End Party Session Notifications

	// Begin Matchmaking Notifications
	BIND_LOBBY_NOTIFICATION(MatchmakingStart, MatchmakingStarted);
	BIND_LOBBY_NOTIFICATION(MatchmakingMatchFound, MatchmakingMatchFound);
	BIND_LOBBY_NOTIFICATION(MatchmakingExpired, MatchmakingExpired);
	BIND_LOBBY_NOTIFICATION(MatchmakingCanceled, MatchmakingCanceled)
	//~ End Matchmaking Notifications

	// Begin Session Storage Notifications
	BIND_LOBBY_NOTIFICATION(SessionStorageChanged, SessionStorageChanged);
	//~ End Session Storage Notifications

	#undef BIND_LOBBY_NOTIFICATION

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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

FUniqueNetIdPtr FOnlineSessionV2AccelByte::CreateSessionIdFromString(const FString& SessionIdStr)
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerNum: %d; SessionName: %s"), HostingPlayerNum, *SessionName.ToString());

	if (IsRunningDedicatedServer())
	{
		// If we are trying to create a new session as a server, just pass an invalid user ID and continue. Server clients
		// do not have user IDs.
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Passing to create session!"));
		return CreateSession(FUniqueNetIdAccelByteUser::Invalid().Get(), SessionName, NewSessionSettings);
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to create session as our AccelByte subsystem is invalid"));
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session as our identity interface is invalid!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(HostingPlayerNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session as we could not find a valid player ID for index %d!"), HostingPlayerNum);
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Passing to create session with player '%s'!"), *PlayerId->ToDebugString());
	return CreateSession(PlayerId.ToSharedRef().Get(), SessionName, NewSessionSettings);
}

bool FOnlineSessionV2AccelByte::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToDebugString(), *SessionName.ToString());
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to create session as our AccelByte subsystem is invalid"));
		return false;
	}

	// Check if we already have a session with this name, and if so bail with warning
	if (GetNamedSession(SessionName) != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create session with name '%s' as a session with that name already exists!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
		return false;
	}

	FString SessionTypeString = TEXT("");
	if (!NewSessionSettings.Get(SETTING_SESSION_TYPE, SessionTypeString))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create new session as the SETTING_SESSION_TYPE was blank! Needs to be either SETTING_SESSION_TYPE_PARTY_SESSION or SETTING_SESSION_TYPE_GAME_SESSION!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(NewSessionSettings);
	if (SessionType == EAccelByteV2SessionType::Unknown)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create new session as the SETTING_SESSION_TYPE was blank or set to an unsupported value! Needs to be either SETTING_SESSION_TYPE_PARTY_SESSION or SETTING_SESSION_TYPE_GAME_SESSION!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
		return false;
	}

	if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Creating a new party session!"));
		return CreatePartySession(HostingPlayerId, SessionName, NewSessionSettings);
	}
	else if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Creating a new game session!"));
		return CreateGameSession(HostingPlayerId, SessionName, NewSessionSettings);
	}

	return false;
}

bool FOnlineSessionV2AccelByte::CreatePartySession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("HostingPlayerId: %s; SessionName: %s"), *HostingPlayerId.ToDebugString(), *SessionName.ToString());
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to create party session as our AccelByte subsystem is invalid"));
		return false;
	}

	// Create new session instance for this party that we are trying to create, and reflect state that we are creating
	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, NewSessionSettings);
	NewSession->SessionState = EOnlineSessionState::Creating;

	NewSession->bHosting = true;

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create party session as our identity interface is invalid!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
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

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateV2Party>(AccelByteSubsystemPtr.Get(), HostingPlayerId, SessionName, NewSessionSettings);
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::CreateGameSession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings, bool bSendCreateRequest)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to create game session as our AccelByte subsystem is invalid"));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game session as the Identityinterface is invalid!"));
		return false;
	}

	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();
	TOptional<bool> IsDS = AccelByteSubsystemPtr->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create game session as the IsDS is not set!"));
		return false;
	}

	FOnlineIdentityAccelBytePtr AccelByteIdentity = StaticCastSharedPtr<FOnlineIdentityAccelByte>(IdentityInterface);
	AccelByteIdentity->GetLocalUserNum(HostingPlayerId, LocalUserNum);

	// Create new session and update state
	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, NewSessionSettings);
	NewSession->SessionState = EOnlineSessionState::Creating;

	// Set number of open connections to the requested connection count from settings
	NewSession->NumOpenPublicConnections = NewSessionSettings.NumPublicConnections;
	NewSession->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
	
	// If we are sending a create request for this session, then that means that we know who is hosting already.
	// For servers, we want to disable this functionality even if we are sending a create request. This is because a server
	// client does not have a user ID, and thus cannot be set as the local owner or remote owner.
	if (bSendCreateRequest && !IsDS.GetValue())
	{
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
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateGameSessionV2>(AccelByteSubsystemPtr.Get()
			, HostingPlayerId
			, SessionName
			, NewSessionSettings
			, IsDS.GetValue());
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::FinalizeCreateGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& BackendSessionInfo)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *BackendSessionInfo.ID);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to finanlize game session as our AccelByte subsystem is invalid"));
		return;
	}

	FNamedOnlineSession* NewSession = GetNamedSession(SessionName);
	if (!ensure(NewSession != nullptr))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating game session as we do not have a session created for name '%s'!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
		return;
	}

	// Create new session info based off of the created session, start by filling session ID
	TSharedRef<FOnlineSessionInfoAccelByteV2> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV2>(BackendSessionInfo.ID);
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(BackendSessionInfo));
	SessionInfo->SetTeamAssignments(BackendSessionInfo.Teams);
	SessionInfo->SetSessionLeaderStorage(BackendSessionInfo.Storage.Leader);
	for(const auto& MemberStorages : BackendSessionInfo.Storage.Member)
	{
		const FUniqueNetIdAccelByteUserRef MemberUniqueNetId = FUniqueNetIdAccelByteUser::Create(MemberStorages.Key);
		SessionInfo->SetSessionMemberStorage(MemberUniqueNetId, MemberStorages.Value);
	}
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

	// Populate session code regardless of session joinablity since all session can be joined by code
	NewSession->SessionSettings.Set(SETTING_SESSION_CODE, BackendSessionInfo.Code);
	
	if (!IsRunningDedicatedServer())
	{
		if (!ensure(NewSession->LocalOwnerId.IsValid()))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating game session as the local owner of the session is invalid!"));
			DestroySession(SessionName);
			return;
		}

		const FUniqueNetIdRef LocalOwnerIdRef = NewSession->LocalOwnerId.ToSharedRef();
		
		// Register ourselves to the session
		RegisterPlayer(SessionName, LocalOwnerIdRef.Get(), false);
		if (SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P)
		{
			//attempt to set up P2P connection if we are creating P2P session
			SetupAccelByteP2PConnection(LocalOwnerIdRef.Get());
		}
		else if(SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::DS)
		{
			// setup session server status polling if notification doesn't arrive in a timely manner
			UE_LOG_AB(Log, TEXT("Trigger session server check poll from Finalize create game session"));
			StartSessionServerCheckPoll(LocalOwnerIdRef, SessionName);
		}
	}

	NewSession->SessionState = EOnlineSessionState::Pending;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::FinalizeCreatePartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& BackendSessionInfo)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *BackendSessionInfo.ID);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to finalize create party session as our AccelByte subsystem is invalid"));
		return;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!ensure(Session != nullptr))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating party session as we do not have a session created for name '%s'!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
		});
		
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
	Session->SessionSettings.Set(SETTING_SESSION_CODE, BackendSessionInfo.Code);

	// Make sure to also register ourselves to the session
	if (!ensure(Session->LocalOwnerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize creating party session as the local owner of the session is invalid!"));
		DestroySession(SessionName);
		return;
	}

	RegisterPlayer(SessionName, Session->LocalOwnerId.ToSharedRef().Get(), false);
	Session->SessionState = EOnlineSessionState::Pending;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineSessionV2AccelByte::ConstructGameSessionFromBackendSessionModel(const FAccelByteModelsV2GameSession& BackendSession, FOnlineSession& OutResult)
{
	// First, read attributes from backend model into session settings
	if (BackendSession.Attributes.JsonObject.IsValid())
	{
		ReadSessionSettingsFromSessionModel(OutResult.SessionSettings, BackendSession);
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
	SessionInfo->SetSessionLeaderStorage(BackendSession.Storage.Leader);
	for(const auto& MemberStorages : BackendSession.Storage.Member)
	{
		const FUniqueNetIdAccelByteUserRef MemberUniqueNetId = FUniqueNetIdAccelByteUser::Create(MemberStorages.Key);
		SessionInfo->SetSessionMemberStorage(MemberUniqueNetId, MemberStorages.Value);
	}

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
		ReadSessionSettingsFromSessionModel(OutResult.SessionSettings, BackendSession);
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
	OutSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, static_cast<int32>(GameSession.Configuration.MinPlayers));
	OutSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, static_cast<int32>(GameSession.Configuration.InviteTimeout));
	OutSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, static_cast<int32>(GameSession.Configuration.InactiveTimeout));
	OutSettings.Set(SETTING_GAMESESSION_CLIENTVERSION, GameSession.Configuration.ClientVersion);
	OutSettings.Set(SETTING_GAMESESSION_DEPLOYMENT, GameSession.Configuration.Deployment);
	OutSettings.Set(SETTING_SESSION_CODE, GameSession.Code);

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
	OutSettings.Set(SETTING_SESSION_MINIMUM_PLAYERS, static_cast<int32>(PartySession.Configuration.MinPlayers));
	OutSettings.Set(SETTING_SESSION_INACTIVE_TIMEOUT, static_cast<int32>(PartySession.Configuration.InactiveTimeout));
	OutSettings.Set(SETTING_SESSION_INVITE_TIMEOUT, static_cast<int32>(PartySession.Configuration.InviteTimeout));
	OutSettings.Set(SETTING_PARTYSESSION_CODE, PartySession.Code);
	OutSettings.Set(SETTING_SESSION_CODE, PartySession.Code);
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
		FieldName == SETTING_SESSION_CODE ||
		FieldName == SETTING_MATCHMAKING_BACKFILL_TICKET_ID;
}

bool FOnlineSessionV2AccelByte::GetServerLocalIp(FString& OutIp) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// First, check if we have an override IP set on the interface, if so always use that.
	if (!LocalServerIpOverride.IsEmpty())
	{
		OutIp = LocalServerIpOverride;
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("OutIp: %s"), *OutIp);
		return true;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (SocketSubsystem == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get local IP for server as our socket subsystem is invalid!"));
		return false;
	}

	bool bCanBindAll;
	TSharedPtr<FInternetAddr> LocalIP = SocketSubsystem->GetLocalHostAddr(*GLog, bCanBindAll);
	if (!LocalIP.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get local IP for server as our local host address from our socket subsystem is invalid!"));
		return false;
	}

	OutIp = LocalIP->ToString(false);

	// To handle a specified serverIP provided through localds commandline argument
	// i.e. "-serverip=192.168.x.x" 
	// If args not found, then the value won't be modified
	FAccelByteUtilities::GetValueFromCommandLineSwitch(ACCELBYTE_ARGS_SERVERIP, OutIp);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("OutIp: %s"), *OutIp);
	return true;
}

bool FOnlineSessionV2AccelByte::GetServerPort(int32& OutPort, bool bIsLocal) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// First, check if we have an override port set on the interface, if so always use that.
	if (bIsLocal && LocalServerPortOverride > 0)
	{
		OutPort = LocalServerPortOverride;
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("OutPort: %d"), OutPort);
		return true;
	}

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get port for server as the current game instance is not a dedicated server!"));
		return false;
	}

	if (GEngine == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get port for server as our engine singleton instance was invalid!"));
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
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get port for server as our world instance was invalid!"));
		return false;
	}

	OutPort = World->URL.Port;
#else
	OutPort = 7777; // #TODO(Maxwell): Figure out a better way to get bound port if not 4.26...
#endif

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("OutPort: %d"), OutPort);
	return true;
}

bool FOnlineSessionV2AccelByte::GetLocalServerName(FString& OutServerName) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// First, check if we have an override name set on the interface, if so always use that.
	if (!LocalServerNameOverride.IsEmpty())
	{
		OutServerName = LocalServerNameOverride;
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("OutServerName: %s"), *OutServerName);
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
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find suitable server name from environment, command line, or computer name!"));
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("OutServerName: %s"), *OutServerName);
	return true;
}

bool FOnlineSessionV2AccelByte::GetServerClaimedSession(FName SessionName, const FString& SessionId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to get server claimed for a session as our AccelByte subsystem is invalid"));
		return false;
	}

	// First, check if we already have a game session created for us
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to create local session for associated dedicated server session as we already locally have a game session instance!"));
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

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetServerClaimedV2Session>(AccelByteSubsystemPtr.Get(), SessionName, SessionId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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

bool FOnlineSessionV2AccelByte::LeaveSession(const FUniqueNetId& LocalUserId, const EAccelByteV2SessionType& SessionType, const FString& SessionId, const FOnLeaveSessionComplete& Delegate, bool bUserKicked)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionId: %s"), *LocalUserId.ToDebugString(), *SessionId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to leave session as our AccelByte subsystem is invalid"));
		return false;
	}

	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLeaveV2GameSession>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionId, Delegate, bUserKicked);
	
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending request to leave game session!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLeaveV2Party>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionId, Delegate, bUserKicked);
		
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending request to leave party session!"));
		return true;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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

bool FOnlineSessionV2AccelByte::ReadSessionSettingsFromSessionModel(FOnlineSessionSettings& OutSettings, const FAccelByteModelsV2BaseSession& Session) const
{
	// Start off by going through each session member and adding a default session settings object if they do not have one
	for (const FAccelByteModelsV2SessionUser& Member : Session.Members)
	{
		FAccelByteUniqueIdComposite MemberCompositeId{};
		MemberCompositeId.Id = Member.ID;
		MemberCompositeId.PlatformType = Member.PlatformID;
		MemberCompositeId.PlatformId = Member.PlatformUserID;
		FUniqueNetIdAccelByteUserRef MemberUniqueId = FUniqueNetIdAccelByteUser::Create(MemberCompositeId);
		
		TSharedPtr<FSessionSettings> FoundMemberSettings;
		bool bSettingsFound = FindPlayerMemberSettings(OutSettings, MemberUniqueId.Get(), FoundMemberSettings);
		if (!bSettingsFound)
		{
			OutSettings.MemberSettings.Add(MemberUniqueId, FSessionSettings());
		}
	}

	// Now, go through the attributes object and load attributes into session settings
	TSharedRef<FJsonObject> OriginalObject = Session.Attributes.JsonObject.ToSharedRef();
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Attribute : OriginalObject->Values)
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
			if (!OriginalObject->TryGetStringField(Attribute.Key, StringValue))
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
			if (!OriginalObject->TryGetBoolField(Attribute.Key, BoolValue))
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
			if (!OriginalObject->TryGetNumberField(Attribute.Key, NumberValue))
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
			if (!OriginalObject->TryGetArrayField(Attribute.Key, ArrayValue))
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
				if (!ConvertSessionSettingJsonToArray(ArrayValue, AttributeValue))
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
			if (!OriginalObject->TryGetObjectField(Attribute.Key, JsonObjectValue))
			{
				UE_LOG_AB(Warning, TEXT("Failed to read session attribute '%s' as an object, skipping!"), *Attribute.Key);
				continue;
			}
			if (Attribute.Key.Len() == ACCELBYTE_ID_LENGTH)
			{
				const FAccelByteModelsV2SessionUser* FoundMember = Session.Members.FindByPredicate([Attribute](const FAccelByteModelsV2SessionUser& Member) {
					return Member.ID.Equals(Attribute.Key, ESearchCase::IgnoreCase);
				});

				if (FoundMember != nullptr)
				{
					FAccelByteUniqueIdComposite MemberCompositeId{};
					MemberCompositeId.Id = FoundMember->ID;
					MemberCompositeId.PlatformType = FoundMember->PlatformID;
					MemberCompositeId.PlatformId = FoundMember->PlatformUserID;
					FUniqueNetIdAccelByteUserRef MemberUniqueId = FUniqueNetIdAccelByteUser::Create(MemberCompositeId);

					// Populate settings from the backend into the found member settings for the player
					FSessionSettings* FoundMemberSettings = OutSettings.MemberSettings.Find(MemberUniqueId);
					if (ensureAlways(FoundMemberSettings != nullptr))
					{
						ReadMemberSettingsFromJsonObject(*FoundMemberSettings, (*JsonObjectValue).ToSharedRef());
					}
					
					break;
				}
			}
				
			// if attribute key is not a member, then add it to session setting.
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
	return true;
}

bool FOnlineSessionV2AccelByte::ReadMemberSettingsFromJsonObject(FSessionSettings& OutSettings, const TSharedRef<FJsonObject>& Object) const
{

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
				UE_LOG_AB(Warning, TEXT("Failed to read member setting attribute '%s' as a string, skipping!"), *Attribute.Key);
				continue;
			}
			OutSettings.Add(FName(Attribute.Key), FOnlineSessionSetting(StringValue));
			break;
		}
		case EJson::Boolean:
		{
			bool BoolValue;
			if (!Object->TryGetBoolField(Attribute.Key, BoolValue))
			{
				UE_LOG_AB(Warning, TEXT("Failed to read member setting attribute '%s' as a bool, skipping!"), *Attribute.Key);
				continue;
			}
			OutSettings.Add(FName(Attribute.Key), FOnlineSessionSetting(BoolValue));
			break;
		}
		case EJson::Number:
		{
			double NumberValue;
			if (!Object->TryGetNumberField(Attribute.Key, NumberValue))
			{
				UE_LOG_AB(Warning, TEXT("Failed to read member setting attribute '%s' as a number, skipping!"), *Attribute.Key);
				continue;
			}
			OutSettings.Add(FName(Attribute.Key), FOnlineSessionSetting(NumberValue));
			break;
		}
		case EJson::Array:
		{
			UE_LOG_AB(Warning, TEXT("Array type used with key '%s' not supported for member settings"), *Attribute.Key);
			break;
		}
		case EJson::Object:
		{
			const TSharedPtr<FJsonObject>* JsonObjectValue;
			if (!Object->TryGetObjectField(Attribute.Key, JsonObjectValue))
			{
				UE_LOG_AB(Warning, TEXT("Failed to read member setting attribute '%s' as an object, skipping!"), *Attribute.Key);
				continue;
			}

			OutSettings.Add(FName(Attribute.Key), FOnlineSessionSetting((*JsonObjectValue).ToSharedRef()));
			break;
		}
		default:
		{
			UE_LOG_AB(Warning, TEXT("Failed to read member setting attribute '%s' as variant data, skipping!"), *Attribute.Key);
			continue;
		}
		}
	}
	return true;
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

		if (Param.Key == SETTING_GAMESESSION_CROSSPLATFORM)
		{
			// Cross platform is a special key in the match ticket attributes. So we need to send it as a snake case field with
			// no type suffix.
			Param.Value.Data.AddToJsonObject(OutObject, TEXT("cross_platform"), false);
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!ensure(Session != nullptr))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as our local session instance is invalid!"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to connect to joined p2p session as our AccelByte subsystem is invalid"));
		return;
	}

	// Attempt to register our socket subsystem as default
	FAccelByteNetworkUtilitiesModule::Get().RegisterDefaultSocketSubsystem();

	// Now, grab the API client for the user that is setting up P2P and run set up in the module
	if (!ensure(Session->LocalOwnerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as the session's local owner is invalid!"));
		return;
	}

	const AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(Session->LocalOwnerId.ToSharedRef().Get());
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as the session's local owner is invalid!"));
		return;
	}

	FAccelByteNetworkUtilitiesModule::Get().Setup(ApiClient);

	// Register a delegate so that we can fire our join session complete delegate when connection finishes
	const FAccelByteNetworkUtilitiesModule::OnICERequestConnectFinished OnICEConnectionCompleteDelegate =
		FAccelByteNetworkUtilitiesModule::OnICERequestConnectFinished::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnICEConnectionComplete, SessionName, Action);
	FAccelByteNetworkUtilitiesModule::Get().RegisterICERequestConnectFinishedDelegate(OnICEConnectionCompleteDelegate);

	// Request connection to the peer
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to connect to P2P socket for session as the session's information instance is invalid!"));
		return;
	}

	const int32 P2PChannel = FMath::RandRange(1024, 4096);
	FAccelByteNetworkUtilitiesModule::Get().RequestConnect(FString::Printf (TEXT("%s:%d"), *SessionInfo->GetPeerId(), P2PChannel));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::UpdateInternalGameSession(const FName& SessionName
	, const FAccelByteModelsV2GameSession& UpdatedGameSession
	, bool& bIsConnectingToP2P
	, const bool bIsFirstJoin
	, const bool bUpdateSessionStorages)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' does not exist locally!"), *SessionName.ToString());
		return;
	}

	const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::GameSession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally is not a game session!"), *SessionName.ToString());
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally does not have valid session info!"), *SessionName.ToString());
		return;
	}

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!SessionData.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally does not have valid session backend data!"), *SessionName.ToString());
		return;
	}

	const TArray<FAccelByteModelsV2SessionUser> OldMembers = SessionData->Members;	
	const EAccelByteV2SessionConfigurationServerType OldServerType = SessionInfo->GetServerType();

	// First update the session data associated with the session info structure
	// TODO: Potentially unnecessary memory allocation
	bool bHasInvitedPlayersChanged = false;
	SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2GameSession>(UpdatedGameSession), bHasInvitedPlayersChanged);
	SessionInfo->SetTeamAssignments(UpdatedGameSession.Teams);

	if(bUpdateSessionStorages)
	{
		SessionInfo->SetSessionLeaderStorage(UpdatedGameSession.Storage.Leader);
		for(const auto& MemberStorages : UpdatedGameSession.Storage.Member)
		{
			const FUniqueNetIdAccelByteUserRef MemberUniqueNetId = FUniqueNetIdAccelByteUser::Create(MemberStorages.Key);
			SessionInfo->SetSessionMemberStorage(MemberUniqueNetId, MemberStorages.Value);
		}
	}

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
		ReadSessionSettingsFromSessionModel(Session->SessionSettings, UpdatedGameSession);
	}

	// After loading in custom attributes, load in reserved/built-in settings
	AddBuiltInGameSessionSettingsToSessionSettings(Session->SessionSettings, UpdatedGameSession);
	SetSessionMaxPlayerCount(Session, UpdatedGameSession.Configuration.MaxPlayers);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::UpdateInternalPartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& UpdatedPartySession)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for party session as the session named '%s' does not exist locally!"), *SessionName.ToString());
		return;
	}

	const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for party session as the session named '%s' locally is not a party session!"), *SessionName.ToString());
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for party session as the session named '%s' locally does not have valid session info!"), *SessionName.ToString());
		return;
	}

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!SessionData.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed update internal data for game session as the session named '%s' locally does not have valid session backend data!"), *SessionName.ToString());
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
		ReadSessionSettingsFromSessionModel(Session->SessionSettings, UpdatedPartySession);
	}

	// After reading attributes, reload reserved/built-in settings for the session
	AddBuiltInPartySessionSettingsToSessionSettings(Session->SessionSettings, UpdatedPartySession);
	SetSessionMaxPlayerCount(Session, UpdatedPartySession.Configuration.MaxPlayers);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineSessionV2AccelByte::StartSession(FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to start session as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to start session as the session does not exist locally!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// If the session is not waiting to start or already ended (meaning we can restart) then error out
	if (Session->SessionState != EOnlineSessionState::Pending && Session->SessionState != EOnlineSessionState::Ended)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to start session in state '%s'! Session needs to be in 'Pending' or 'Ended' state!"), EOnlineSessionState::ToString(Session->SessionState));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// #NOTE #SESSIONv2 There is a 'Starting' state before this 'InProgress' state - intended to allow for sending data
	// to backend for starting session. Maybe want to supply some kind of handler here to allow developer to make
	// those calls?
	Session->SessionState = EOnlineSessionState::InProgress;

	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
		SessionInterface->TriggerOnStartSessionCompleteDelegates(SessionName, true);
	});

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystemPtr->GetVoiceInterface());
		if (VoiceInterface.IsValid())
		{
			VoiceInterface->RemoveAllTalkers();
		}
	}
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData /*= true*/)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; bShouldRefreshOnlineData: %s"), *SessionName.ToString(), LOG_BOOL_FORMAT(bShouldRefreshOnlineData));
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to update session as our AccelByte subsystem is invalid"));
		return false;
	}
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session with name '%s' as it does not exist!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, false);
			SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, false);
		});
		return false;
	}

	FString UpdatedSessionTypeStr{};
	if (!UpdatedSessionSettings.Get(SETTING_SESSION_TYPE, UpdatedSessionTypeStr) || UpdatedSessionTypeStr.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update session settings as 'SETTING_SESSION_TYPE' was not set in the new settings object! Use GetSessionSettings(SessionName) to update an existing session's settings."), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, false);
			SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, false);
		});
		return false;
	}

	Session->SessionSettings = UpdatedSessionSettings;
	
	// If we don't need to refresh our session data on the backend, just bail here
	if (!bShouldRefreshOnlineData)
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
			SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, true);
		});
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Skipping updating session settings on backend as bShouldRefreshOnlineData is marked false!"));
		return false;
	}

	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();
	TOptional<bool> IsDS = AccelByteSubsystemPtr->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
			SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, true);
		});

		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update session as IsDS is not set!"));
		return false;
	}

	auto IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
			SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, true);
		});

		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update session as the IdentityInterface is not valid!"));
		return false;
	}
	
	if(!IsDS.GetValue() && Session->LocalOwnerId.IsValid())
	{
		auto LocalUserId = FUniqueNetIdAccelByteUser::CastChecked(Session->LocalOwnerId.ToSharedRef());
		if (LocalUserId->IsValid())
		{
			IdentityInterface->GetLocalUserNum(LocalUserId.Get(), LocalUserNum);
		}
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), *Session->LocalOwnerId, true);
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteUpdateGameSessionV2>(AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, SessionName
			, UpdatedSessionSettings
			, IsDS.GetValue());
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		if (IsDS.GetValue())
		{
			AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
				SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
				SessionInterface->TriggerOnSessionUpdateRequestCompleteDelegates(SessionName, true);
			});
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Game servers are not able to update party sessions!"));
			return false;
		}

		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteUpdatePartyV2>(AccelByteSubsystemPtr.Get(), SessionName, UpdatedSessionSettings);
	}
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created async task to update session data on backend!"));
	return true;
}

bool FOnlineSessionV2AccelByte::EndSession(FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to end session as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to end session as the session does not exist locally!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// If the session is not in progress, then don't allow us to end it!
	if (Session->SessionState != EOnlineSessionState::InProgress)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to end session in state '%s'! Session needs to be in an 'InProgress' state!"), EOnlineSessionState::ToString(Session->SessionState));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
			SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	// #NOTE #SESSIONv2 There is also an 'Ending' before this one state for committing stats and other data to the backend.
	// Maybe a future improvement to have some kind of mechanism for allowing the developer to commit end game info before
	// fully ending session?
	Session->SessionState = EOnlineSessionState::Ended;

	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName]() {
		SessionInterface->TriggerOnEndSessionCompleteDelegates(SessionName, true);
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate)
{
	return DestroySession(SessionName, CompletionDelegate, false);
}

bool FOnlineSessionV2AccelByte::DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate , bool bUserKicked)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to destroy session as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session as the session does not exist locally!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName, CompletionDelegate]() {
			CompletionDelegate.ExecuteIfBound(SessionName, false);
			SessionInterface->TriggerOnDestroySessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session as our session info instance is invalid!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName, CompletionDelegate]()
		{
			CompletionDelegate.ExecuteIfBound(SessionName, false);
			SessionInterface->TriggerOnDestroySessionCompleteDelegates(SessionName, false);
		});
		
		return false;
	}

	if (SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::P2P)
	{
		TeardownAccelByteP2PConnection();
	}

	// Bail if we are already destroying session!
	if (Session->SessionState == EOnlineSessionState::Destroying)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to destroy session already in the process of being destroyed!"));
		// Purposefully not calling delegate here, as we are still awaiting the previous destroy call to execute delegate
		return false;
	}
	else if(Session->SessionState == EOnlineSessionState::Creating)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session as our session is still creating!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName, CompletionDelegate]()
		{
			CompletionDelegate.ExecuteIfBound(SessionName, false);
			SessionInterface->TriggerOnDestroySessionCompleteDelegates(SessionName, false);
		});

		return false;
	}

	Session->SessionState = EOnlineSessionState::Destroying;

	// Get owner log in status to determine if we are able to issue a 'LeaveSession' call for this session
	FOnlineIdentityAccelBytePtr IdentityInterface{};
	ELoginStatus::Type LoginStatus = ELoginStatus::NotLoggedIn;
	if (Session->LocalOwnerId.IsValid() && FOnlineIdentityAccelByte::GetFromSubsystem(AccelByteSubsystemPtr.Get(), IdentityInterface))
	{
		LoginStatus = IdentityInterface->GetLoginStatus(Session->LocalOwnerId.ToSharedRef().Get());
	}

	if (!IsRunningDedicatedServer() && LoginStatus == ELoginStatus::LoggedIn)
	{
		// Grab local owner ID so that we can make a call to leave session on their behalf
		if (!ensure(Session->LocalOwnerId.IsValid()))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to destroy session as our session has no local owner ID!"));
			return false;
		}

		const FUniqueNetIdRef SessionOwnerId = Session->LocalOwnerId.ToSharedRef();

		// Fire off call to leave session. Appropriate delegates will be fired by this method, along with clean up of sessions.
		EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);

		const FOnLeaveSessionComplete OnLeaveSessionCompleteDelegate = FOnLeaveSessionComplete::CreateLambda([CompletionDelegate, SessionName](bool bWasSuccessful, FString SessionId) {
			CompletionDelegate.ExecuteIfBound(SessionName, bWasSuccessful);
		});
		LeaveSession(SessionOwnerId.Get(), SessionType, Session->GetSessionIdStr(), OnLeaveSessionCompleteDelegate, bUserKicked);
		
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending request for player '%s' to leave session!"), *SessionOwnerId->ToDebugString());
	}
	else
	{
		// If this is a server or the owning player is not logged in, just remove the session from the interface and move on
		RemoveNamedSession(SessionName);
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName, CompletionDelegate]() {
			CompletionDelegate.ExecuteIfBound(SessionName, true);
			SessionInterface->TriggerOnDestroySessionCompleteDelegates(SessionName, true);
		});
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	}

	FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystemPtr->GetVoiceInterface());
	if (VoiceInterface.IsValid())
	{
		VoiceInterface->RemoveAllTalkers();
	}

	return true;
}

FServerApiClientPtr FOnlineSessionV2AccelByte::GetServerApiClient() const
{
	FOnlineSubsystemAccelBytePtr SubsystemPtr = AccelByteSubsystem.Pin();
	if(!SubsystemPtr.IsValid())
	{
		return nullptr;
	}
	

	FAccelByteInstancePtr AccelByteInstancePtr = SubsystemPtr->GetAccelByteInstance().Pin();
	if(!AccelByteInstancePtr.IsValid())
	{
		return nullptr;
	}

	return AccelByteInstancePtr->GetServerApiClient();
}

bool FOnlineSessionV2AccelByte::IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find player in session '%s' as the session provided doesn't exist!"), *SessionName.ToString());
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
	TSharedRef<FOnlineSessionSearchAccelByte> SessionSearchAccelByteRef = MakeShared<FOnlineSessionSearchAccelByte>(SearchSettings);
	SearchSettings = SessionSearchAccelByteRef;
	return StartMatchmaking(LocalPlayers, SessionName, NewSessionSettings, SessionSearchAccelByteRef, CompletionDelegate);
}

bool FOnlineSessionV2AccelByte::StartMatchmaking(const TArray<FSessionMatchmakingUser>& LocalPlayers, FName SessionName,
	const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearchAccelByte>& SearchSettings,
	const FOnStartMatchmakingComplete& CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s; SessionName: %s"), ((LocalPlayers.IsValidIndex(0)) ? *LocalPlayers[0].UserId->ToDebugString() : TEXT("")), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed start matchmaking as our AccelByte subsystem is invalid"));
		return false;
	}

	// Fail the matchmaking request if we are already matchmaking currently
	if (SearchSettings->SearchState != EOnlineAsyncTaskState::NotStarted || (CurrentMatchmakingSearchHandle.IsValid() && CurrentMatchmakingSearchHandle->SearchState != EOnlineAsyncTaskState::NotStarted))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start matchmaking as we are already currently matchmaking!"));
		AccelByteSubsystemPtr->ExecuteNextTick([CompletionDelegate, SessionName]() {
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
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot start matchmaking without specifying at least one local player!"));
		AccelByteSubsystemPtr->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), EmptyResults);
		});
		return false;
	}

	if (LocalPlayers.Num() > 1)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("StartMatchmaking does not support matchmaking with more than one local user!"));
		AccelByteSubsystemPtr->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), EmptyResults);
		});
		return false;
	}

	FString MatchPool{};
	if (!SearchSettings->QuerySettings.Get(SETTING_SESSION_MATCHPOOL, MatchPool) || MatchPool.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("No match pool was set in the session search settings! In order to start matchmaking, you must specify what match pool to search by setting SETTING_SESSION_MATCHPOOL in the SearchSettings passed in!"));
		AccelByteSubsystemPtr->ExecuteNextTick([CompletionDelegate, SessionName]() {
			FSessionMatchmakingResults EmptyResults{};
			CompletionDelegate.ExecuteIfBound(SessionName, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), EmptyResults);
		});
		return false;
	}

	// Convert our session search handle into an AccelByte subclass so that we can store ticket ID
	CurrentMatchmakingSearchHandle = SearchSettings;
	CurrentMatchmakingSearchHandle->SearchingPlayerId = LocalPlayers[0].UserId;
	CurrentMatchmakingSearchHandle->SearchingSessionName = SessionName;
	CurrentMatchmakingSearchHandle->MatchPool = MatchPool;

	if (SearchSettings->GetSearchStorage().IsValid())
	{
		CurrentMatchmakingSearchHandle->SetSearchStorage(SearchSettings->GetSearchStorage());
	}

	// #NOTE Previously, we would set search state to InProgress in the FOnlineAsyncTaskAccelByteStartV2Matchmaking::Initialize
	// method. However, this runs the risk of allowing a second StartMatchmaking call to come in before that method runs
	// and cause issues. Instead, set to InProgress here to prevent duplicate matchmaking start calls.
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;
	
	CurrentMatchmakingSessionSettings = NewSessionSettings;

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), *CurrentMatchmakingSearchHandle->SearchingPlayerId.Get(), true);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteStartV2Matchmaking>(AccelByteSubsystemPtr.Get(), CurrentMatchmakingSearchHandle.ToSharedRef(), SessionName, MatchPool, CompletionDelegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to cancel matchmaking as our AccelByte subsystem is invalid"));
		return false;
	}
	
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking as our identity interface is invalid!"));
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
		});
		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel matchmaking as we could not get a unique ID for player at index %d!"), SearchingPlayerNum);
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
		});
		return false;
	}

	return CancelMatchmaking(PlayerId.ToSharedRef().Get(), SessionName);
}

bool FOnlineSessionV2AccelByte::CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; SessionName: %s"), *SearchingPlayerId.ToDebugString(), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to cancel matchmaking as our AccelByte subsystem is invalid"));
		return false;
	}

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("No matchmaking search handle attached to the session interface at this time. Treating the cancel request as a success to allow game client to clean up local state if stuck."));
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, true);
		});
		return true;
	}

	if (CurrentMatchmakingSearchHandle->SearchState != EOnlineAsyncTaskState::InProgress)
	{
		CurrentMatchmakingSearchHandle.Reset();
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Matchmaking search handle attached to session interface is not marked as in progress. Treating the cancel request as a success to allow game client to clean up local state if stuck."));
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, true);
		});
		return true;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCancelV2Matchmaking>(AccelByteSubsystemPtr.Get(), CurrentMatchmakingSearchHandle.ToSharedRef(), SessionName);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to find session as our AccelByte subsystem is invalid"));
		return false;
	}
	
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find sessions as our identity interface is invalid!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this)]() {
			SessionInterface->TriggerOnFindSessionsCompleteDelegates(false);
		});
		
		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find sessions as we could not find a unique ID for player at index %d!"), SearchingPlayerNum);

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this)]() {
			SessionInterface->TriggerOnFindSessionsCompleteDelegates(false);
		});
		
		return false;
	}

	return FindSessions(PlayerId.ToSharedRef().Get(), SearchSettings);
}

bool FOnlineSessionV2AccelByte::FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s"), *SearchingPlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to find session as our AccelByte subsystem is invalid"));
		return false;
	}

	FAccelByteModelsV2GameSessionQueryFilter QueryFilter{};
	//Maintain default behavior
	QueryFilter.GameSessionAvailability = EAccelByteV2QueryGameSessionAvailability::AvailableSessionOnly;

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindGameSessionsV2>(AccelByteSubsystemPtr.Get(), SearchingPlayerId, SearchSettings, QueryFilter);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindGameSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings, FAccelByteModelsV2GameSessionQueryFilter QueryFilter)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(VeryVerbose, TEXT("Failed to find session as our AccelByte subsystem is invalid"));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find sessions as our identity interface is invalid!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this)]() {
			SessionInterface->TriggerOnFindSessionsCompleteDelegates(false);
			});

		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(SearchingPlayerNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find sessions as we could not find a unique ID for player at index %d!"), SearchingPlayerNum);

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this)]() {
			SessionInterface->TriggerOnFindSessionsCompleteDelegates(false);
			});

		return false;
	}

	return FindGameSessions(PlayerId.ToSharedRef().Get(), SearchSettings, QueryFilter);
}

bool FOnlineSessionV2AccelByte::FindGameSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings, FAccelByteModelsV2GameSessionQueryFilter QueryFilter)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s"), *SearchingPlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to find session as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindGameSessionsV2>(AccelByteSubsystemPtr.Get(), SearchingPlayerId, SearchSettings, QueryFilter);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingPlayerId: %s; SessionId: %s"), *SearchingUserId.ToDebugString(), *SessionId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to find session by id as our AccelByte subsystem is invalid"));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as the Identityinterface is invalid!"));
		return false;
	}

	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();

	FOnlineIdentityAccelBytePtr AccelByteIdentity = StaticCastSharedPtr<FOnlineIdentityAccelByte>(IdentityInterface);
	AccelByteIdentity->GetLocalUserNum(SearchingUserId, LocalUserNum);

	TOptional<bool> IsDS = AccelByteSubsystemPtr->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as the Identityinterface is invalid!"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get()
		, SearchingUserId
		, SessionId
		, CompletionDelegate
		, nullptr
		, IsDS.GetValue());

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off async task to find session with ID '%s'!"), *SessionId.ToDebugString());
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to join session as our AccelByte subsystem is invalid"));
		return false;
	}
	
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		UE_LOG_AB(Warning, TEXT("Failed to join session as our identity interface is invalid!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		});
		
		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		UE_LOG_AB(Warning, TEXT("Failed to join session as we could not get a unique ID for player at index %d!"), LocalUserNum);
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		});
		
		return false;
	}

	return JoinSession(PlayerId.ToSharedRef().Get(), SessionName, DesiredSession);
}

bool FOnlineSessionV2AccelByte::JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to join session as our AccelByte subsystem is invalid"));
		return false;
	}

	if (GetNamedSession(SessionName) != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session with name '%s' as a session with that name already exists!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
		});

		return false;
	}

	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, DesiredSession.Session);
	NewSession->SessionState = EOnlineSessionState::Creating;
	NewSession->LocalOwnerId = LocalUserId.AsShared();

	// Check if the local player attempting to join the session is already marked as joined on the backend. This will
	// skip the subsequent join session call in the async task if true.
	bool bIsLocalUserJoined { false };
	if (NewSession->SessionInfo.IsValid())
	{
		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(NewSession->SessionInfo);
		if (SessionInfo.IsValid())
		{
			FAccelByteModelsV2SessionUser* FoundMember = nullptr;
			const bool bHasLocalUser = SessionInfo->FindMember(LocalUserId, FoundMember);
			if (bHasLocalUser)
			{
				bIsLocalUserJoined = FoundMember->StatusV2 == EAccelByteV2SessionMemberStatus::JOINED
					|| FoundMember->StatusV2 == EAccelByteV2SessionMemberStatus::CONNECTED;
			}
		}
	}

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

	if (!IsRunningDedicatedServer())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), LocalUserId, true);
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(NewSession->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteJoinV2GameSession>(AccelByteSubsystemPtr.Get()
			, LocalUserId
			, SessionName
			, bIsLocalUserJoined);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Spawning async task to join game session on backend!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteJoinV2Party>(AccelByteSubsystemPtr.Get()
			, LocalUserId
			, SessionName
			, bIsLocalUserJoined);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Spawning async task to join party session on backend!"));
		return true;
	}

	// #TODO #SESSIONv2 Not sure if for non-game and non-party sessions we also want to send a join request...
	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
		SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::Success);
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FString& Code, EAccelByteV2SessionType SessionType)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; SessionType: %s; PartyCode: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *FAccelByteUtilities::GetUEnumValueAsString(SessionType), *Code);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to join session as our AccelByte subsystem is invalid"));
		return false;
	}
	
	if(SessionType != EAccelByteV2SessionType::GameSession && SessionType != EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session with code, session type %s not supported to join with code"), *FAccelByteUtilities::GetUEnumValueAsString(SessionType));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::UnknownError);
		});

		return false;
	}
	
	// First, ensure that we are not going to clobber an existing session with the given name
	if (GetNamedSession(SessionName) != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join session with name '%s' as a session with that name already exists!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
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

	if (!IsRunningDedicatedServer())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), LocalUserId, true);
	}

	if(SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteJoinV2PartyByCode>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Code);
	}
	else if(SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteJoinV2GameSessionByCode>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Code);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::GenerateNewGameCode(const FUniqueNetId& LocalUserId, FName SessionName,
	const FOnGenerateNewGameCodeComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to generate game code as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate new code for party with session name '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false, TEXT(""));
		});
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::GameSession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate new party code for session with name '%s' as the session is not a game session!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false, TEXT(""));
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateNewV2GameCode>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::RevokeGameCode(const FUniqueNetId& LocalUserId, FName SessionName,
	const FOnRevokeGameCodeComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to revoke game code as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to revoke code for game with session name '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::GameSession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to revoke game code for session with name '%s' as the session is not a game session!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();
	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to revoke game code for session as the IdentityInterface is invalid!"));

		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});

		return false;
	}
	IdentityInterface->GetLocalUserNum(LocalUserId, LocalUserNum);
	TOptional<bool> IsDS = AccelByteSubsystemPtr->IsDedicatedServer(LocalUserNum);

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRevokeV2GameCode>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Delegate, IsDS.GetValue());

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::GenerateNewPartyCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnGenerateNewPartyCodeComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to generate new party code as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate new code for party with session name '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false, TEXT(""));
		});
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate new party code for session with name '%s' as the session is not a party session!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false, TEXT(""));
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateNewV2PartyCode>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::RevokePartyCode(const FUniqueNetId& LocalUserId, FName SessionName, const FOnRevokePartyCodeComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
    	if(!AccelByteSubsystemPtr.IsValid())
    	{
    		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to revoke party code as our AccelByte subsystem is invalid"));
    		return false;
    	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to revoke code for party with session name '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to revoke party code for session with name '%s' as the session is not a party session!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRevokeV2PartyCode>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
		OutAttributes.Roles = FoundAttributes->Roles;
		OutAttributes.Platforms = FoundAttributes->Platforms;
	}

	return OutAttributes;
}

bool FOnlineSessionV2AccelByte::UpdatePlayerAttributes(const FUniqueNetId& LocalPlayerId, const FOnlineSessionV2AccelBytePlayerAttributes& NewAttributes, const FOnUpdatePlayerAttributesComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s"), *LocalPlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to update party attribute as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdatePlayerAttributes>(AccelByteSubsystemPtr.Get(), LocalPlayerId, NewAttributes, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	if (!SessionInfo.IsValid())
	{
		return false;
	}

	const bool bIsP2PHost = (LocalUserId == SessionInfo->GetLeaderId().ToSharedRef().Get());
	return SessionInfo->IsP2PMatchmaking() && bIsP2PHost;
}

bool FOnlineSessionV2AccelByte::FindPlayerMemberSettings(FOnlineSessionSettings& InSettings, const FUniqueNetId& PlayerId, FSessionSettings& OutMemberSettings) const
{
	UE_LOG_AB(Warning, TEXT("This function is deprecated due to potential stability and reliability issues. use the new overloaded function instead."));
	// #NOTE Due to the way that our user IDs are set up, hashes may be different, such as if one ID has platform
	// information, and the other lacks that information. However, we mostly just care about matching on the player's
	// AccelByte ID anyway. With this in mind, iterate through the map entries and find the one that matches the correct
	// AccelByte ID, and return it to the player.
	FUniqueNetIdAccelByteUserRef AccelBytePlayerId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);
	for (const TPair<FUniqueNetIdRef, FSessionSettings>& MemberPair : InSettings.MemberSettings)
	{
		FUniqueNetIdAccelByteUserRef FoundMemberAccelByteId = FUniqueNetIdAccelByteUser::CastChecked(MemberPair.Key);
		if (AccelBytePlayerId->GetAccelByteId().Equals(FoundMemberAccelByteId->GetAccelByteId()))
		{
			OutMemberSettings = MemberPair.Value;
			return true;
		}
	}
	return false;
}

bool FOnlineSessionV2AccelByte::FindPlayerMemberSettings(FOnlineSessionSettings& InSettings, const FUniqueNetId& PlayerId, TSharedPtr<FSessionSettings>& OutMemberSettings) const
{
	// Cast the PlayerId to FUniqueNetIdAccelByteUser
	FUniqueNetIdAccelByteUserRef AccelBytePlayerId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);

	// Iterate through the member settings
	for (const TPair<FUniqueNetIdRef, FSessionSettings>& MemberPair : InSettings.MemberSettings)
	{
		FUniqueNetIdAccelByteUserRef FoundMemberAccelByteId = FUniqueNetIdAccelByteUser::CastChecked(MemberPair.Key);

		// Compare AccelByte IDs
		if (AccelBytePlayerId->GetAccelByteId().Equals(FoundMemberAccelByteId->GetAccelByteId()))
		{
			// Create a shared pointer to the found member settings
			OutMemberSettings = MakeShareable(new FSessionSettings(MemberPair.Value));
			return true;
		}
	}
	return false;
}

bool FOnlineSessionV2AccelByte::UpdateSessionLeaderStorage(const FUniqueNetId& LocalUserId, FName SessionName, FJsonObjectWrapper const& Data)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	if (IsRunningDedicatedServer())
	{
		TriggerOnUpdateSessionLeaderStorageCompleteDelegates(SessionName, FOnlineError::CreateError(TEXT("UpdateSessionLeaderStorage"), EOnlineErrorResult::RequestFailure));
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Dedicated server unable to update session leader storage"))
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to update session leader storage as our AccelByte subsystem is invalid"));
		return false;
	}

	const FNamedOnlineSession* SessionToUpdate = GetNamedSession(SessionName);
	if (SessionToUpdate == nullptr)
	{
		TriggerOnUpdateSessionLeaderStorageCompleteDelegates(SessionName, FOnlineError::CreateError(TEXT("UpdateSessionLeaderStorage"), EOnlineErrorResult::RequestFailure));
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to update session leader storage as current user is not in %s"), *SessionName.ToString())
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionToUpdate, Data);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
	return true;
}

bool FOnlineSessionV2AccelByte::UpdateSessionMemberStorage(const FUniqueNetId& LocalUserId, FName SessionName, FJsonObjectWrapper const& Data)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString());

	if (IsRunningDedicatedServer())
	{
		TriggerOnUpdateSessionMemberStorageCompleteDelegates(SessionName, LocalUserId, FOnlineError::CreateError(TEXT("UpdateSessionMemberStorage"), EOnlineErrorResult::RequestFailure));
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Dedicated server unable to update session leader storage"))
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to update session member storage as our AccelByte subsystem is invalid"));
		return false;
	}

	const FNamedOnlineSession* SessionToUpdate = GetNamedSession(SessionName);
	if (SessionToUpdate == nullptr)
	{
		TriggerOnUpdateSessionMemberStorageCompleteDelegates(SessionName, LocalUserId, FOnlineError::CreateError(TEXT("UpdateSessionMemberStorage"), EOnlineErrorResult::RequestFailure));
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to update session member storage as current user is not in %s"), *SessionName.ToString())
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionToUpdate, Data);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
	return true;
}

bool FOnlineSessionV2AccelByte::GetMyActiveMatchTicket(const FUniqueNetId& LocalUserId, FName SessionName, const FString& MatchPool)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s, MatchPool: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *MatchPool);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get my active match ticket as our AccelByte subsystem is invalid"));
		return false;
	}
	
	if(CurrentMatchmakingSearchHandle.IsValid() && CurrentMatchmakingSearchHandle->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot get active match ticket as we are already currently matchmaking!"));
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnGetMyActiveMatchTicketCompleteDelegates(false, SessionName, nullptr);
		});
		return false;
	}

	if(MatchPool.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot get active match ticket as the matchpool parameter is empty!"));
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), SessionName]() {
			SessionInterface->TriggerOnGetMyActiveMatchTicketCompleteDelegates(false, SessionName, nullptr);
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, MatchPool);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::SessionContainsMember(const FUniqueNetId& UserId, FName SessionName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, SessionName: %s"), *UserId.ToDebugString(), *SessionName.ToString());
	FNamedOnlineSession* Session = GetNamedSession(SessionName);

	if(Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Named session not found"));
		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> ABSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	return ABSessionInfo->ContainsMember(UserId);
}

bool FOnlineSessionV2AccelByte::CancelSessionInvite(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Invitee)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionName: %s; Friend: %s"), LocalUserNum, *SessionName.ToString(), *Invitee.ToDebugString());

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't cancel session invite from dedicated server"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to cancel session invite notif as our AccelByte subsystem is invalid"));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel session invite as our identity interface is invalid!"));
		// cannot trigger delegate here because we can't get the localUserId without Identity Interface
		
		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel session invite as we could not get a unique ID from player index %d!"), LocalUserNum);
		// cannot trigger delegate here because we can't get the localUserId in Identity Interface

		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Canceling invite for user '%s'!"), *Invitee.ToDebugString());
	
	return CancelSessionInvite(PlayerId.ToSharedRef().Get(), SessionName, Invitee);
}

bool FOnlineSessionV2AccelByte::CancelSessionInvite(const FUniqueNetId& LocalUserId, FName SessionName,	const FUniqueNetId& Invitee)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""))

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't cancel session invite from dedicated server"));
		return false;
	}
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to cancel session invite notif as our AccelByte subsystem is invalid"));
		return false;
	}

	const FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
    {
    	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel invitation to session as the session does not exist locally!"));

    	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), LocalUserId = LocalUserId.AsShared(), SessionName, Invitee = Invitee.AsShared()]()
    		{
				const FOnlineError Error = FOnlineError::CreateError(
					ONLINE_ERROR_NAMESPACE,
					EOnlineErrorResult::RequestFailure,
					FString::FromInt(static_cast<int32>(ErrorCodes::InvalidRequest)),
					FText::FromString(TEXT("User is not in any session.")));
    			SessionInterface->TriggerOnCancelSessionInviteCompleteDelegates(*LocalUserId, SessionName, *Invitee, Error);
    		});
    	
    	return false;
    }

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	switch (SessionType) {
	case EAccelByteV2SessionType::GameSession:
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCancelV2GameSessionInvite>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Invitee);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Canceling %s game session invite for user %s"), *SessionName.ToString(), *Invitee.ToDebugString())
		break;
	case EAccelByteV2SessionType::PartySession:
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCancelV2PartyInvite>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Invitee);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Canceling %s party invite for user %s"), *SessionName.ToString(), *Invitee.ToDebugString())
		break;
	case EAccelByteV2SessionType::Unknown:
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel invitation to session as the session type unknown!"));
		break;
	}

	return true;
}

void FOnlineSessionV2AccelByte::SetMatchTicketCheckEnabled(const bool Enabled)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetEnableMatchTicketPolling().SetValue(Enabled);
}

bool FOnlineSessionV2AccelByte::GetMatchTicketCheckEnabled() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning true as that is the default
		return true;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning true as that is the default
		return true;
	}

	return Config->GetEnableMatchTicketPolling().GetValue();
}

void FOnlineSessionV2AccelByte::SetMatchTicketCheckInitialDelay(const int32 Sec)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetMatchTicketPollStartDelaySeconds().SetValue(Sec);
}

int32 FOnlineSessionV2AccelByte::GetMatchTicketCheckInitialDelay() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	return Config->GetMatchTicketPollStartDelaySeconds().GetValue();
}

void FOnlineSessionV2AccelByte::SetMatchTicketCheckPollInterval(const int32 Sec)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetMatchTicketPollIntervalSeconds().SetValue(Sec);
}

int32 FOnlineSessionV2AccelByte::GetMatchTicketCheckPollInterval() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	return Config->GetMatchTicketPollIntervalSeconds().GetValue();
}

void FOnlineSessionV2AccelByte::SetSessionServerCheckPollEnabled(bool Enabled)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetEnableServerInfoPolling().SetValue(Enabled);
}

bool FOnlineSessionV2AccelByte::GetSessionServerCheckPollEnabled() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning true as that is the intended default
		return true;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning true as that is the intended default
		return true;
	}

	return Config->GetEnableServerInfoPolling().GetValue();
}

void FOnlineSessionV2AccelByte::SetSessionServerCheckPollInitialDelay(int32 Sec)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetServerInfoPollStartDelaySeconds().SetValue(Sec);
}

int32 FOnlineSessionV2AccelByte::GetSessionServerCheckPollInitialDelay() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	return Config->GetServerInfoPollStartDelaySeconds().GetValue();
}

void FOnlineSessionV2AccelByte::SetSessionServerCheckPollInterval(int32 Sec)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetServerInfoPollIntervalSeconds().SetValue(Sec);
}

int32 FOnlineSessionV2AccelByte::GetSessionServerCheckPollInterval() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning 15 as that is the intended default
		return 15;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning 15 as that is the intended default
		return 15;
	}

	return Config->GetServerInfoPollIntervalSeconds().GetValue();
}

void FOnlineSessionV2AccelByte::SetSessionInviteCheckPollEnabled(bool Enabled)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetEnableSessionInvitePolling().SetValue(Enabled);
}

bool FOnlineSessionV2AccelByte::GetSessionInviteCheckPollEnabled() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning true as that is the intended default
		return true;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning true as that is the intended default
		return true;
	}

	return Config->GetEnableSessionInvitePolling().GetValue();
}

void FOnlineSessionV2AccelByte::SetSessionInviteCheckPollInitialDelay(int32 Sec)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	Config->GetSessionInvitePollStartDelaySeconds().SetValue(Sec);
}

int32 FOnlineSessionV2AccelByte::GetSessionInviteCheckPollInitialDelay() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning 30 as that is the intended default
		return 30;
	}

	return Config->GetSessionInvitePollStartDelaySeconds().GetValue();
}

void FOnlineSessionV2AccelByte::SetSessionInviteCheckPollInterval(int32 Sec)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		return;
	}

	return Config->GetSessionInvitePollIntervalSeconds().SetValue(Sec);
}

int32 FOnlineSessionV2AccelByte::GetSessionInviteCheckPollInterval() const
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		// Returning 15 as that is the intended default
		return 15;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		// Returning 15 as that is the intended default
		return 15;
	}

	return Config->GetSessionInvitePollIntervalSeconds().GetValue();
}

bool FOnlineSessionV2AccelByte::FindSessionByStringId(const FUniqueNetId& SearchingUserId
	, const EAccelByteV2SessionType& SessionType
	, const FString& SessionId
	, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingUserId: %s; SessionType: %s; SessionId: %s")
		, *SearchingUserId.ToDebugString()
		, *FAccelByteUtilities::GetUEnumValueAsString(SessionType)
		, *SessionId);

	if (!SearchingUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as the UserId is invalid"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as the AccelByte subsystem is invalid"));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as the Identityinterface is invalid!"));
		return false;
	}

	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();

	FOnlineIdentityAccelBytePtr AccelByteIdentity = StaticCastSharedPtr<FOnlineIdentityAccelByte>(IdentityInterface);
	AccelByteIdentity->GetLocalUserNum(SearchingUserId, LocalUserNum);

	FindSessionByStringId(LocalUserNum, SessionType, SessionId, CompletionDelegate);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Find session!"));
	return true;
}

bool FOnlineSessionV2AccelByte::FindSessionByStringId(int LocalUserNum
	, const EAccelByteV2SessionType& SessionType
	, const FString& SessionId
	, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionType: %s; SessionId: %s")
		, LocalUserNum
		, *FAccelByteUtilities::GetUEnumValueAsString(SessionType)
		, *SessionId);

	if (SessionType == EAccelByteV2SessionType::Unknown)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to find session when type is unknown!"));
		return false;
	}

	if (SessionId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to find session when ID is blank!"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find session as the AccelByte subsystem is invalid"));
		return false;
	}

	TOptional<bool> IsDedicatedServer = AccelByteSubsystemPtr->IsDedicatedServer(LocalUserNum);

	bool IsDS = false;

	if (IsDedicatedServer.IsSet())
	{
		IsDS = IsDedicatedServer.GetValue();
	}

	FUniqueNetIdAccelByteResourceRef SessionUniqueId = FUniqueNetIdAccelByteResource::Create(SessionId);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, SessionUniqueId.Get()
			, CompletionDelegate
			, nullptr
			, IsDS);
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2PartyById>(AccelByteSubsystemPtr.Get()
			, LocalUserNum
			, SessionUniqueId.Get()
			, CompletionDelegate
			, nullptr
			, IsDS);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as our AccelByte subsystem is invalid"));
		return false;
	}
	
	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as our identity interface is invalid!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), LocalUserNum]() {
			SessionInterface->TriggerOnFindFriendSessionCompleteDelegates(LocalUserNum, false, {});
		});
		
		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as we could not find a unique ID for player at index %d!"), LocalUserNum);

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), LocalUserNum]() {
			SessionInterface->TriggerOnFindFriendSessionCompleteDelegates(LocalUserNum, false, {});
		});
		
		return false;
	}

	return FindFriendSession(PlayerId.ToSharedRef().Get(), Friend);
}

bool FOnlineSessionV2AccelByte::FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as our AccelByte subsystem is invalid"));
		return false;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as our identity interface is invalid!"));

		// cannot trigger error delegate here because localUserNum can only be got from identity interface
		
		return false;
	}
	
	int32 localUserNum;
	if(!IdentityInterface->GetLocalUserNum(LocalUserId, localUserNum))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as we cannot find the local user ID controller number"));

		// cannot trigger error delegate here because localUserNum not found
		
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Find friend session is not implemented!"));

	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), localUserNum]() {
		SessionInterface->TriggerOnFindFriendSessionCompleteDelegates(localUserNum, false, {});
	});
		
	return false;
}

bool FOnlineSessionV2AccelByte::FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& FriendList)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as our AccelByte subsystem is invalid"));
		return false;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as our identity interface is invalid!"));

		// cannot trigger error delegate here because localUserNum can only be got from identity interface
		
		return false;
	}
	
	int32 LocalUserNum;
	if(!IdentityInterface->GetLocalUserNum(LocalUserId, LocalUserNum))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find friend session as we cannot find the local user ID controller number"));

		// cannot trigger error delegate here because localUserNum not found
		
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Find friend session is not implemented!"));

	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), LocalUserNum]() {
		SessionInterface->TriggerOnFindFriendSessionCompleteDelegates(LocalUserNum, false, {});
	});
		
	return false;
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionName: %s; Friend: %s"), LocalUserNum, *SessionName.ToString(), *Friend.ToDebugString());

	if (IsRunningDedicatedServer())
	{
		// If we are running a dedicated server, then we just want to continue sending the invite with an invalid local user ID.
		// Servers do not have user IDs for the authenticated client, so just fake it with the invalid ID.
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending invite to user '%s'!"), *Friend.ToDebugString());
		return SendSessionInviteToFriend(FUniqueNetIdAccelByteUser::Invalid().Get(), SessionName, Friend);
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as our AccelByte subsystem is invalid"));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as our identity interface is invalid!"));
		
		// cannot trigger delegate here because we can't get the localUserId without Identity Interface
		
		return false;
	}

	FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as we could not get a unique ID from player index %d!"), LocalUserNum);

		// cannot trigger delegate here because we can't get the localUserId in Identity Interface

		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending invite to user '%s'!"), *Friend.ToDebugString());
	return SendSessionInviteToFriend(PlayerId.ToSharedRef().Get(), SessionName, Friend);
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; FriendId: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *Friend.ToDebugString());

	if (!LocalUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as the LocalUserId is invalid"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!Session)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to invite player to session as the session does not exist locally!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), LocalUserId = LocalUserId.AsShared(), SessionName, Friend = Friend.AsShared()]() {
				SessionInterface->TriggerOnSendSessionInviteCompleteDelegates(LocalUserId.Get(), SessionName, false, Friend.Get());
			});
		
		return false;
	}

	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();
	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to invite player to session as the IdentityInterface is invalid!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), LocalUserId = LocalUserId.AsShared(), SessionName, Friend = Friend.AsShared()]() {
			SessionInterface->TriggerOnSendSessionInviteCompleteDelegates(LocalUserId.Get(), SessionName, false, Friend.Get());
			});

		return false;
	}
	IdentityInterface->GetLocalUserNum(LocalUserId, LocalUserNum);
	TOptional<bool> IsDS = AccelByteSubsystemPtr->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to invite player to session as the IsDS is not set!"));

		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), LocalUserId = LocalUserId.AsShared(), SessionName, Friend = Friend.AsShared()]() {
			SessionInterface->TriggerOnSendSessionInviteCompleteDelegates(LocalUserId.Get(), SessionName, false, Friend.Get());
			});

		return false;
	}

	if (!IsDS.GetValue())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), LocalUserId, true);
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		if (IsDS.GetValue())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send session invite as game server is not allowed to send a public game session invite. "));
			return false;
		}

		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteSendV2GameSessionInvite>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Friend);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending invite to player for game session!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteSendV2PartyInvite>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Friend);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending invite to player for party session!"));
		return true;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed to Send Session Invite to friend, Session type not supported!"));
	
	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this),  LocalUserId = LocalUserId.AsShared(), SessionName, Friend = Friend.AsShared()]() {
		SessionInterface->TriggerOnSendSessionInviteCompleteDelegates(LocalUserId.Get(), SessionName, false, Friend.Get());
	});

	return false;
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends)
{
	UE_LOG_AB(Warning, TEXT("FOnlineSessionV2AccelByte::SendSessionInviteToFriends is not implemented! Please send invites one at a time through FOnlineSessionV2AccelByte::SendSessionInviteToFriend!"));
	// no delegates to trigger here
	return false;
}

bool FOnlineSessionV2AccelByte::SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends)
{
	UE_LOG_AB(Warning, TEXT("FOnlineSessionV2AccelByte::SendSessionInviteToFriends is not implemented! Please send invites one at a time through FOnlineSessionV2AccelByte::SendSessionInviteToFriend!"));
	// no delegates to trigger here
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
	if (!SessionInfo.IsValid())
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; bWasInvited: %s"), *SessionName.ToString(), LOG_BOOL_FORMAT(bWasInvited));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register player to session as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		if (!IsRunningDedicatedServer())
		{
			// If we do not have a session with this name, and we are not running a DS, just fail the call
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register players to session as a session with name '%s' does not exist!"), *SessionName.ToString());

			AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
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
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register players to session as a session info does not exist!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Players, SessionName]() {
			SessionInterface->TriggerOnRegisterPlayersCompleteDelegates(SessionName, Players, false);
		});
		
		return false;
	}

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!ensure(SessionData.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register players to session as a backend session data does not exist!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Players, SessionName]() {
			SessionInterface->TriggerOnRegisterPlayersCompleteDelegates(SessionName, Players, false);
		});
		
		return false;
	}

	for (const TSharedRef<const FUniqueNetId>& PlayerToAdd : Players)
	{
		FUniqueNetIdMatcher PlayerToAddMatcher(PlayerToAdd.Get());
		int32 FoundPlayerIndex = Session->RegisteredPlayers.IndexOfByPredicate(PlayerToAddMatcher);
		if (FoundPlayerIndex == INDEX_NONE)
		{
			Session->RegisteredPlayers.Emplace(PlayerToAdd);

			// Attempt to find an existing member settings object for this player, and if not found, initialize one
			const FSessionSettings* FoundMemberSettings = nullptr;
			for (const TPair<FUniqueNetIdRef, FSessionSettings>& KV : Session->SessionSettings.MemberSettings)
			{
				if (KV.Key.Get() == PlayerToAdd.Get())
				{
					FoundMemberSettings = &KV.Value;
					break;
				}
			}

			if (FoundMemberSettings == nullptr)
			{
				// No member settings object initialized for this user, do so now
				Session->SessionSettings.MemberSettings.Add(PlayerToAdd, FSessionSettings());
			}

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
				FOnlineVoiceAccelBytePtr VoiceInterface = StaticCastSharedPtr<FOnlineVoiceAccelByte>(AccelByteSubsystemPtr->GetVoiceInterface());
				if (VoiceInterface.IsValid())
				{
					VoiceInterface->RegisterTalker(PlayerToAdd, *Session);
				}
			}

			if (IsRunningDedicatedServer())
			{
				FServerApiClientPtr ServerApiClient = GetServerApiClient();
				if(!GetServerApiClient().IsValid())
				{
					return false;
				}
				const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
				FUniqueNetIdAccelByteUserPtr AccelByteUser = FUniqueNetIdAccelByteUser::CastChecked(PlayerToAdd);
				if (PredefinedEventInterface.IsValid() && AccelByteUser.IsValid())
				{
					FAccelByteModelsDSGameClientJoinedPayload DSGameClientJoinedPayload{};
					DSGameClientJoinedPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
					DSGameClientJoinedPayload.UserId = AccelByteUser->GetAccelByteId();
					PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSGameClientJoinedPayload>(DSGameClientJoinedPayload));
				}
			}
		}
	}

	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
		SessionInterface->TriggerOnRegisterPlayersCompleteDelegates(SessionName, Players, true);
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister player from session as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		if (!IsRunningDedicatedServer())
		{
			// If we are not running a server instance and there is no session, then just fail as the player needs to have one to register
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister players from session as a session with name '%s' does not exist!"), *SessionName.ToString());

			AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
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
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister players to session as a session info does not exist!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Players, SessionName]() {
			SessionInterface->TriggerOnUnregisterPlayersCompleteDelegates(SessionName, Players, false);
		});
		
		return false;
	}

	TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!ensure(SessionData.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister players to session as a backend session data does not exist!"), *SessionName.ToString());
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Players, SessionName]() {
			SessionInterface->TriggerOnUnregisterPlayersCompleteDelegates(SessionName, Players, false);
		});
		
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

			if (IsRunningDedicatedServer())
			{
				FServerApiClientPtr ServerApiClient = GetServerApiClient();
				const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
				FUniqueNetIdAccelByteUserPtr AccelByteUser = FUniqueNetIdAccelByteUser::CastChecked(PlayerToRemove);
				if (PredefinedEventInterface.IsValid() && AccelByteUser.IsValid())
				{
					FAccelByteModelsDSGameClientLeftPayload DSGameClientLeftPayload{};
					DSGameClientLeftPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
					DSGameClientLeftPayload.UserId = AccelByteUser->GetAccelByteId();
					PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSGameClientLeftPayload>(DSGameClientLeftPayload));
				}
			}
		}
	}

	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), Players, SessionName]() {
		SessionInterface->TriggerOnUnregisterPlayersCompleteDelegates(SessionName, Players, true);
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
		if (!SessionInfo.IsValid())
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionName: %s; TargetPlayerId: %s"), LocalUserNum, *SessionName.ToString(), *TargetPlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove player from session as our AccelByte subsystem is invalid"));
		return;
	}

	// Grab a user ID for the local user index provided through the identity interface
	FOnlineIdentityAccelBytePtr IdentityInterface = nullptr;
	if (!FOnlineIdentityAccelByte::GetFromSubsystem(AccelByteSubsystemPtr.Get(), IdentityInterface))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove player '%s' from session '%s' as a unique ID for local player %d was not found!"), *TargetPlayerId.ToDebugString(), *SessionName.ToString(), LocalUserNum);
		return;
	}

	FUniqueNetIdPtr LocalPlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalPlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove player '%s' from session '%s' as a unique ID for local player %d was not found!"), *TargetPlayerId.ToDebugString(), *SessionName.ToString(), LocalUserNum);
		return;
	}

	KickPlayer(LocalPlayerId.ToSharedRef().Get(), SessionName, TargetPlayerId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}
#endif

bool FOnlineSessionV2AccelByte::QueryAllInvites(const FUniqueNetId& PlayerId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s"), *PlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query all invites as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites>(AccelByteSubsystemPtr.Get(), PlayerId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s; InvitedSessionId: %s"), *PlayerId.ToDebugString(), *InvitedSession.Session.GetSessionIdStr());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reject invite as our AccelByte subsystem is invalid"));
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(InvitedSession.Session.Session.SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRejectV2GameSessionInvite>(AccelByteSubsystemPtr.Get(), PlayerId, InvitedSession.Session, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending rejection for game session invite!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRejectV2PartyInvite>(AccelByteSubsystemPtr.Get(), PlayerId, InvitedSession.Session, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending rejection for party session invite!"));
		return true;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

bool FOnlineSessionV2AccelByte::RestoreActiveSessions(const FUniqueNetId& LocalUserId, const FOnRestoreActiveSessionsComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s"), *LocalUserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to restore active sessions as our AccelByte subsystem is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRestoreAllV2Sessions>(AccelByteSubsystemPtr.Get(), LocalUserId, Delegate);
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionToLeaveId: %s"), *LocalUserId.ToDebugString(), *SessionToLeave.Session.GetSessionIdStr());
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	const FServerApiClientPtr ServerApiClientPtr = GetServerApiClient();
	if(!ServerApiClientPtr.IsValid())
	{
		Delegate.ExecuteIfBound(false);
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Attempt to RegisterServer failed, ServerApiClient is invalid"));
		return;
	}

	const FAccelByteApiUtilitiesPtr Utilities = ServerApiClientPtr->GetServerApiUtilities().Pin();
	if(!Utilities.IsValid())
	{
		Delegate.ExecuteIfBound(false);
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Attempt to RegisterServer failed, Utilities is invalid"));
		return;
	}

	EAccelByteCurrentServerManagementType CurrentServerType = Utilities->GetCurrentServerManagementType();
	switch (CurrentServerType)
	{
	case EAccelByteCurrentServerManagementType::NOT_A_SERVER:
		Delegate.ExecuteIfBound(false);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Attempt to RegisterServer from a non-server build."));
		return;
	default:
		break;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Attempt to RegisterServer failed, subsystem is invalid"));
		return;
	}

	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (!Config.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Attempt to RegisterServer failed, config is invalid"));
		return;
	}

	if (Config->GetEnableManualServerRegistration().GetValue())
	{
		ResetWarningReminderForServerSendReady();
		const float WarningDelaySeconds = SendServerReadyWarningInMinutes * 60.0f;
		SendServerReadyWarningReminderDelegate = FTickerDelegate::CreateRaw(this, &FOnlineSessionV2AccelByte::OnServerNotSendReadyWhenTimesUp, Delegate);
		SendServerReadyWarningReminderHandle = FTickerAlias::GetCoreTicker().AddTicker(SendServerReadyWarningReminderDelegate, WarningDelaySeconds);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("This server requires to be set as Ready. Expecting to call SendServerReady() function."));
	}
	else //Automatic
	{
		SendServerReady(SessionName, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("OK. This server already call SendServerReady()."));
	}
}

bool FOnlineSessionV2AccelByte::OnServerNotSendReadyWhenTimesUp(float DeltaTime, FOnRegisterServerComplete Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	Delegate.ExecuteIfBound(false);
	ResetWarningReminderForServerSendReady();
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Server is not flagged as ready in %d minutes.\nPlease call SendServerReady() function. "), SendServerReadyWarningInMinutes);

	return false;
}

void FOnlineSessionV2AccelByte::ResetWarningReminderForServerSendReady()
{
	// Reminder removal
	if (SendServerReadyWarningReminderHandle.IsValid())
	{
		SendServerReadyWarningReminderHandle.Reset();
	}
	if (SendServerReadyWarningReminderDelegate.IsBound())
	{
		SendServerReadyWarningReminderDelegate.Unbind();
	}
	FTickerAlias::GetCoreTicker().RemoveTicker(SendServerReadyWarningReminderHandle);
}

void FOnlineSessionV2AccelByte::SendServerReady(FName SessionName, const FOnRegisterServerComplete& Delegate)
{	
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SendServerReady()"));
	ResetWarningReminderForServerSendReady();

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send server ready as our AccelByte subsystem is invalid"));
		return;
	}

	const FServerApiClientPtr ServerApiClientPtr = GetServerApiClient();
	if(!ServerApiClientPtr.IsValid())
	{
		Delegate.ExecuteIfBound(false);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to send server ready, ServerApiClient is invalid!"));
		return;
	}

	const FAccelByteApiUtilitiesPtr Utilities = ServerApiClientPtr->GetServerApiUtilities().Pin();
	if(!Utilities.IsValid())
	{
		Delegate.ExecuteIfBound(false);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to send server ready, Utilities is invalid!"));
		return;
	}

	EAccelByteCurrentServerManagementType CurrentServerType = Utilities->GetCurrentServerManagementType();
	switch (CurrentServerType)
	{
	case EAccelByteCurrentServerManagementType::NOT_A_SERVER:
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), Delegate]()
			{
				Delegate.ExecuteIfBound(false);
			});
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("SendServerReady() not executed because this is not a dedicated server"));
		return;
	}
	case EAccelByteCurrentServerManagementType::ONLINE_AMS:
	{
		FServerApiClientPtr ServerApiClient = GetServerApiClient();
		const AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived OnAMSDrainReceivedDelegate = AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnAMSDrain);
		ServerApiClient->ServerAMS.SetOnAMSDrainReceivedDelegate(OnAMSDrainReceivedDelegate);
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendReadyToAMS>(AccelByteSubsystemPtr.Get(), Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Registering cloud server to AMS!"));
		return;
	}
	case EAccelByteCurrentServerManagementType::ONLINE_ARMADA:
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRegisterRemoteServerV2>(AccelByteSubsystemPtr.Get(), SessionName, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Registering cloud server to Armada!"));
		return;
	}
	case EAccelByteCurrentServerManagementType::LOCAL_SERVER:
	{
		// #NOTE Deliberately leaving out session name from the local task, as nothing in that task relies on the name
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRegisterLocalServerV2>(AccelByteSubsystemPtr.Get(), Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Registering locally hosted server to Armada!"));
		return;
	}
	default:
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), Delegate]()
			{
				Delegate.ExecuteIfBound(false);
			});
		return;
	}
}

void FOnlineSessionV2AccelByte::UnregisterServer(FName SessionName, const FOnUnregisterServerComplete& Delegate /*= FOnUnregisterServerComplete()*/)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Only dedicated servers should be able to register to Armada or AMS
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register server to Armada as the current game instance is not a dedicated server!"));
		return;
	}
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister server as our AccelByte subsystem is invalid"));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	if (IsServerUseAMS() && !ServerApiClient->ServerSettings->DSId.IsEmpty())
	{
		// Disconnect from DS hub & from AMS
		DisconnectFromAMS();
		DisconnectFromDSHub();

		// Trigger UnregisterServer delegate
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = AsShared(), SessionName, Delegate]()
		{
			Delegate.ExecuteIfBound(true);
		});

		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Unregistering cloud server from AMS!"));
		return;
	}
	
	// For an Armada-spawned server pod, there will always be a POD_NAME environment variable set. For local servers this
	// will most likely never be the case. With this, if we have the pod name variable, then register as a non-local server
	// otherwise, register as a local server.
	const FString PodName = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME"));
	if (!PodName.IsEmpty())
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2>(AccelByteSubsystemPtr.Get(), SessionName, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Unregistering cloud server to Armada!"));
	}
	else
	{
		// #NOTE Deliberately leaving out session name from the local task, as nothing in that task relies on the name
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUnregisterLocalServerV2>(AccelByteSubsystemPtr.Get(), Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Unregistering locally hosted server to Armada!"));
	}
}

void FOnlineSessionV2AccelByte::SetServerTimeout(int32 NewTimeout)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Only dedicated servers should be able to set timeout
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set server timeout as the current game instance is not a dedicated server!"));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	if (IsServerUseAMS() && !ServerApiClient->ServerSettings->DSId.IsEmpty())
	{
		SetDSTimeout(NewTimeout);

		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send set server timeout to AMS!"));
		return;
	}
}

void FOnlineSessionV2AccelByte::ResetServerTimeout()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Only dedicated servers should be able to reset timeout
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reset server timeout as the current game instance is not a dedicated server!"));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	if (IsServerUseAMS() && !ServerApiClient->ServerSettings->DSId.IsEmpty())
	{
		ResetDSTimeout();

		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send reset server timeout to AMS!"));
		return;
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
	if (Session == nullptr)
	{
		return nullptr;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		return nullptr;
	}

	return SessionInfo->GetLeaderId();
}

bool FOnlineSessionV2AccelByte::KickPlayer(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToKick, const FOnKickPlayerComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToKick: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PlayerIdToKick.ToDebugString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot kick player from session as the session does not exist locally!"));
		// no delegates to trigger
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to kick player from session as our AccelByte subsystem is invalid"));
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		if (IsRunningDedicatedServer())
		{
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteServerKickV2GameSession>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, PlayerIdToKick, Delegate);
		}
		else
		{
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteKickV2GameSession>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, PlayerIdToKick, Delegate);
		}

		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to kick player from game session!"));
		return true;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteKickV2Party>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, PlayerIdToKick, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to kick player from party session!"));
		return true;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return false;
}

bool FOnlineSessionV2AccelByte::PromoteGameSessionLeader(const FUniqueNetId& LocalUserId, const FName& SessionName,
	const FUniqueNetId& PlayerIdToPromote)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToPromoteAsGameSessionLeader: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PlayerIdToPromote.ToDebugString());

	const FNamedOnlineSession* Session = GetNamedSession(SessionName);
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote to leader as our AccelByte subsystem is invalid"));
		return false;
	}
	
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot promote player from game session as the session does not exist locally!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), PlayerIdToPromote = PlayerIdToPromote.AsShared()]() {
			const FOnlineErrorAccelByte Error = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, TEXT("promote-game-session-leader-failed-session-not-exist-locally"), EOnlineErrorResult::Unknown);
			SessionInterface->TriggerOnPromoteGameSessionLeaderCompleteDelegates(PlayerIdToPromote.Get(), Error);
		});
		
		return false;
	}

	const EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	switch (SessionType)
	{
		case EAccelByteV2SessionType::GameSession:
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), LocalUserId, true);
			AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelBytePromoteV2GameSessionLeader>(AccelByteSubsystemPtr.Get(), LocalUserId, Session->GetSessionIdStr(), PlayerIdToPromote);
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to promote player to leader of game session!"));
			return true;

		case EAccelByteV2SessionType::PartySession:
		case EAccelByteV2SessionType::Unknown:
		default:
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Unable to promote game session leader of SessionName: %s as the session is of type %s!"), *SessionName.ToString(), *FAccelByteUtilities::GetUEnumValueAsString(SessionType));
			AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), PlayerIdToPromote = PlayerIdToPromote.AsShared()]() {
			const FOnlineErrorAccelByte Error = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, TEXT("promote-game-session-leader-failed-invalid-session-type"), EOnlineErrorResult::InvalidParams);
				SessionInterface->TriggerOnPromoteGameSessionLeaderCompleteDelegates(PlayerIdToPromote.Get(), Error);
			});
		return false;
	}
}

bool FOnlineSessionV2AccelByte::PromotePartySessionLeader(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToPromote, const FOnPromotePartySessionLeaderComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToPromote: %s"), *LocalUserId.ToDebugString(), *SessionName.ToString(), *PlayerIdToPromote.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote to leader as our AccelByte subsystem is invalid"));
		return false;
	}
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot promote party session leader from session as the session does not exist locally!"));
		
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate, PlayerIdToPromote = PlayerIdToPromote.AsShared()]() {
			const FOnlineErrorAccelByte Error = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, TEXT("promote-party-session-leader-failed-session-not-exist-locally"), EOnlineErrorResult::Unknown);
			Delegate.ExecuteIfBound(PlayerIdToPromote.Get(), Error);
		});
		
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Unable to promote party leader of SessionName: %s as the session is of type GameSession!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), PlayerIdToPromote = PlayerIdToPromote.AsShared()]() {
		const FOnlineErrorAccelByte Error = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, TEXT("promote-party-session-leader-failed-invalid-session-type"), EOnlineErrorResult::InvalidParams);
			SessionInterface->TriggerOnPromoteGameSessionLeaderCompleteDelegates(PlayerIdToPromote.Get(), Error);
		});
		return false;
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectLobby>(AccelByteSubsystemPtr.Get(), LocalUserId, true);
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelBytePromoteV2PartyLeader>(AccelByteSubsystemPtr.Get(), LocalUserId, SessionName, Session->GetSessionIdStr(), PlayerIdToPromote, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to promote player to leader of party session!"));
		return true;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), PlayerIdToPromote = PlayerIdToPromote.AsShared()]() {
	const FOnlineErrorAccelByte Error = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, TEXT("promote-party-session-leader-failed-invalid-session-type"), EOnlineErrorResult::InvalidParams);
		SessionInterface->TriggerOnPromoteGameSessionLeaderCompleteDelegates(PlayerIdToPromote.Get(), Error);
	});
	return false;
}

TArray<FString> FOnlineSessionV2AccelByte::GetRegionList(const FUniqueNetId& LocalPlayerId) const
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s"), *LocalPlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get region as our AccelByte subsystem is invalid"));
		return TArray<FString>();
	}

	AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalPlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not get list of regions as we could not get an API client for the user specified!"));
		return TArray<FString>();
	}

	const auto Qos = ApiClient->GetQosApi().Pin();
	if (!ensure(Qos.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not get list of regions as we could not get an Qos API for the user specified!"));
		return TArray<FString>();
	}

	// Get latencies to QOS regions, and sort by lowest latency
	TArray<TPair<FString, float>> Latencies = Qos->GetCachedLatencies();
	Latencies.Sort([](const TPair<FString, float>& LeftHandLatency, const TPair<FString, float>& RightHandLatency) {
		return LeftHandLatency.Value < RightHandLatency.Value;
	});

	// Now add each region to the output array
	TArray<FString> OutRegions;
	for (const TPair<FString, float>& Latency : Latencies)
	{
		OutRegions.Emplace(Latency.Key);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to refresh session as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (!Session)
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot refresh session as the session does not exist locally!"));
		return false;
	}

	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();
	TOptional<bool> IsDS = AccelByteSubsystemPtr->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Delegate]() {
			Delegate.ExecuteIfBound(false);
			});

		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot refresh session as the IsDS is not valid!"));
		return false;
	}
	auto IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Delegate]() {
			Delegate.ExecuteIfBound(false);
			});

		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot refresh session as the IdentityInterface is not valid!"));
		return false;
	}

	auto LocalUserId = FUniqueNetIdAccelByteUser::Invalid();
	// Only override LocalUserNum for non DS
	if(!IsDS.GetValue() && Session->LocalOwnerId.IsValid())
	{
		LocalUserId = FUniqueNetIdAccelByteUser::CastChecked(Session->LocalOwnerId.ToSharedRef());
		if (LocalUserId->IsValid())
		{
			IdentityInterface->GetLocalUserNum(LocalUserId.Get(), LocalUserNum);
		}
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType == EAccelByteV2SessionType::GameSession)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshV2GameSession>(AccelByteSubsystemPtr.Get(), LocalUserId.Get(), SessionName, Delegate, IsDS.GetValue());
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to refresh local game session with backend data!"));
	}
	else if (SessionType == EAccelByteV2SessionType::PartySession)
	{
		if (IsRunningDedicatedServer())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Game servers are not able to refresh party sessions!"));
			
			AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Delegate]() {
				Delegate.ExecuteIfBound(false);
			});
		
			return false;
		}

		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshV2PartySession>(AccelByteSubsystemPtr.Get(), SessionName, Delegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to refresh local party session with backend data!"));
	}

	return true;
}

bool FOnlineSessionV2AccelByte::RefreshActiveSessions(const FOnRefreshActiveSessionsComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to refresh active session as our AccelByte subsystem is invalid"));
		return false;
	}

	if (Sessions.Num() < 1)
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionInterface = SharedThis(this), Delegate]() {
			Delegate.ExecuteIfBound(false, {});
		});
		
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot refresh active session as the session is empty!"))
		return false;
	}

	TArray<FName> SessionNames;
	Sessions.GetKeys(SessionNames);

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshActiveSessions>(AccelByteSubsystemPtr.Get(), SessionNames, Delegate);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to refresh active sessions with backend data!"));

	return true;
}

bool FOnlineSessionV2AccelByte::RefreshActiveSessionsV2AfterReconnected(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to refresh active session as our AccelByte subsystem is invalid"));
		return false;
	}

	if (Sessions.Num() == 0)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("There is no active session needs to be refreshed."))
		return true;
	}

	TArray<FName> SessionNames;
	Sessions.GetKeys(SessionNames);

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions>(AccelByteSubsystemPtr.Get(), LocalUserNum, SessionNames, true);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sent off request to refresh active sessions with backend data!"));

	return true;
}

bool FOnlineSessionV2AccelByte::CreateBackfillTicket(const FName& SessionName, const FOnCreateBackfillTicketComplete& Delegate)
{
	return CreateBackfillTicket(SessionName, TEXT(""), Delegate);
}

bool FOnlineSessionV2AccelByte::CreateBackfillTicket(const FName& SessionName, const FString& MatchPool, const FOnCreateBackfillTicketComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create backfill ticket as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot create backfill ticket for session as the session does not exist locally!"));
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate, SessionName]() {
			Delegate.ExecuteIfBound(false);
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateBackfillTicket>(AccelByteSubsystemPtr.Get(), SessionName, MatchPool, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::DeleteBackfillTicket(const FName& SessionName, const FOnDeleteBackfillTicketComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete backfill ticket as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot delete backfill ticket for session as the session does not exist locally!"));
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate, SessionName]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteBackfillTicket>(AccelByteSubsystemPtr.Get(), SessionName, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::AcceptBackfillProposal(const FName& SessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Proposal, bool bStopBackfilling, const FOnAcceptBackfillProposalComplete& Delegate, FAccelByteModelsV2MatchmakingBackfillAcceptanceOptionalParam const& OptionalParameter)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to accept backfill approval as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot accept backfill proposal for session as the session does not exist locally!"));
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteAcceptBackfillProposal>(AccelByteSubsystemPtr.Get(), SessionName, Proposal, OptionalParameter, bStopBackfilling, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::RejectBackfillProposal(const FName& SessionName, const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Proposal, bool bStopBackfilling, const FOnRejectBackfillProposalComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reject backfill proposal as our AccelByte subsystem is invalid"));
		return false;
	}

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot reject backfill proposal for session as the session does not exist locally!"));
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRejectBackfillProposal>(AccelByteSubsystemPtr.Get(), SessionName, Proposal, bStopBackfilling, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

#pragma region PAST_SESSION_SYNC_TO_PARTY
bool FOnlineSessionV2AccelByte::IsCurrentUserEligiblePartyStorageAction(FUniqueNetIdAccelByteUserPtr UserUniqueNetId)
{
	FNamedOnlineSession* Session = GetPartySession();
	if (Session == nullptr)
	{
		return false;
	}

	EAccelByteV2SessionType SessionType = GetSessionTypeFromSettings(Session->SessionSettings);
	if (SessionType != EAccelByteV2SessionType::PartySession)
	{
		return false;
	}

	if (!UserUniqueNetId.IsValid() || !Session->LocalOwnerId.IsValid())
	{
		return false;
	}

	if (!Session->LocalOwnerId->ToString().Equals(UserUniqueNetId->UniqueNetIdStr))
	{
		return false;
	}

	return true;
}

bool FOnlineSessionV2AccelByte::GetPartySessionStorage(FUniqueNetIdAccelByteUserPtr UserUniqueNetId, const FOnGetPartySessionStorageComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: party only"));

	if (!IsCurrentUserEligiblePartyStorageAction(UserUniqueNetId))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Current user is not eligible to do GET action toward party session storage."));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get party session storage as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetPartySessionStorage>(AccelByteSubsystemPtr.Get(), UserUniqueNetId, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created async task to update party attribute with past session info on backend!"));
	return true;
}

bool FOnlineSessionV2AccelByte::UpdatePartySessionStorageWithPastSessionInfo(FUniqueNetIdAccelByteUserPtr UserUniqueNetId)
{
	if (!bIsEnablePartyMemberPastSessionRecordSync)
	{
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: party only"));
	
	FNamedOnlineSession* Session = GetPartySession();

	if (!IsCurrentUserEligiblePartyStorageAction(UserUniqueNetId) || Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Current user is not eligible to do PATCH action toward party session storage."));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update party attribute with past session info as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage>(AccelByteSubsystemPtr.Get()
		, UserUniqueNetId
		, PartySessionStorageLocalUserManager.ExtractCacheToWriteToPartyStorage(Session->LocalOwnerId));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created async task to update party attribute with past session info on backend!"));
	return true;
}

void FOnlineSessionV2AccelByte::SetEnablePartyMemberPastSessionRecordSync(bool bEnable, uint32 AmountOfPastSessionRecorded)
{
	bIsEnablePartyMemberPastSessionRecordSync = bEnable;
	this->PartySessionStorageLocalUserManager.PastSessionManager.SetMaxStoredSessionIdCount(AmountOfPastSessionRecorded);
}

bool FOnlineSessionV2AccelByte::GetEnablePartyMemberPastSessionRecordSync() { return bIsEnablePartyMemberPastSessionRecordSync; }

TArray<FString> FOnlineSessionV2AccelByte::ExtractExcludedSessionFromPartySessionStorage(FOnlineSessionSearchAccelByte& SearchHandle, const FAccelByteModelsV2PartySessionStorage& Storage)
{
	// Defensive checking earlier
	if (SearchHandle.GameSessionExclusion.CurrentType == FAccelByteModelsGameSessionExcludedSession::ExclusionType::NONE)
	{
		return TArray<FString>();
	}
	if (SearchHandle.GameSessionExclusion.CurrentType == FAccelByteModelsGameSessionExcludedSession::ExclusionType::EXPLICIT_LIST)
	{
		return SearchHandle.GameSessionExclusion.GetExcludedGameSessionIDs();
	}

	TSet<FString> UniqueSessionIDs{};
	TArray<FAccelByteModelsV2PartySessionStorageReservedData> ReservedDataFromEachUser{};
	Storage.Reserved.GenerateValueArray(ReservedDataFromEachUser);
	for (int i = 0; i < ReservedDataFromEachUser.Num(); i++)
	{
		auto SessionIDs = ReservedDataFromEachUser[i].PastSessionIDs;

		// Resize to N-Past Session count
		if (SearchHandle.GameSessionExclusion.CurrentType == FAccelByteModelsGameSessionExcludedSession::ExclusionType::N_PAST_SESSION)
		{
			int32 ExcessToRemove = SessionIDs.Num() - SearchHandle.GameSessionExclusion.ExcludedPastSessionCount;
			if (ExcessToRemove > 0)
			{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
				SessionIDs.RemoveAt(0, ExcessToRemove, EAllowShrinking::Yes);
#else
				SessionIDs.RemoveAt(0, ExcessToRemove, true);
#endif // ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
			}
		}
		UniqueSessionIDs.Append(SessionIDs);
	}

	return UniqueSessionIDs.Array();
}


#pragma endregion

bool FOnlineSessionV2AccelByte::UpdateMemberStatus(FName SessionName, const FUniqueNetId& PlayerId, const EAccelByteV2SessionMemberStatus& Status, const FOnSessionMemberStatusUpdateComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s; PlayerId: %s; Status: %s"), *SessionName.ToString(), *PlayerId.ToDebugString(), *StaticEnum<EAccelByteV2SessionMemberStatus>()->GetNameStringByValue(static_cast<int64>(Status)));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update member status as our AccelByte subsystem is invalid"));
		return false;
	}
	
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not member status for session named '%s' as the session does not exist locally!"), *SessionName.ToString());
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate]() {
			Delegate.ExecuteIfBound(false);
		});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateMemberStatus>(AccelByteSubsystemPtr.Get(), SessionName, PlayerId, Status, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::EnqueueBackendDataUpdate(const FName& SessionName, const TSharedPtr<FAccelByteModelsV2BaseSession>& NewSessionData, const bool bIsDSReadyUpdate/*=false*/)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not enqueue session backend data update as there is no session with the provided name!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not enqueue session backend data update as session does not have a valid session info instance!"));
		return;
	}

	const TSharedPtr<FAccelByteModelsV2BaseSession> ExistingSessionData = SessionInfo->GetBackendSessionData();
	if (!NewSessionData.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not enqueue session backend data update as session does not have a valid existing backend data instance!"));
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

	// If we should update, or if we have an update with new server information, signal in the array that we have a
	// pending update to process
	if (bShouldUpdate || bIsDSReadyUpdate)
	{
		SessionsWithPendingQueuedUpdates.AddUnique(SessionName);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnInvitedToGameSessionNotification(FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *InviteEvent.SessionID);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to hadnle game session invite notification as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	FApiClientPtr ApiClientPtr = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	TSharedPtr<FAccelByteKey> LobbyLockKey;
	if (ApiClientPtr.IsValid())
	{
		auto Lobby =  ApiClientPtr->GetLobbyApi().Pin();
		if (Lobby.IsValid())
		{
			LobbyLockKey = Lobby->LockNotifications();
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Invalid Lobby from API Client"));
		}
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InviteEvent.SessionID);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as we could not create a valid session unique ID from the ID in the notification!"));
		return;
	}

	SetSessionInvitationGetInfoInProgress(InviteEvent.SessionID);
	OnFindGameSessionForInviteCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindGameSessionForInviteComplete, InviteEvent);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get(), PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindGameSessionForInviteCompleteDelegate, LobbyLockKey);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2GameSessionInvitedPayload GameSessionInvitedPayload{};
		GameSessionInvitedPayload.UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerId)->GetAccelByteId();
		GameSessionInvitedPayload.GameSessionId = InviteEvent.SessionID;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2GameSessionInvitedPayload>(GameSessionInvitedPayload));
	}

	StopSessionInviteCheckPoll(PlayerId, InviteEvent.SessionID);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGameSessionInvitationTimeoutNotification(FAccelByteModelsV2GameSessionUserInviteTimeoutEvent InvitationTimeoutEvent, int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite timeout notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invitation timeout notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invitation timeout notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InvitationTimeoutEvent.SessionID);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invitation timeout notification as we could not create a valid session unique ID from the ID in the notification!"));
		return;
	}

	auto TimeoutInvitationPtr = SessionInvites.FindByPredicate([PartyID = InvitationTimeoutEvent.SessionID](const FOnlineSessionInviteAccelByte& Invite) {
		return Invite.Session.GetSessionIdStr().Contains(PartyID);
		});

	if (TimeoutInvitationPtr == nullptr)
	{
		return;
	}

	//Make a copy before remove it
	FAccelByteTimeManagerWPtr TimeManagerWPtr = nullptr;
	const FAccelByteInstancePtr AccelByteInstance =  AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
	if(AccelByteInstance.IsValid())
	{
		TimeManagerWPtr = AccelByteInstance->GetTimeManager();
	}
	FOnlineSessionInviteAccelByte TimeoutInvitation{TimeManagerWPtr};
	TimeoutInvitation.SessionType = EAccelByteV2SessionType::GameSession;
	TimeoutInvitation.RecipientId = PlayerId;
	TimeoutInvitation.Session = TimeoutInvitationPtr->Session;
	TimeoutInvitation.SenderId = TimeoutInvitationPtr->SenderId;
	TimeoutInvitation.ExpiredAt = TimeoutInvitationPtr->ExpiredAt;

	const FString& InviteIdToBeRemoved = TimeoutInvitationPtr->Session.GetSessionIdStr();
	RemoveInviteById(InviteIdToBeRemoved);
	TriggerOnV2SessionInviteTimeoutReceivedDelegates(PlayerId.ToSharedRef().Get(), TimeoutInvitation, EAccelByteV2SessionType::GameSession);
	//TODO:
	//const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
}

void FOnlineSessionV2AccelByte::OnFindGameSessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; bWasSuccessful: %s; SessionId: %s"), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful), *Result.GetSessionIdStr());

	OnFindGameSessionForInviteCompleteDelegate.Unbind();

	RemoveSessionInvitationGetInfoInProgress(InviteEvent.SessionID);

	if (!bWasSuccessful)
	{
		AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to notify player about invite to session as we failed to query data about session from backend!"));
		return;
	}
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to notify player about invite to session as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// Find sender ID in the backend session data so that we can get their platform info for the ID
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Result.Session.SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our session's information instance is invalid!"));
		return;
	}

	const TSharedPtr<FAccelByteModelsV2GameSession> GameSessionData = SessionInfo->GetBackendSessionDataAsGameSession();
	if (!ensure(GameSessionData.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as our local copy of the backend session data is invalid!"));
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
		FAccelByteModelsV2SessionUser* FoundSender = GameSessionData->Members.FindByPredicate([InviteEvent](const FAccelByteModelsV2SessionUser& User) {
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
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session invite notification as the ID of the sender of the invite is invalid!"));
		return;
	}

	FAccelByteTimeManagerWPtr TimeManagerWPtr = nullptr;
	const FAccelByteInstancePtr AccelByteInstance =  AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
	if(AccelByteInstance.IsValid())
	{
		TimeManagerWPtr = AccelByteInstance->GetTimeManager();
	}
	
	FOnlineSessionInviteAccelByte NewInvite{TimeManagerWPtr};
	NewInvite.SessionType = EAccelByteV2SessionType::GameSession;
	NewInvite.RecipientId = PlayerId;
	NewInvite.Session = Result;
	NewInvite.SenderId = SenderId;
	NewInvite.ExpiredAt = InviteEvent.ExpiredAt;
	UpdateSessionInvite(NewInvite);

	TriggerOnSessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), AccelByteSubsystemPtr->GetAppId(), Result);
	TriggerOnV2SessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), NewInvite);
	TriggerOnInviteListUpdatedDelegates();

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGameSessionMembersChangedNotification(FAccelByteModelsV2GameSessionMembersChangedEvent MembersChangedEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s; JoinerId: %s"), *MembersChangedEvent.Session.ID, *MembersChangedEvent.JoinerID);

	FNamedOnlineSession* Session = GetNamedSessionById(MembersChangedEvent.Session.ID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with members changed event as we do not have the session stored locally!"));
		return;
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(MembersChangedEvent.Session));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGameSessionUpdatedNotification(FAccelByteModelsV2GameSession UpdatedGameSession, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *UpdatedGameSession.ID);

	FNamedOnlineSession* Session = GetNamedSessionById(UpdatedGameSession.ID);
	if (Session == nullptr)
	{
		FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if(!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session updated notification as our AccelByte subsystem is invalid"));
			return;
		}

		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
		if (!ensure(IdentityInterface.IsValid()))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session updated notification as our identity interface is invalid!"));
			return;
		}

		const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (!ensure(PlayerId.IsValid()))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session updated notification as a unique ID could not be found for player at index %d!"), LocalUserNum);
			return;
		}

		FApiClientPtr ApiClientPtr = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
		TSharedPtr<FAccelByteKey> LobbyLockKey;
		if (ApiClientPtr.IsValid())
		{
			auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
			if (Lobby.IsValid())
			{
				LobbyLockKey = Lobby->LockNotifications();
			}
			else
			{
				UE_LOG_AB(Warning, TEXT("Invalid Lobby from API Client"));
			}
		}
		else
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session updated notification as ApiClient is invalid for player at index %d!"), LocalUserNum);
			return;
		}

		const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(UpdatedGameSession.ID);
		if (!ensure(SessionUniqueId.IsValid()))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session updated notification as a unique ID could not be created for the match's session ID!"));
			return;
		}

		const FOnSingleSessionResultCompleteDelegate OnCheckGameSessionDataCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnCheckGameSessionDataComplete);
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get(), PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnCheckGameSessionDataCompleteDelegate, LobbyLockKey);

		return;
	}

	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(UpdatedGameSession));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnKickedFromGameSessionNotification(FAccelByteModelsV2GameSessionUserKickedEvent KickedEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *KickedEvent.SessionID);

	// First check if the inbound session ID matches one stored locally
	FNamedOnlineSession* Session = GetNamedSessionById(*KickedEvent.SessionID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session kicked notification as we do not have the session stored locally!"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session kicked notification as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session kicked notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session kicked notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// If the game session we're kicked from matches the local session found, destroy the session, which will also
	// trigger leave if needed
	TriggerOnKickedFromSessionDelegates(Session->SessionName);
	DestroySession(Session->SessionName, FOnDestroySessionCompleteDelegate(), true);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnDsStatusChangedNotification(FAccelByteModelsV2DSStatusChangedNotif DsStatusChangeEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *DsStatusChangeEvent.SessionID);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle ds status changed notification as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle DS status changed notification as our identity interface is invalid!"));
		return;
	}

	const bool bDsStatusError = DsStatusChangeEvent.Session.DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::FAILED_TO_REQUEST;

	FNamedOnlineSession* Session = GetNamedSessionById(DsStatusChangeEvent.SessionID);
	if (Session == nullptr)
	{

		const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (!ensure(PlayerId.IsValid()))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle DS status changed notification as a unique ID could not be found for player at index %d!"), LocalUserNum);
			return;
		}

		FApiClientPtr ApiClientPtr = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
		TSharedPtr<FAccelByteKey> LobbyLockKey;
		if (ApiClientPtr.IsValid())
		{
			auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
			if (Lobby.IsValid())
			{
				LobbyLockKey = Lobby->LockNotifications();
			}
			else
			{
				UE_LOG_AB(Warning, TEXT("Invalid Lobby from API Client"));
			}
		}
		else
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle DS status changed notification as ApiClient is invalid for player at index %d!"), LocalUserNum);
			return;
		}

		const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(DsStatusChangeEvent.SessionID);
		if (!ensure(SessionUniqueId.IsValid()))
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle DS status changed notification as a unique ID could not be created for the match's session ID!"));
			return;
		}

		const FOnSingleSessionResultCompleteDelegate OnCheckGameSessionDataCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnCheckGameSessionDataComplete);
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get(), PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnCheckGameSessionDataCompleteDelegate, LobbyLockKey);

		return;
	}

	if (bDsStatusError)
	{
		TriggerOnSessionServerErrorDelegates(Session->SessionName, DsStatusChangeEvent.Error);
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("DS status changed with an error message attached! Error message: %s"), *DsStatusChangeEvent.Error);
		return;
	}

	const bool bStatusIsReady = DsStatusChangeEvent.Session.DSInformation.StatusV2 == EAccelByteV2GameSessionDsStatus::AVAILABLE;

	// If the new status is not ready, then we cannot do anything, so just bail
	if (!bStatusIsReady)
	{
		// #NOTE No warning here as technically nothing bad happened
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (PlayerId.IsValid())
	{
		StopSessionServerCheckPoll(PlayerId, Session->SessionName);
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(DsStatusChangeEvent.Session), true);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	FApiClientPtr ApiClientPtr = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	TSharedPtr<FAccelByteKey> LobbyLockKey;
	if (ApiClientPtr.IsValid())
	{
		auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
		if (Lobby.IsValid())
		{
			LobbyLockKey = Lobby->LockNotifications();
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Invalid Lobby from API Client"));
		}
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as ApiClient is invalid for player at index %d!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InviteEvent.PartyID);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as we could not create a valid session unique ID from the ID in the notification!"));
		return;
	}

	const FOnSingleSessionResultCompleteDelegate OnFindPartySessionForInviteCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindPartySessionForInviteComplete, InviteEvent);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2PartyById>(AccelByteSubsystemPtr.Get(), PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindPartySessionForInviteCompleteDelegate, LobbyLockKey);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2PartySessionInvitedPayload PartySessionInvitedPayload{};
		PartySessionInvitedPayload.UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerId)->GetAccelByteId();
		PartySessionInvitedPayload.PartySessionId = InviteEvent.PartyID;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionInvitedPayload>(PartySessionInvitedPayload));
	}
}

void FOnlineSessionV2AccelByte::OnPartySessionInvitationTimeoutNotification(FAccelByteModelsV2PartyInviteTimeoutEvent InvitationTimeoutEvent, int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite timeout notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invitation timeout notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invitation timeout notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(InvitationTimeoutEvent.PartyID);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invitation timeout notification as we could not create a valid session unique ID from the ID in the notification!"));
		return;
	}

	auto TimeoutInvitationPtr = SessionInvites.FindByPredicate([PartyID = InvitationTimeoutEvent.PartyID](const FOnlineSessionInviteAccelByte& Invite) {
		return Invite.Session.GetSessionIdStr().Contains(PartyID);
		});

	if (TimeoutInvitationPtr == nullptr)
	{
		return;
	}

	//Make a copy before remove it
	FAccelByteTimeManagerWPtr TimeManagerWPtr = nullptr;
	const FAccelByteInstancePtr AccelByteInstance =  AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
	if(AccelByteInstance.IsValid())
	{
		TimeManagerWPtr = AccelByteInstance->GetTimeManager();
	}
	
	FOnlineSessionInviteAccelByte TimeoutInvitation{TimeManagerWPtr};
	TimeoutInvitation.SessionType = EAccelByteV2SessionType::PartySession;
	TimeoutInvitation.RecipientId = PlayerId;
	TimeoutInvitation.Session = TimeoutInvitationPtr->Session;
	TimeoutInvitation.SenderId = TimeoutInvitationPtr->SenderId;
	TimeoutInvitation.ExpiredAt = TimeoutInvitationPtr->ExpiredAt;

	const FString& InviteIdToBeRemoved = TimeoutInvitationPtr->Session.GetSessionIdStr();
	RemoveInviteById(InviteIdToBeRemoved);
	TriggerOnV2SessionInviteTimeoutReceivedDelegates(PlayerId.ToSharedRef().Get(), TimeoutInvitation, EAccelByteV2SessionType::PartySession);
	//TODO:
	//const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
};

void FOnlineSessionV2AccelByte::OnKickedFromPartySessionNotification(FAccelByteModelsV2PartyUserKickedEvent KickedEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *KickedEvent.PartyID);

	// First check if the inbound party ID matches one stored locally
	FNamedOnlineSession* Session = GetNamedSessionById(*KickedEvent.PartyID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session kicked notification as we do not have the session stored locally!"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session kicked notification as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session kicked notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session kicked notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// If the party session we're kicked from matches the local session found, destroy the session, which will also
	// trigger leave if needed
	TriggerOnKickedFromSessionDelegates(Session->SessionName);
	DestroySession(Session->SessionName, FOnDestroySessionCompleteDelegate(), true);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2PartySessionKickedPayload PartySessionKickedPayload{};
		PartySessionKickedPayload.UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerId)->GetAccelByteId();
		PartySessionKickedPayload.PartySessionId = KickedEvent.PartyID;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionKickedPayload>(PartySessionKickedPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnFindPartySessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2PartyInvitedEvent InviteEvent)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; bWasSuccessful: %s; SessionId: %s; SenderId: %s"), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful), *Result.GetSessionIdStr(), *InviteEvent.SenderID);

	if (!bWasSuccessful)
	{
		AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to notify player about invite to session as we failed to query data about session from backend!"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to notify player about invite to session as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as we could not get a unique ID for player at index %d!"), LocalUserNum);
		return;
	}

	// Find sender ID in the backend session data so that we can get their platform info for the ID
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Result.Session.SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our session's information instance is invalid!"));
		return;
	}

	const TSharedPtr<FAccelByteModelsV2PartySession> PartySessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	if (!ensure(PartySessionData.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as our local copy of the backend session data is invalid!"));
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
		FAccelByteModelsV2SessionUser* FoundSender = PartySessionData->Members.FindByPredicate([InviteEvent](const FAccelByteModelsV2SessionUser& User) {
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
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle party session invite notification as the ID of the sender of the invite is invalid!"));
		return;
	}

	FAccelByteTimeManagerWPtr TimeManagerWPtr = nullptr;
	const FAccelByteInstancePtr AccelByteInstance =  AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
	if(AccelByteInstance.IsValid())
	{
		TimeManagerWPtr = AccelByteInstance->GetTimeManager();
	}

	FOnlineSessionInviteAccelByte NewInvite{TimeManagerWPtr};
	NewInvite.SessionType = EAccelByteV2SessionType::PartySession;
	NewInvite.RecipientId = PlayerId;
	NewInvite.Session = Result;
	NewInvite.SenderId = SenderId;
	NewInvite.ExpiredAt = InviteEvent.ExpiredAt;
	UpdateSessionInvite(NewInvite);

	TriggerOnSessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), AccelByteSubsystemPtr->GetAppId(), Result);
	TriggerOnV2SessionInviteReceivedDelegates(PlayerId.ToSharedRef().Get(), SenderId.ToSharedRef().Get(), NewInvite);
	TriggerOnInviteListUpdatedDelegates();
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnPartySessionMembersChangedNotification(FAccelByteModelsV2PartyMembersChangedEvent MembersChangedEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s; JoinerId: %s"), *MembersChangedEvent.Session.ID, *MembersChangedEvent.JoinerID);

	FNamedOnlineSession* Session = GetNamedSessionById(MembersChangedEvent.Session.ID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with with members changed event as we do not have the session stored locally!"));
		return;
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2PartySession>(MembersChangedEvent.Session));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnPartySessionUpdatedNotification(FAccelByteModelsV2PartySession UpdatedPartySession, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *UpdatedPartySession.ID);

	FNamedOnlineSession* Session = GetNamedSessionById(UpdatedPartySession.ID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with with new attributes as we do not have the session stored locally!"));
		return;
	}

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2PartySession>(UpdatedPartySession));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnPartySessionInviteRejectedNotification(FAccelByteModelsV2PartyUserRejectedEvent RejectEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionId: %s"), LocalUserNum, *RejectEvent.PartyID);

	FNamedOnlineSession* Session = GetNamedSessionById(RejectEvent.PartyID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as we do not have that session stored locally!"), *RejectEvent.PartyID);
		return;
	}

	// Grab session data to update members there
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as the locally stored session has invalid session info!"), *RejectEvent.PartyID);
		return;
	}

	TSharedPtr<FAccelByteModelsV2PartySession> SessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	if (!ensure(SessionData.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as the locally stored session has invalid backend data!"), *RejectEvent.PartyID);
		return;
	}

	SessionData->Members = RejectEvent.Members;

	bool bHasInvitedPlayersChanged = false;
	bool bHasJoinedMembersChanged = false;
	SessionInfo->UpdatePlayerLists(bHasJoinedMembersChanged, bHasInvitedPlayersChanged);
	
	if (bHasInvitedPlayersChanged)
	{
		TriggerOnSessionInvitesChangedDelegates(Session->SessionName);
	}

	FUniqueNetIdPtr UniqueRejectID = nullptr;
	FAccelByteModelsV2SessionUser* FoundSender = RejectEvent.Members.FindByPredicate([RejectEvent](const FAccelByteModelsV2SessionUser& User) {
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

	SessionInfo->UpdateLeaderId();

	TriggerOnUpdateSessionCompleteDelegates(Session->SessionName, true);
	TriggerOnSessionUpdateReceivedDelegates(Session->SessionName);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::UnbindLobbyMulticastDelegate()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""))

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unbind lobby multicast delegates as our AccelByte subsystem is invalid"));
		return;
	}

	// It seems currently this interface only support single local user, not ideal to hardcode this but this is the simplest way for now.
	const int32 LocalPlayerNum = 0;
	const FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalPlayerNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Failed to unbind multicast delegate for session updates as local player '%d' has an invalid API client!"), LocalPlayerNum);
		return;
	}

	const auto Lobby = ApiClient->GetLobbyApi().Pin();
	if (!Lobby.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Failed to unbind multicast delegate for session updates as local player '%d' has an invalid Lobby API!"), LocalPlayerNum);
		return;
	}

	Lobby->RemoveV2GameSessionInviteCanceledNotifDelegate(OnGameSessionInviteCanceledHandle);
	Lobby->RemoveV2PartyInviteCanceledNotifDelegate(OnPartyInviteCanceledHandle);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
}

void FOnlineSessionV2AccelByte::UpdateSessionMembers(FNamedOnlineSession* Session, const TArray<FAccelByteModelsV2SessionUser>& PreviousMembers, const bool bHasInvitedPlayersChanged)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Session->GetSessionIdStr());

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not diff members for session '%s' as the local session has invalid session info!"), *Session->GetSessionIdStr());
		return;
	}

	const TSharedPtr<FAccelByteModelsV2BaseSession> SessionData = SessionInfo->GetBackendSessionData();
	if (!ensure(SessionData.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not diff members for session '%s' as its session info has invalid backend data!"), *Session->GetSessionIdStr());
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
		const FAccelByteModelsV2SessionUser* PreviousMember = PreviousMembers.FindByPredicate([NewMember](const FAccelByteModelsV2SessionUser& Member) {
			return Member.ID == NewMember.ID;
		});

		// If this user's status hasn't changed, then we want to ensure that we have this player in the RegisteredPlayers
		// array. If they are already in the array, then skip. Otherwise, register them.
		if (PreviousMember != nullptr && PreviousMember->StatusV2 == NewMember.StatusV2)
		{
			const bool bIsJoined = (PreviousMember->StatusV2 == EAccelByteV2SessionMemberStatus::JOINED || PreviousMember->StatusV2 == EAccelByteV2SessionMemberStatus::CONNECTED);
			const bool bNeedsRegistration = bIsJoined && !Session->RegisteredPlayers.ContainsByPredicate([PreviousMember](const FUniqueNetIdRef& PlayerId) {
				FUniqueNetIdAccelByteUserRef AccelBytePlayerId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);
				return AccelBytePlayerId->GetAccelByteId() == PreviousMember->ID;
			});

			if (bNeedsRegistration)
			{
				RegisterJoinedSessionMember(Session, *PreviousMember);
			}

			continue;
		}

		const bool bIsJoinStatus = NewMember.StatusV2 == EAccelByteV2SessionMemberStatus::JOINED || NewMember.StatusV2 == EAccelByteV2SessionMemberStatus::CONNECTED;
		const bool bIsLeaveStatus = NewMember.StatusV2 == EAccelByteV2SessionMemberStatus::LEFT || NewMember.StatusV2 == EAccelByteV2SessionMemberStatus::KICKED || NewMember.StatusV2 == EAccelByteV2SessionMemberStatus::DROPPED;

		if (bIsJoinStatus)
		{
			RegisterJoinedSessionMember(Session, NewMember);
		}
		else if (bIsLeaveStatus)
		{
			UnregisterLeftSessionMember(Session, NewMember);
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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

		// #NOTE: OnSessionParticipantsChange deprecated in 5.2 and removed in 5.5
#if !(ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5)
		TriggerOnSessionParticipantsChangeDelegates(Session->SessionName, JoinedUserId.ToSharedRef().Get(), true);
#endif

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
		TriggerOnSessionParticipantJoinedDelegates(Session->SessionName, JoinedUserId.ToSharedRef().Get());
#endif
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

	// Both on OnSessionParticipantRemoved and OnSessionParticipantsChanged are deprecated in 5.2 and removed in 5.5
#if !(ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5)
	// Participant removed delegate seems to be only in 4.27+, guarding it so we don't fail to compile on other engine versions.
#if ENGINE_MAJOR_VERSION >= 5 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27)
	TriggerOnSessionParticipantRemovedDelegates(Session->SessionName, LeftUserId.ToSharedRef().Get());
#endif
	TriggerOnSessionParticipantsChangeDelegates(Session->SessionName, LeftUserId.ToSharedRef().Get(), false);
#endif // !(ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5)

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
	const EOnSessionParticipantLeftReason LeftReason = LeftMember.StatusV2 == EAccelByteV2SessionMemberStatus::KICKED ? EOnSessionParticipantLeftReason::Kicked : EOnSessionParticipantLeftReason::Left;
	TriggerOnSessionParticipantLeftDelegates(Session->SessionName, LeftUserId.ToSharedRef().Get(), LeftReason);
#endif

	// If the user that has left is ourselves, then we want to destroy this session as well. Developers will need to listen
	// to OnDestroySessionComplete to ensure that they are catching being booted out of a session from the backend.
	if (Session->LocalOwnerId.IsValid() && LeftUserId.ToSharedRef().Get() == Session->LocalOwnerId.ToSharedRef().Get())
	{
		DestroySession(Session->SessionName);
	}
}

void FOnlineSessionV2AccelByte::OnMatchmakingStartedNotification(FAccelByteModelsV2StartMatchmakingNotif MatchmakingStartedNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; TicketID: %s; PartyID: %s; Namespace: %s; MatchPool: %s"),
		LocalUserNum, *MatchmakingStartedNotif.TicketID, *MatchmakingStartedNotif.PartyID, *MatchmakingStartedNotif.Namespace, *MatchmakingStartedNotif.MatchPool);

	bool bIsTicketCanceled = CanceledTicketIds.ContainsByPredicate([MatchmakingStartedNotif](const FString& TicketId) {
		return TicketId.Equals(MatchmakingStartedNotif.TicketID);
	});

	if (bIsTicketCanceled)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Ignoring matchmaking start notification as we explicitly canceled this ticket."));
		return;
	}

	if (CurrentMatchmakingSearchHandle.IsValid())
	{
		// Since we already have a matchmaking search handle, we don't need to do anything else here and can bail
		TriggerOnMatchmakingStartedDelegates();
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("CurrentMatchmakingSearchHandle is valid"));
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle matchmaking started notification as our AccelByte subsystem is invalid"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle matchmaking start notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr LocalPlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(LocalPlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle matchmaking start notification as a unique ID could not be found for player at index %d!"), LocalUserNum);
		return;
	}

	// This validation is used for party member.
	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		// If we don't have a valid session search instance stored, then the party leader has started matchmaking, in which we
		// need to create our own search handle to get notifications from matchmaking to join later
		CurrentMatchmakingSearchHandle = MakeShared<FOnlineSessionSearchAccelByte>();
		CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::InProgress;
		CurrentMatchmakingSearchHandle->SearchingPlayerId = LocalPlayerId;
		CurrentMatchmakingSearchHandle->TicketId = MatchmakingStartedNotif.TicketID;
		CurrentMatchmakingSearchHandle->MatchPool = MatchmakingStartedNotif.MatchPool;
		
		if(MatchmakingStartedNotif.TicketInformation.Storage.JsonObject.IsValid())
		{
			CurrentMatchmakingSearchHandle->SearchStorage = MatchmakingStartedNotif.TicketInformation.Storage.JsonObject.ToSharedRef();
		}
		else
		{
			CurrentMatchmakingSearchHandle->SearchStorage = MakeShared<FJsonObject>();
		}

		if (MatchmakingStartedNotif.TicketInformation.CrossPlayEnabled)
		{
			CurrentMatchmakingSearchHandle->CrossPlatforms = MatchmakingStartedNotif.TicketInformation.CrossPlatforms;
		}

		// Check for session name in notification storage
		FName SessionName = NAME_GameSession;
		const bool bHasSessionNameInStorage = MatchmakingStartedNotif.TicketInformation.Storage.JsonObject.IsValid() &&
			MatchmakingStartedNotif.TicketInformation.Storage.JsonObject->HasField(STORAGE_SESSION_NAME);
		if(bHasSessionNameInStorage)
		{
			SessionName = FName(MatchmakingStartedNotif.TicketInformation.Storage.JsonObject->GetStringField(STORAGE_SESSION_NAME));
		}
		
		CurrentMatchmakingSearchHandle->SearchingSessionName = SessionName;

		TriggerOnMatchmakingStartedDelegates();
	}
	else
	{
		// Since we're using user attribute (from party leader) to enable the cross play
		// This will populate the matchmaking search handle for the party member.
		if (MatchmakingStartedNotif.TicketInformation.CrossPlayEnabled)
		{
			CurrentMatchmakingSearchHandle->CrossPlatforms = MatchmakingStartedNotif.TicketInformation.CrossPlatforms;
		}
	}

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2MatchmakingStartedPayload MatchmakingStartedPayload{};
		MatchmakingStartedPayload.UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(LocalPlayerId)->GetAccelByteId();
		MatchmakingStartedPayload.MatchTicketId = MatchmakingStartedNotif.TicketID;
		MatchmakingStartedPayload.PartySessionId = MatchmakingStartedNotif.PartyID;
		MatchmakingStartedPayload.MatchPool = MatchmakingStartedNotif.MatchPool;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2MatchmakingStartedPayload>(MatchmakingStartedPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnMatchmakingMatchFoundNotification(FAccelByteModelsV2MatchFoundNotif MatchFoundEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *MatchFoundEvent.Id);

	if (bFindMatchmakingGameSessionByIdInProgress)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("FindMatchmakingGameSessionById already in progress"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as our AccelByte subsystem is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as a unique ID could not be found for player at index %d!"), LocalUserNum);
		return;
	}

	FApiClientPtr ApiClientPtr = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	TSharedPtr<FAccelByteKey> LobbyLockKey;
	if (ApiClientPtr.IsValid())
	{
		auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
		if (Lobby.IsValid())
		{
			LobbyLockKey = Lobby->LockNotifications();
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Invalid Lobby from API Client"));
		}
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as ApiClient is invalid for player at index %d!"), LocalUserNum);
		return;
	}

	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(MatchFoundEvent.Id);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as a unique ID could not be created for the match's session ID!"));
		return;
	}

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle match found notification as we do not have a valid search handle to save results to!"));
		return;
	}

	// stop match ticket status polling
	StopMatchTicketCheckPoll();

	// start match invite status polling if we don't have the invite yet
	const bool bInviteAlreadyReceived = SessionInvites.ContainsByPredicate([&MatchFoundEvent](const FOnlineSessionInviteAccelByte& Invite)
	{
		return Invite.Session.GetSessionIdStr() == MatchFoundEvent.Id;
	});
	
	if(!bInviteAlreadyReceived)
	{
		StartSessionInviteCheckPoll(PlayerId, MatchFoundEvent.Id);
	}

	SetFindMatchmakingGameSessionByIdInProgress(true);
	const FOnSingleSessionResultCompleteDelegate OnFindMatchmakingGameSessionByIdCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnFindMatchmakingGameSessionByIdComplete);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get(), PlayerId.ToSharedRef().Get(), SessionUniqueId.ToSharedRef().Get(), OnFindMatchmakingGameSessionByIdCompleteDelegate, LobbyLockKey);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnMatchmakingExpiredNotification(FAccelByteModelsV2MatchmakingExpiredNotif MatchmakingExpiredNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Ticket id %s"), *MatchmakingExpiredNotif.TicketID);

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("no matchmaking search handle found"));
		return;
	}
	
	if(CurrentMatchmakingSearchHandle->TicketId != MatchmakingExpiredNotif.TicketID)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Ticket Id did not match with current matchmaking process"));
		return;
	}

	FName SessionName = CurrentMatchmakingSearchHandle->SearchingSessionName;
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
	TriggerOnMatchmakingExpiredDelegates(CurrentMatchmakingSearchHandle);
	CurrentMatchmakingSearchHandle.Reset();
	CurrentMatchmakingSessionSettings = {};

	// stop match status check polling
	StopMatchTicketCheckPoll();

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Matchmaking ticket expired!"));
	TriggerOnMatchmakingCompleteDelegates(SessionName, false);
}

void FOnlineSessionV2AccelByte::OnMatchmakingCanceledNotification(FAccelByteModelsV2MatchmakingCanceledNotif MatchmakingCanceledNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Make sure we're matchmaking first
	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to handle matchmaking canceled notification as no matchmaking search handle was found!"));
		return;
	}

	// Check if this notification is the one we are actively matchmaking
	if(CurrentMatchmakingSearchHandle->TicketId != MatchmakingCanceledNotif.TicketID)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Received matchmaking canceled notification with ID %s, but differs from current active ticket %s. Ignoring."), *MatchmakingCanceledNotif.TicketID, *CurrentMatchmakingSearchHandle->TicketId);
		return;
	}

	// Add ticket ID to the canceled array
	AddCanceledTicketId(CurrentMatchmakingSearchHandle->TicketId);

	// Clear current matchmaking handle, and mark as failed
	FName SessionName = CurrentMatchmakingSearchHandle->SearchingSessionName;
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Failed;
	CurrentMatchmakingSearchHandle.Reset();
	CurrentMatchmakingSessionSettings = {};

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Matchmaking ticket canceled!"));
	TriggerOnMatchmakingCompleteDelegates(SessionName, false);
	TriggerOnMatchmakingCanceledDelegates();

	const FOnlineErrorAccelByte Error = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, FOnlineError::CreateError(
		ONLINE_ERROR_NAMESPACE, EOnlineErrorResult::Canceled, "", FText::FromString(MatchmakingCanceledNotif.Reason)));
	TriggerOnMatchmakingCanceledReasonDelegates(Error);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnFindMatchmakingGameSessionByIdComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	SetFindMatchmakingGameSessionByIdInProgress(false);

	if (!CurrentMatchmakingSearchHandle.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finish matchmaking as we do not have a valid search handle instance!"));

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
		
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session information for match found from matchmaking! Considering matchmaking failed from here!"));
		TriggerOnMatchmakingCompleteDelegates(SessionName, false);

		return;
	}

	CurrentMatchmakingSearchHandle->SearchResults.Emplace(Result);
	CurrentMatchmakingSearchHandle->SearchState = EOnlineAsyncTaskState::Done;

	TriggerOnMatchmakingCompleteDelegates(CurrentMatchmakingSearchHandle->SearchingSessionName, true);

	// After triggering this delegate, we will want to reset the current matchmaking search handle so that we're able to re-enter matchmaking at a later time
	CurrentMatchmakingSearchHandle.Reset();

	// Attempt to handle auto join session. If result is true, then we have successfully joined this game session through auto join.
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Result.Session.SessionInfo);
	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionInfo = SessionInfo->GetBackendSessionDataAsGameSession();
	if (!GameSessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to get game session information to attempt auto join processing!"));
		return;
	}

	if (HandleAutoJoinGameSession(GameSessionInfo.ToSharedRef().Get(), LocalUserNum))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Auto-joined matchmaking session successfully"));
	}
	else
	{
		// Not an error to not auto join, so logging a blank success trace
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	}
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to setup accelbyte p2p connection as our AccelByte subsystem is invalid"));
		return;
	}
	
	// Start by registering the AccelByte P2P socket subsystem as the default
	FAccelByteNetworkUtilitiesModule::Get().RegisterDefaultSocketSubsystem();

	// Now, grab the API client for the user that is setting up P2P and run set up in the module
	const AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalPlayerId);
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
	const NetworkUtilities::EAccelByteP2PConnectionStatus& Status, FName SessionName, EOnlineSessionP2PConnectedAction Action)
{
	UE_LOG_AB(Log, TEXT("FOnlineSessionV2AccelByte::OnICEConnectionComplete"))
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

	AsyncTask(ENamedThreads::GameThread, [SessionInterface = SharedThis(this), Action, SessionName, Result, PeerId] {
		if(Action == EOnlineSessionP2PConnectedAction::Join)
        {
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
			check(Session != nullptr);

			TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
			check(SessionInfo.IsValid());
			const TTuple<FString, int32> PeerChannel = FAccelByteNetworkUtilitiesModule::ExtractPeerAndChannel(PeerId);
			SessionInfo->SetP2PChannel(PeerChannel.Value);
			
			SessionInterface->TriggerOnJoinSessionCompleteDelegates(SessionName, Result);
        }
		else if (Action == EOnlineSessionP2PConnectedAction::Update)
		{
			SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
			SessionInterface->TriggerOnSessionUpdateReceivedDelegates(SessionName);
		}
	});
}

void FOnlineSessionV2AccelByte::ConnectToDSHub(const FString& ServerName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("ServerName: %s"), *ServerName);

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	// First, register any delegates that we want to listen to from DS hub
	const AccelByte::GameServerApi::FOnServerClaimedNotification OnServerClaimedNotificationDelegate = AccelByte::GameServerApi::FOnServerClaimedNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnServerClaimedNotification);
	ServerApiClient->ServerDSHub.SetOnServerClaimedNotificationDelegate(OnServerClaimedNotificationDelegate);

	const AccelByte::GameServerApi::FOnV2BackfillProposalNotification OnV2BackfillProposalNotificationDelegate = AccelByte::GameServerApi::FOnV2BackfillProposalNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnV2BackfillProposalNotification);
	ServerApiClient->ServerDSHub.SetOnV2BackfillProposalNotificationDelegate(OnV2BackfillProposalNotificationDelegate);

	const AccelByte::GameServerApi::FOnV2BackfillTicketExpiredNotification OnV2BackfillTicketExpiredNotificationDelegate = AccelByte::GameServerApi::FOnV2BackfillTicketExpiredNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnV2BackfillTicketExpiredNotification);
	ServerApiClient->ServerDSHub.SetOnV2BackfillTicketExpiredNotificationDelegate(OnV2BackfillTicketExpiredNotificationDelegate);

	const AccelByte::GameServerApi::FOnV2SessionMemberChangedNotification OnV2SessionMemberChangedNotification = AccelByte::GameServerApi::FOnV2SessionMemberChangedNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnV2DsSessionMemberChangedNotification);
	ServerApiClient->ServerDSHub.SetOnV2SessionMemberChangedNotificationDelegate(OnV2SessionMemberChangedNotification);

	const AccelByte::GameServerApi::FOnV2SessionEndedNotification OnV2SessionEndedNotification = AccelByte::GameServerApi::FOnV2SessionEndedNotification::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnV2DsSessionEndedNotification);
	ServerApiClient->ServerDSHub.SetOnV2SessionEndedNotificationDelegate(OnV2SessionEndedNotification);

	const AccelByte::GameServerApi::FConnectSuccess OnDSHubConnectSuccessNotificationDelegate = AccelByte::GameServerApi::FConnectSuccess::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnDSHubConnectSuccessNotification);
	ServerApiClient->ServerDSHub.SetOnConnectSuccess(OnDSHubConnectSuccessNotificationDelegate);

	const AccelByte::GameServerApi::FConnectionClosed OnDSHubConnectionClosedNotificationDelegate = AccelByte::GameServerApi::FConnectionClosed::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnDSHubConnectionClosedNotification);
	ServerApiClient->ServerDSHub.SetOnConnectionClosed(OnDSHubConnectionClosedNotificationDelegate);
	
	const AccelByte::GameServerApi::FConnectError OnDSHubFailedToConnectDelegate = AccelByte::GameServerApi::FConnectError::CreateThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnDSHubFailedToConnect);
	ServerApiClient->ServerDSHub.SetOnConnectError(OnDSHubFailedToConnectDelegate);

	ServerApiClient->ServerDSHub.OnReconnectAttemptedMulticastDelegate().AddThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnDSHubReconnectAttempted);

	ServerApiClient->ServerDSHub.OnMassiveOutageMulticastDelegate().AddThreadSafeSP(SharedThis(this), &FOnlineSessionV2AccelByte::OnDSHubMassiveOutageEvent);

	// Finally, connect to the DS hub websocket
	ServerApiClient->ServerDSHub.Connect(ServerName);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::DisconnectFromDSHub()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	if(!ServerApiClient.IsValid())
	{
		return;
	}

	// Unbind any delegates that we want to stop listening to DS hub for
	ServerApiClient->ServerDSHub.SetOnServerClaimedNotificationDelegate(AccelByte::GameServerApi::FOnServerClaimedNotification());
	ServerApiClient->ServerDSHub.SetOnV2BackfillProposalNotificationDelegate(AccelByte::GameServerApi::FOnV2BackfillProposalNotification());
	ServerApiClient->ServerDSHub.SetOnV2SessionMemberChangedNotificationDelegate(AccelByte::GameServerApi::FOnV2SessionMemberChangedNotification());
	ServerApiClient->ServerDSHub.SetOnV2SessionEndedNotificationDelegate(AccelByte::GameServerApi::FOnV2SessionEndedNotification());
	ServerApiClient->ServerDSHub.SetOnConnectSuccess(AccelByte::GameServerApi::FConnectSuccess());
	ServerApiClient->ServerDSHub.SetOnConnectionClosed(AccelByte::GameServerApi::FConnectionClosed());
	ServerApiClient->ServerDSHub.SetOnConnectError(AccelByte::GameServerApi::FConnectError());
	
	// Finally, disconnect the DS hub websocket
	ServerApiClient->ServerDSHub.Disconnect();

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnServerClaimedNotification(const FAccelByteModelsServerClaimedNotification& Notification)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Notification.Session_id)

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// Also bail if we already have a game session set up
	if (GetNamedSession(NAME_GameSession) != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle session claimed notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	// Take the session ID we got for the server claim and retrieve session data for it
	GetServerClaimedSession(NAME_GameSession, Notification.Session_id);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsDSClaimedPayload DSClaimedPayload{};
		DSClaimedPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
		DSClaimedPayload.GameSessionId = Notification.Session_id;
		PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSClaimedPayload>(DSClaimedPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnV2BackfillProposalNotification(const FAccelByteModelsV2MatchmakingBackfillProposalNotif& Notification)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("BackfillTicketId: %s; ProposalId: %s"), *Notification.BackfillTicketID, *Notification.ProposalID);

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle backfill proposal notification as our AccelByte subsystem is invalid"));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	TriggerOnBackfillProposalReceivedDelegates(Notification);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsDSBackfillProposalReceivedPayload DSBackfillProposalReceivedPayload{};
		DSBackfillProposalReceivedPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
		DSBackfillProposalReceivedPayload.BackfillTicketId = Notification.BackfillTicketID;
		DSBackfillProposalReceivedPayload.ProposalId = Notification.ProposalID;
		DSBackfillProposalReceivedPayload.MatchPool = Notification.MatchPool;
		DSBackfillProposalReceivedPayload.GameSessionId = Notification.MatchSessionID;
		DSBackfillProposalReceivedPayload.ProposedTeams = Notification.ProposedTeams;
		DSBackfillProposalReceivedPayload.AddedTickets = Notification.AddedTickets;
		PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSBackfillProposalReceivedPayload>(DSBackfillProposalReceivedPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnV2BackfillTicketExpiredNotification(
	const FAccelByteModelsV2MatchmakingBackfillTicketExpireNotif& Notification)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("TicketId: %s"), *Notification.TicketId);

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Backfill ticket expired notification only works for dedicated server"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle backfill ticket expired notification as our AccelByte subsystem is invalid"));
		return;
	}

	FNamedOnlineSession* Session = GetNamedSession(NAME_GameSession);
	if (!ensureAlways(Session != nullptr))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to get session instance to update stored backfill ticket ID!"));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	Session->SessionSettings.Remove(SETTING_MATCHMAKING_BACKFILL_TICKET_ID);

	TriggerOnBackfillTicketExpiredReceivedDelegates(NAME_GameSession,Notification);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsDSBackfillTicketExpiredPayload DSBackfillTicketExpiredPayload{};
		DSBackfillTicketExpiredPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
		DSBackfillTicketExpiredPayload.TicketId = Notification.TicketId;
		PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSBackfillTicketExpiredPayload>(DSBackfillTicketExpiredPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnV2DsSessionMemberChangedNotification(const FAccelByteModelsV2GameSession& Notification)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Notification.ID);

	FNamedOnlineSession* Session = GetNamedSessionById(Notification.ID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update session with members changed event as we do not have the session stored locally!"));
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle session member changed notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	// TODO: Potentially unnecessary memory allocation
	EnqueueBackendDataUpdate(Session->SessionName, MakeShared<FAccelByteModelsV2GameSession>(Notification));
	
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsDSMemberChangedNotifReceivedPayload DSMemberChangedNotifReceived{};
		DSMemberChangedNotifReceived.PodName = ServerApiClient->ServerDSM.GetServerName();
		DSMemberChangedNotifReceived.GameSessionId = Notification.ID;
		DSMemberChangedNotifReceived.Members = Notification.Members;
		DSMemberChangedNotifReceived.Teams = Notification.Teams;
		PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSMemberChangedNotifReceivedPayload>(DSMemberChangedNotifReceived));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnV2DsSessionEndedNotification(const FAccelByteModelsSessionEndedNotification& Notification)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Notification.Session_id);
	
	FNamedOnlineSession* Session = GetNamedSessionById(Notification.Session_id);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not get the session. Failed to disconnect from AMS!"));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	if (ServerApiClient->ServerAMS.IsConnected() || !ServerApiClient->ServerDSM.GetServerName().IsEmpty())
	{
		TriggerOnV2SessionEndedDelegates(Session->SessionName);
	}
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnDSHubConnectSuccessNotification()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle DSHub connect success notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsDSHubConnectedPayload DSHubConnectedPayload{};
		DSHubConnectedPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
		PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSHubConnectedPayload>(DSHubConnectedPayload));
	}
}

void FOnlineSessionV2AccelByte::OnDSHubFailedToConnect(const FString& Reason)
{
	TriggerAccelByteOnDSHubFailedToConnectDelegates(Reason);
}

void FOnlineSessionV2AccelByte::OnDSHubConnectionClosedNotification(int32 StatusCode, const FString & Reason, bool bWasClean)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle DShub connectino closed notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsDSHubDisconnectedPayload DSHubDisconnectedPayload{};
		DSHubDisconnectedPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
		DSHubDisconnectedPayload.StatusCode = StatusCode;
		PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSHubDisconnectedPayload>(DSHubDisconnectedPayload));
	}
}

void FOnlineSessionV2AccelByte::OnDSHubReconnectAttempted(const FReconnectAttemptInfo& Info)
{
	TriggerAccelByteOnDSHubReconnectAttemptedDelegates(Info);
}

void FOnlineSessionV2AccelByte::OnDSHubMassiveOutageEvent(const FMassiveOutageInfo& Info)
{
	TriggerAccelByteOnDSHubMassiveOutageEventDelegates(Info);
}

void FOnlineSessionV2AccelByte::OnAMSConnectedCallback(bool bWasSuccessful, const FOnlineErrorAccelByte& ErrorMessage)
{
	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	ServerApiClient->ServerAMS.OnReconnectAttemptedMulticastDelegate().AddThreadSafeSP(AsShared()
			, &FOnlineSessionV2AccelByte::OnAMSReconnectAttempted);

	ServerApiClient->ServerAMS.OnMassiveOutageMulticastDelegate().AddThreadSafeSP(AsShared()
		, &FOnlineSessionV2AccelByte::OnAMSMassiveOutageEvent);
}

void FOnlineSessionV2AccelByte::OnAMSReconnectAttempted(const FReconnectAttemptInfo& Info)
{
	TriggerAccelByteOnAMSReconnectAttemptedDelegates(Info);
}

void FOnlineSessionV2AccelByte::OnAMSMassiveOutageEvent(const FMassiveOutageInfo& Info)
{
	TriggerAccelByteOnAMSMassiveOutageEventDelegates(Info);
}

void FOnlineSessionV2AccelByte::OnSessionStorageChangedNotification(FAccelByteModelsV2SessionStorageChangedEvent Notification, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *Notification.SessionID);

	const FNamedOnlineSession* Session = GetNamedSessionById(Notification.SessionID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update local session storage with session storage changed event as we do not have the session stored locally!"));
		return;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (SessionInfo.IsValid())
	{
		if (Notification.IsLeader)
		{
			SessionInfo.Get()->SetSessionLeaderStorage(Notification.StorageChanges);
			TriggerOnSessionLeaderStorageUpdateReceivedDelegates(Session->SessionName);
		}
		else
		{
			const FUniqueNetIdAccelByteUserRef ActorUniqueNetId = FUniqueNetIdAccelByteUser::Create(Notification.ActorUserID);
			SessionInfo.Get()->SetSessionMemberStorage(ActorUniqueNetId, Notification.StorageChanges);
			TriggerOnSessionMemberStorageUpdateReceivedDelegates(Session->SessionName, ActorUniqueNetId.Get());
		}
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	}
	else
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update local session storage with session storage changed event as local session info is invalid!"));
	}
}

void FOnlineSessionV2AccelByte::SendReadyToAMS()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send Ready Message to AMS"));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}
	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	ServerApiClient->ServerAMS.SendReadyMessage();

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetDSTimeout(int32 NewTimeout)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send DS Timeout Message to AMS"));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	if (NewTimeout > 1)
	{
		ServerApiClient->ServerAMS.SetDSTimeout(NewTimeout);
	}
	else
	{
		ServerApiClient->ServerAMS.SetDSTimeout(1);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::ResetDSTimeout()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send DS Reset Timeout Message to AMS"));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerAMS.ResetDSTimeout();

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::DisconnectFromAMS()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	if(!ServerApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	// Unbind any delegates that we want to stop listening to AMS for
	ServerApiClient->ServerAMS.SetOnAMSDrainReceivedDelegate(AccelByte::GameServerApi::ServerAMS::FOnAMSDrainReceived());

	// Finally, disconnect the DS AMS websocket
	ServerApiClient->ServerAMS.Disconnect();

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::InitMetricExporter(const FString& Address, uint16 Port, uint16 IntervalSeconds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerMetric.Initialize(Address, Port, IntervalSeconds);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetMetricLabel(const FString& Key, const FString& Value)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerMetric.SetLabel(Key, Value);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::EnqueueMetric(const FString& Key, double Value)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerMetric.EnqueueMetric(Key, Value);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::EnqueueMetric(const FString& Key, int32 Value)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerMetric.EnqueueMetric(Key, Value);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::EnqueueMetric(const FString& Key, const FString& Value)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerMetric.EnqueueMetric(Key, Value);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetOptionalMetricsEnabled(bool Enable)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerMetric.SetOptionalMetricsEnabled(Enable);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetMetricCollector(const TSharedPtr<IAccelByteStatsDMetricCollector>& Collector)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();

	ServerApiClient->ServerMetric.SetStatsDMetricCollector(Collector);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::InitializePlayerAttributes(const FUniqueNetId& LocalPlayerId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalPlayerId: %s"), *LocalPlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initialize player attribute as our AccelByte subsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteInitializePlayerAttributes>(AccelByteSubsystemPtr.Get(), LocalPlayerId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("ServerQueryGameSessions only works for Dedicated Server"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query game session as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2>(AccelByteSubsystemPtr.Get(), Request, Offset, Limit);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineSessionV2AccelByte::ServerQueryPartySessions(const FAccelByteModelsV2QueryPartiesRequest& Request, int64 Offset, int64 Limit)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("ServerQueryPartySessions only works for Dedicated Server"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query party session as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2>(AccelByteSubsystemPtr.Get(), Request, Offset, Limit);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::AddCanceledTicketId(const FString& TicketId)
{
	LastCanceledTicketIdAddedTimeSeconds = FPlatformTime::Seconds();
	CanceledTicketIds.Add(TicketId);
}

bool FOnlineSessionV2AccelByte::IsServerUseAMS() const
{
	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	
	return !ServerApiClient->ServerSettings->DSId.IsEmpty() && IsRunningDedicatedServer();
}

bool FOnlineSessionV2AccelByte::SendDSSessionReady()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("This method only can be used for dedicated server"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send ds session ready as our AccelByte subsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendDSSessionReady>(AccelByteSubsystemPtr.Get(), true);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sending server ready to session service"));
	return true;
}

void FOnlineSessionV2AccelByte::HandleUserLogoutCleanUp(const FUniqueNetId& LocalUserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserId: %s"), *LocalUserId.ToDebugString());
	
	// If current matchmaking search was invoked by this user, reset the handle
	if (CurrentMatchmakingSearchHandle.IsValid() && (CurrentMatchmakingSearchHandle->SearchingPlayerId.ToSharedRef().Get()) == LocalUserId)
	{
		CurrentMatchmakingSearchHandle.Reset();
	}

	StopMatchTicketCheckPoll();

	// Gather all named sessions that the player was an owner of to destroy
	TArray<FName> SessionsToDestroy{};
	{
		FScopeLock ScopeLock(&SessionLock);
		for (TPair<FName, TSharedPtr<FNamedOnlineSession>>& SessionEntry : Sessions)
		{
			FUniqueNetIdPtr LocalOwnerId = SessionEntry.Value->LocalOwnerId;
			if (!LocalOwnerId.IsValid())
			{
				continue;
			}

#if !AB_USE_V2_SESSIONS
// Empty statement, do nothing.
#else
			TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionEntry.Value->SessionInfo);
			if(SessionInfo.IsValid())
			{
				StopSessionInviteCheckPoll(LocalUserId.AsShared(), SessionInfo->GetSessionId().ToString());
			}
#endif

			StopSessionServerCheckPoll(LocalUserId.AsShared(), SessionEntry.Key);

			if (LocalOwnerId.ToSharedRef().Get() == LocalUserId)
			{
				SessionsToDestroy.Emplace(SessionEntry.Key);
			}
		}
	}

	for (const FName& SessionName : SessionsToDestroy)
	{
		DestroySession(SessionName);
	}

	// Remove any restored session that was restored by the logged out player
	RestoredSessions.RemoveAll([&LocalUserId](const FOnlineRestoredSessionAccelByte& RestoredSession) {
		if(!RestoredSession.LocalOwnerId.IsValid())
		{
			return false;
		}
		else
		{
			return RestoredSession.LocalOwnerId.ToSharedRef().Get() == LocalUserId;
		}
	});

	// Remove any session invite that was recieved by the logged out player
	SessionInvites.RemoveAll([&LocalUserId](const FOnlineSessionInviteAccelByte& SessionInvite) {
		return SessionInvite.RecipientId.ToSharedRef().Get() == LocalUserId;
	});

	// Remove cached player attributes from map
	UserIdToPlayerAttributesMap.Remove(LocalUserId.AsShared());

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnAMSDrain()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// Bail if this is not a dedicated server
	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return;
	}

	TriggerOnAMSDrainReceivedDelegates();

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineSessionV2AccelByte::HandleAutoJoinGameSession(const FAccelByteModelsV2GameSession& GameSession
	, const int32 LocalUserNum
	, bool bHasDsError
	, FString DsErrorString)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Checking if added to auto join game session from backend"));

	if (!GameSession.Configuration.AutoJoin)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Game session ID %s is not auto joinable"), *GameSession.ID);
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, GameSession.ID);
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to hadnle auto join game session as our AccelByte subsystem is invalid"));
		return false;
	}

	// Check if this game session update is auto joined in the backend
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle posible auto joined game session as our identity interface is invalid!"));
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, GameSession.ID);
		return false;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle posible auto joined game session as local user is invalid!"));
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, GameSession.ID);
		return false;
	}

	const FUniqueNetIdAccelByteUserRef UserUniqueNetId = FUniqueNetIdAccelByteUser::Create(*LocalUserId.Get());
	const FString UserID = UserUniqueNetId.Get().GetAccelByteId();

	const FAccelByteModelsV2SessionUser* FoundMember = GameSession.Members.FindByPredicate(
		[UserID](const FAccelByteModelsV2SessionUser& Member)
		{
			return Member.ID == UserID && Member.StatusV2 == EAccelByteV2SessionMemberStatus::JOINED;
		});

	if (FoundMember == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Not marked as joined to the session on the backend, ignoring auto join for now."), *GameSession.ID);
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, GameSession.ID);
		return false;
	}

	// Check if we have already stored a local session that corresponds with this data
	FNamedOnlineSession* FoundSession = GetNamedSessionById(GameSession.ID);
	if (FoundSession != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Ignoring auto join for session as we already are joined locally. SessionID: %s"), *GameSession.ID);
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, GameSession.ID);
		return false;
	}

	FOnlineSession AutoJoinedSession;
	ConstructGameSessionFromBackendSessionModel(GameSession, AutoJoinedSession);
		
	FName SessionName = NAME_GameSession;
	const bool bHasSessionNameInStorage = GameSession.Storage.Leader.JsonObject.IsValid() &&
		GameSession.Storage.Leader.JsonObject->HasField(STORAGE_SESSION_NAME);
	if (bHasSessionNameInStorage)
	{
		SessionName = FName(GameSession.Storage.Leader.JsonObject->GetStringField(STORAGE_SESSION_NAME));
	}
		
	FNamedOnlineSession* NewSession = AddNamedSession(SessionName, AutoJoinedSession);
	NewSession->SessionState = EOnlineSessionState::Pending;
	NewSession->LocalOwnerId = UserUniqueNetId;

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(NewSession->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		// Remove pending session as this auto join attempt was not valid
		RemoveNamedSession(SessionName);

		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle auto joined game session as failed to cast FOnlineSessionInfo to FOnlineSessionInfoAccelByteV2"));
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, GameSession.ID);
		return false;
	}

	PartySessionStorageLocalUserManager.PastSessionManager.InsertPastSessionID(LocalUserId, GameSession.ID);
	UpdatePartySessionStorageWithPastSessionInfo(UserUniqueNetId);
	TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::Success);

	if (bHasDsError)
	{
		TriggerOnSessionServerErrorDelegates(SessionName, DsErrorString);
		UE_LOG_AB(Warning, TEXT("Auto joined session had DS error occur! Error message: %s"), *DsErrorString);
	}

	// session is auto joined, we won't be in invited state
	StopSessionInviteCheckPoll(UserUniqueNetId, GameSession.ID);
	
	if(SessionInfo->HasConnectionInfo())
	{
		TriggerOnSessionServerUpdateDelegates(SessionName);
	}
	// if session doesn't have session ID yet AND this is a game session that has a DS,
	// then we start our server check poll timer
	else if (GameSession.Configuration.Type == EAccelByteV2SessionConfigurationServerType::DS)
	{
		UE_LOG_AB(Verbose, TEXT("Trigger session server check poll from handle auto join game session"));
		StartSessionServerCheckPoll(UserUniqueNetId, SessionName);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Successfully synced auto join game session from backend"));
	TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, true, GameSession.ID);
	return true;
}

void FOnlineSessionV2AccelByte::OnGameSessionInviteRejectedNotification(FAccelByteModelsV2GameSessionUserRejectedEvent RejectEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; SessionId: %s"), LocalUserNum, *RejectEvent.SessionID);

	FNamedOnlineSession* Session = GetNamedSessionById(RejectEvent.SessionID);
	if (Session == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as we do not have that session stored locally!"), *RejectEvent.SessionID);
		return;
	}

	// Grab session data to update members there
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as the locally stored session has invalid session info!"), *RejectEvent.SessionID);
		return;
	}

	TSharedPtr<FAccelByteModelsV2PartySession> SessionData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	if (!ensure(SessionData.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not update invites for session '%s' as the locally stored session has invalid backend data!"), *RejectEvent.SessionID);
		return;
	}

	SessionData->Members = RejectEvent.Members;

	bool bHasInvitedPlayersChanged = false;
	bool bHasJoinedMembersChanged = false;
	SessionInfo->UpdatePlayerLists(bHasJoinedMembersChanged, bHasInvitedPlayersChanged);

	if (bHasInvitedPlayersChanged)
	{
		TriggerOnSessionInvitesChangedDelegates(Session->SessionName);
	}

	FUniqueNetIdPtr UniqueRejectID = nullptr;
	FAccelByteModelsV2SessionUser* FoundSender = RejectEvent.Members.FindByPredicate([RejectEvent](const FAccelByteModelsV2SessionUser& User) {
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

	SessionInfo->UpdateLeaderId();

	TriggerOnUpdateSessionCompleteDelegates(Session->SessionName, true);
	TriggerOnSessionUpdateReceivedDelegates(Session->SessionName);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::SetFindMatchmakingGameSessionByIdInProgress(const bool State)
{
	bFindMatchmakingGameSessionByIdInProgress = State;
}

void FOnlineSessionV2AccelByte::SetSessionInvitationGetInfoInProgress(const FString& SessionId)
{
	FScopeLock Lock(&SessionInvitationGetInfoInProgressLock);
	SessionInvitationGetInfoInProgress.Emplace(SessionId);
}

void FOnlineSessionV2AccelByte::RemoveSessionInvitationGetInfoInProgress(const FString& SessionId)
{
	FScopeLock Lock(&SessionInvitationGetInfoInProgressLock);
	SessionInvitationGetInfoInProgress.Remove(SessionId);
}

bool FOnlineSessionV2AccelByte::SessionInvitationGetInfoInProgressExist(const FString& SessionId)
{
	FScopeLock Lock(&SessionInvitationGetInfoInProgressLock);
	return SessionInvitationGetInfoInProgress.Contains(SessionId);
}

void FOnlineSessionV2AccelByte::UpdateSessionInvite(const FOnlineSessionInviteAccelByte& NewInvite)
{
	bool bSessionInviteExist{false};
	int32 SessionInviteFoundIndex{0};
	for (const FOnlineSessionInviteAccelByte& SessionInvite : SessionInvites)
	{
		if (SessionInvite.Session.GetSessionIdStr() == NewInvite.Session.GetSessionIdStr())
		{
			bSessionInviteExist = true;
			break;
		}
		
		SessionInviteFoundIndex++;
	}

	if (bSessionInviteExist && SessionInvites.IsValidIndex(SessionInviteFoundIndex))
	{
		SessionInvites[SessionInviteFoundIndex] = NewInvite;
	}
	else
	{
		SessionInvites.Emplace(NewInvite);
	}
}

bool FOnlineSessionV2AccelByte::RemoveSessionInvite(const FString& ID)
{
	const int32 SessionInviteIndexToRemove = SessionInvites.IndexOfByPredicate([&ID](const FOnlineSessionInviteAccelByte& SessionInvite)
		{
			return SessionInvite.Session.Session.GetSessionIdStr() == ID;
		});
	
	if (SessionInviteIndexToRemove != INDEX_NONE)
	{
		SessionInvites.RemoveAt(SessionInviteIndexToRemove);
		return true;
	}

	return false;
}

void FOnlineSessionV2AccelByte::OnGameSessionInviteCanceledNotification(const FAccelByteModelsV2GameSessionInviteCanceledEvent& CanceledEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Local user %d received invite canceled for game session %s"), LocalUserNum, *CanceledEvent.SessionID);

	TriggerOnSessionInviteCanceledDelegates(CanceledEvent.SessionID);
	
	if(RemoveSessionInvite(CanceledEvent.SessionID))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Game session invite successfully removed from cache"))
		return;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Game session invite not found in cache"))
}

void FOnlineSessionV2AccelByte::OnPartySessionInviteCanceledNotification(const FAccelByteModelsV2PartyInviteCanceledEvent& CanceledEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Local user %d received invite canceled for party %s"), LocalUserNum, *CanceledEvent.PartyID);

	TriggerOnSessionInviteCanceledDelegates(CanceledEvent.PartyID);
	
	if(RemoveSessionInvite(CanceledEvent.PartyID))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Party invite successfully removed from cache"))
		return;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Party invite not found in cache"))
}

void FOnlineSessionV2AccelByte::OnPartySessionCreatedNotification(const FAccelByteModelsV2PartyCreatedEvent& CreatedEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Local user %d received party created notif for party %s"), LocalUserNum, *CreatedEvent.PartyID);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to hadnle party session created notification as our AccelByte subsystem is invalid"));
		return;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register the created party as our identity interface is invalid!"));
		return;
	}

	const auto PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if(!PlayerId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register the created party as we could not get a local user ID for player with index '%d'!"), LocalUserNum);
		return;
	}
	TSharedRef<FOnlineSessionInfoAccelByteV2> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV2>(CreatedEvent.PartyID);
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2PartyById>(AccelByteSubsystemPtr.Get()
		, *PlayerId.Get()
		, SessionInfo->GetSessionId()
		, FOnSingleSessionResultCompleteDelegate::CreateLambda([SessionInterface = SharedThis(this), PlayerId](int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result) 
			{
				if (bWasSuccessful)
				{
					SessionInterface->TriggerOnPartySessionCreatedDelegates(*PlayerId.Get(), Result.Session);
					return;
				}
			}));
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created Party Notification"));
}
 
void FOnlineSessionV2AccelByte::OnGameSessionJoinedNotification(FAccelByteModelsV2GameSessionUserJoinedEvent JoinedEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *JoinedEvent.SessionID);
 
	// Check if we have already joined this session locally
	FNamedOnlineSession* ExistingSession = GetNamedSessionById(JoinedEvent.SessionID);
	if (ExistingSession != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Already joined to session locally, ignoring. SessionId: %s"), *JoinedEvent.SessionID);
		return;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle game session joined notification as our AccelByte subsystem is invalid"));
		return;
	}
 
	// Retrieve player unique ID from identity interface
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle session joined notification as our identity interface is invalid!"));
		return;
	}
 
	const FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!ensure(PlayerId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle session joined notification as a unique ID could not be found for player at index %d!"), LocalUserNum);
		return;
	}

	FApiClientPtr ApiClientPtr = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	TSharedPtr<FAccelByteKey> LobbyLockKey;
	if (ApiClientPtr.IsValid())
	{
		auto Lobby = ApiClientPtr->GetLobbyApi().Pin();
		if (Lobby.IsValid())
		{
			LobbyLockKey = Lobby->LockNotifications();
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Invalid Lobby from API Client"));
		}
	}
 
	// Create session unique ID from notification string ID
	const FUniqueNetIdPtr SessionUniqueId = CreateSessionIdFromString(JoinedEvent.SessionID);
	if (!ensure(SessionUniqueId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle session joined notification as a unique ID could not be created from the session's string ID! SessionId: %s"), *JoinedEvent.SessionID);
		return;
	}
 
	// We have not joined this session, so attempt to query data for it
	//
	// #TODO Maxwell: Unlike other session notifications, the join notification does not return the full session data
	// in the notification payload. This means that to handle auto join, we will always need to query this data first
	// check if auto join is enabled, and then perform the local join if it is. If a session join somehow occurred
	// _outside_ of normal OSS flow without auto join enabled, this would unfortunately slip through our local state.
	const FOnSingleSessionResultCompleteDelegate OnFindJoinedGameSessionByIdCompleteDelegate =
		FOnSingleSessionResultCompleteDelegate::CreateThreadSafeSP(SharedThis(this)
			, &FOnlineSessionV2AccelByte::OnFindJoinedGameSessionByIdComplete
			, JoinedEvent.SessionID);
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteFindV2GameSessionById>(AccelByteSubsystemPtr.Get()
		, PlayerId.ToSharedRef().Get()
		, SessionUniqueId.ToSharedRef().Get()
		, OnFindJoinedGameSessionByIdCompleteDelegate
		, LobbyLockKey);
 
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnGameSessionEndedNotification(FAccelByteModelsV2GameSessionEndedEvent EndedEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionId: %s"), *EndedEvent.SessionID);

	// Check if we have already joined this session locally
	FNamedOnlineSession* ExistingSession = GetNamedSessionById(EndedEvent.SessionID);
	if (ExistingSession != nullptr)
	{
		RemoveNamedSession(ExistingSession->SessionName);
		TriggerOnDestroySessionCompleteDelegates(ExistingSession->SessionName, true);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("The local session will be cleaned to sync with the backend state. SessionId: %s"), *EndedEvent.SessionID);
		return;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnFindJoinedGameSessionByIdComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& FoundSession, FString SessionId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; bWasSuccessful: %s"), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful));

	if (!bWasSuccessful)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find game session information for session joined notification"));
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, SessionId);
		return;
	}

	// Check if we still have not joined this session, such as if another notification had come in and triggered auto join
	FNamedOnlineSession* ExistingSession = GetNamedSessionById(FoundSession.GetSessionIdStr());
	if (ExistingSession != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found auto join enabled session, but we have already joined it locally. Ignoring."));
		// Not triggering delegate here as it should have already been done by the previous auto join call
		return;
	}

	// We have not already joined this session, grab the session data structure and pass to the auto join processing
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(FoundSession.Session.SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("SessionInfo instance was invalid"));
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, SessionId);
		return;
	}

	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionData = SessionInfo->GetBackendSessionDataAsGameSession();
	if (!GameSessionData.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game session data from found session"));
		TriggerOnAutoJoinGameSessionCompleteDelegates(LocalUserNum, false, SessionId);
		return;
	}

	HandleAutoJoinGameSession(GameSessionData.ToSharedRef().Get(), LocalUserNum);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineSessionV2AccelByte::OnCheckGameSessionDataComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& FoundSession)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; bWasSuccessful: %s"), LocalUserNum, LOG_BOOL_FORMAT(bWasSuccessful));

	if (!bWasSuccessful)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find game session information for session joined notification"));
		return;
	}

	// Check if we still have not joined this session, such as if another notification had come in and triggered auto join
	FNamedOnlineSession* ExistingSession = GetNamedSessionById(FoundSession.GetSessionIdStr());
	if (ExistingSession != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found auto join enabled session, but we have already joined it locally. Ignoring."));
		return;
	}

	// We have not already joined this session, grab the session data structure and pass to the auto join processing
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(FoundSession.Session.SessionInfo);
	if (!SessionInfo.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("SessionInfo instance was invalid"));
		return;
	}

	TSharedPtr<FAccelByteModelsV2GameSession> GameSessionData = SessionInfo->GetBackendSessionDataAsGameSession();
	if (!GameSessionData.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get game session data from found session"));
		return;
	}

	if (HandleAutoJoinGameSession(GameSessionData.ToSharedRef().Get(), LocalUserNum))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Log, TEXT("Synced auto joined game session from backend"));
		return;
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineSessionV2AccelByte::UpdateDSInformation(FName SessionName
	, FAccelByteModelsGameSessionUpdateDSInformationRequest const& NewDSInformation
	, FOnUpdateDSInformationComplete const& CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		// #NOTE: Normally would want to execute next tick, but cannot since subsystem is invalid
		CompletionDelegate.ExecuteIfBound(SessionName
			, ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), FText::FromString(TEXT("update-ds-information-subsystem-invalid"))));
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update DS information: Unable to pin AccelByte subsystem instance"));
		return false;
	}

	const FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AccelByteSubsystemPtr->ExecuteNextTick([SessionName, CompletionDelegate]() {
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
			const EOnlineErrorResult ErrorResult = EOnlineErrorResult::NoGameSession;
#else
			// UE versions lower than 5.3 do not have the NoGameSession error, so for those use the less descriptive InvalidParams
			const EOnlineErrorResult ErrorResult = EOnlineErrorResult::InvalidParams;
#endif // ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3

			CompletionDelegate.ExecuteIfBound(SessionName
				, ONLINE_ERROR(ErrorResult, FString(), FText::FromString(TEXT("update-ds-information-no-session-found"))));
		});
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update DS information: session '%s' does not exist"), *SessionName.ToString());
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateDSInformation>(AccelByteSubsystemPtr.Get()
		, SessionName
		, NewDSInformation
		, CompletionDelegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineSessionV2AccelByte::ConnectToThirdPartyDSHub(const FString& ServerName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("ServerName: %s"), *ServerName);

	if (!IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to connect to DSHub: only intended for dedicated server instances"));
		return;
	}

	FServerApiClientPtr ServerApiClient = GetServerApiClient();
	if (!ServerApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to connect to DSHub: server API client invalid"));
		return;
	}

	if (ServerApiClient->ServerDSHub.IsConnected())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("DSHub already connected"));
		return;
	}

	ConnectToDSHub(ServerName);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
