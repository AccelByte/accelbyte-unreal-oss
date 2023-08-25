// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlinePartyInterfaceAccelByte.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineError.h"
#include "Api/AccelByteLobbyApi.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineSessionSettings.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteCreateV1Party.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteJoinV1Party.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteLeaveV1Party.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteSendV1PartyInvite.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteKickV1PartyMember.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelBytePromoteV1PartyLeader.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteUpdateV1PartyData.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteRestoreV1Parties.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteGetV1PartyInviteInfo.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteAddJoinedV1PartyMember.h"
#include "AsyncTasks/PartyV1/OnlineAsyncTaskAccelByteGetV1PartyCode.h"
#include "OnlineSubsystemUtils.h"

// Some delegates require reasons as to why the delegate might have failed, for this case, this is a constant for when
// we do not support the current method that the developer is attempting to call
#define UNSUPPORTED_METHOD_REASON -10000

bool WarnForUsingV1PartyWithV2Sessions()
{
#if AB_USE_V2_SESSIONS
	UE_LOG_AB(Warning, TEXT("Tried to use party calls in FOnlinePartySystemAccelByte while using V2 sessions! Party logic has moved from the party interface to the session interface while using V2 sessions. Please update to using session interface for party management!"));
	return true;
#endif

	return false;
}

FOnlinePartyIdAccelByte::FOnlinePartyIdAccelByte(const FString& InIdStr)
	: IdStr(InIdStr)
{
}

const uint8* FOnlinePartyIdAccelByte::GetBytes() const
{
	// Just return the FString TCHAR array casted as a uint8 array. On one hand, this seems unsafe to just do a straight
	// cast of this, however this is what the FUniqueNetIdString implementation does, so I assume this is fine. 
	return reinterpret_cast<const uint8*>(IdStr.GetCharArray().GetData());
}

int32 FOnlinePartyIdAccelByte::GetSize() const
{
	// Just like FUniqueNetIdString::GetSize, this will just return the current size in bytes of the underlying string using the
	// size of the character type stored as well as the number of characters that the string has.
	return IdStr.GetCharArray().Num() * IdStr.GetCharArray().GetTypeSize();
}

bool FOnlinePartyIdAccelByte::IsValid() const
{
	// Just like in FUniqueNetIdAccelByte::IsValid, the only real check we want to do is to see if the length of our
	// underlying string matches that of a UUID v4 string without hyphens
	bool bIsIdCorrectLength = IdStr.Len() == ACCELBYTE_ID_LENGTH;

	UE_LOG_AB(VeryVerbose, TEXT("Validation result for PartyId '%s': Successful=%s; ID Length=%d; Expected ID Length: %d"), *IdStr, LOG_BOOL_FORMAT(bIsIdCorrectLength), IdStr.Len(), ACCELBYTE_ID_LENGTH);
	return bIsIdCorrectLength;
}

FString FOnlinePartyIdAccelByte::ToString() const
{
	return IdStr;
}

FString FOnlinePartyIdAccelByte::ToDebugString() const
{
	return IdStr;
}

FOnlinePartyAccelByte::FOnlinePartyAccelByte(const TSharedRef<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe>& InOwningInterface, const FString& InPartyId, const FString& InInviteToken, const FPartyConfiguration& InPartyConfiguration, const TSharedRef<const FUniqueNetIdAccelByteUser>& InLeaderId, const TSharedRef<FOnlinePartyData>& InPartyData, const FOnlinePartyTypeId InPartyTypeId)
	: FOnlineParty(MakeShared<FOnlinePartyIdAccelByte>(InPartyId), InPartyTypeId)
	, OwningInterface(InOwningInterface)
	, InviteToken(InInviteToken)
	, PartyConfiguration(MakeShared<const FPartyConfiguration>(InPartyConfiguration))
	, PartyData(InPartyData)
{
	SetState(EPartyState::Active);
	LeaderId = InLeaderId;
}

bool FOnlinePartyAccelByte::CanLocalUserInvite(const FUniqueNetId& LocalUserId) const
{
	// There is no restrictions on what users can invite someone to a party, all users in a party can invite others
	return true;
}

bool FOnlinePartyAccelByte::IsJoinable() const
{
	// Since we only can join parties once we have an invite code for them, this will always return false
	return false;
}

TSharedRef<const FPartyConfiguration> FOnlinePartyAccelByte::GetConfiguration() const
{
	return PartyConfiguration;
}

const uint32 FOnlinePartyAccelByte::GetMemberCount() const
{
	return UserIdToPartyMemberMap.Num();
}

TSharedRef<FOnlinePartyAccelByte> FOnlinePartyAccelByte::CreatePartyFromPartyInfo(const TSharedRef<const FUniqueNetIdAccelByteUser> LocalUserId, const TSharedRef<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface, const FAccelByteModelsPartyJoinResponse& PartyInfo, const TArray<TSharedRef<FAccelByteUserInfo>>& PartyMemberInfo, const TSharedRef<FOnlinePartyData>& InPartyData, const FString& PartyCode, const FAccelByteModelsBulkUserStatusNotif& InPartyMemberStatus)
{
	FAccelByteModelsInfoPartyResponse NewPartyInfo;
	NewPartyInfo.Code = PartyInfo.Code;
	NewPartyInfo.PartyId = PartyInfo.PartyId;
	NewPartyInfo.InvitationToken = PartyInfo.InvitationToken;
	NewPartyInfo.Invitees = PartyInfo.Invitees;
	NewPartyInfo.LeaderId = PartyInfo.LeaderId;
	NewPartyInfo.Members = PartyInfo.Members;
	
	return CreatePartyFromPartyInfo(LocalUserId, PartyInterface, NewPartyInfo, PartyMemberInfo, InPartyData, PartyCode, InPartyMemberStatus);
}

TSharedRef<FOnlinePartyAccelByte> FOnlinePartyAccelByte::CreatePartyFromPartyInfo(const TSharedRef<const FUniqueNetIdAccelByteUser> LocalUserId, const TSharedRef<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface, const FAccelByteModelsInfoPartyResponse& PartyInfo, const TArray<TSharedRef<FAccelByteUserInfo>>& PartyMemberInfo, const TSharedRef<FOnlinePartyData>& InPartyData, const FString& PartyCode, const FAccelByteModelsBulkUserStatusNotif& InPartyMemberStatus)
{
	// Fill out a basic config of flags that we know/support for this party
	FPartyConfiguration Config;
	Config.bChatEnabled = true;
	Config.bIsAcceptingMembers = true;

	// Find the leader in the members array so that we can give the correct ID for them
	const TSharedRef<FAccelByteUserInfo>* FoundPartyLeader = PartyMemberInfo.FindByPredicate([&PartyInfo](const TSharedRef<FAccelByteUserInfo>& Member) {
		return Member->Id->GetAccelByteId() == PartyInfo.LeaderId;
	});

	FUniqueNetIdAccelByteUserRef LeaderCompositeId = FUniqueNetIdAccelByteUser::Invalid();
	if (FoundPartyLeader == nullptr)
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = PartyInfo.LeaderId;
		LeaderCompositeId = FUniqueNetIdAccelByteUser::Create(CompositeId);
	}
	else
	{
		LeaderCompositeId = (*FoundPartyLeader)->Id.ToSharedRef();
	}

	TSharedRef<FOnlinePartyAccelByte> Party = MakeShared<FOnlinePartyAccelByte>(PartyInterface, PartyInfo.PartyId, PartyInfo.InvitationToken, Config, LeaderCompositeId, InPartyData);
	for (const TSharedRef<FAccelByteUserInfo>& Member : PartyMemberInfo)
	{
		TSharedRef<FOnlinePartyMemberAccelByte> PartyMember = MakeShared<FOnlinePartyMemberAccelByte>(Member->Id.ToSharedRef(), Member->DisplayName);
		const FAccelByteModelsUserStatusNotif* MemberStatus = InPartyMemberStatus.Data.FindByPredicate([&Member](FAccelByteModelsUserStatusNotif const& Status)
		{
			return Member->Id->GetAccelByteId().Equals(Status.UserID);
		});
		if(MemberStatus != nullptr)
		{
			EMemberConnectionStatus NewMemberStatusConnection = EMemberConnectionStatus::Uninitialized;
			switch(MemberStatus->Availability)
			{
			case EAvailability::Online:
			case EAvailability::Busy:
			case EAvailability::Away:
				NewMemberStatusConnection = EMemberConnectionStatus::Connected;
				break;
			case EAvailability::Offline:
			case EAvailability::Invisible:
			default:
				NewMemberStatusConnection = EMemberConnectionStatus::Disconnected;
				break;
			}
			PartyMember->SetMemberConnectionStatus(NewMemberStatusConnection);
		}
		Party->AddMember(LocalUserId, PartyMember);
	}
	
	if (!PartyCode.IsEmpty())
	{
		Party->SetPartyCode(PartyCode);
	}

	// We also want to add the users that have pending invites to this party to our invites array
	for (const FString& InviteeId : PartyInfo.Invitees)
	{
		// Create composite ID for the invited user, we don't need to have a full composite here
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = InviteeId;
		TSharedRef<const FUniqueNetIdAccelByteUser> InviteeNetId = FUniqueNetIdAccelByteUser::Create(CompositeId);

		// NOTE(Maxwell, 7/27/2021): Backend does not return the ID of the player that invited the user, only the IDs of the
		// users invited. As a temporary measure, use the ID of the current user as the inviter ID.
		Party->AddUserToInvitedPlayers(LocalUserId, LocalUserId, InviteeNetId);
	}

	Party->AddPlayerCrossplayPreferenceAndPlatform(LocalUserId);

	return Party;
}

void FOnlinePartyAccelByte::AddMember(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<FOnlinePartyMemberAccelByte>& Member)
{
	TSharedRef<const FUniqueNetId> NewMemberId = Member->GetUserId();
	UserIdToPartyMemberMap.Add(FUniqueNetIdAccelByteUser::CastChecked(NewMemberId), Member);
	OwningInterface->TriggerOnPartyMemberJoinedDelegates(LocalUserId.Get(), PartyId.Get(), NewMemberId.Get());
}

void FOnlinePartyAccelByte::AddUserToInvitedPlayers(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InviterUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InvitedUserId)
{
	InvitedPlayers.Add(FInvitedPlayerPair(InviterUserId, InvitedUserId));
	OwningInterface->TriggerOnPartyInvitesChangedDelegates(LocalUserId.Get());
}

TSharedPtr<const FOnlinePartyMemberAccelByte> FOnlinePartyAccelByte::GetMember(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId) const
{
	for (auto UserIdToPartyMemberPair : UserIdToPartyMemberMap)
	{
		TSharedRef<const FUniqueNetIdAccelByteUser> UserIdKey = UserIdToPartyMemberPair.Key;
		if (UserIdKey->GetAccelByteId() == UserId->GetAccelByteId())
		{
			return UserIdToPartyMemberPair.Value;
		}
	}
	return nullptr;
}

bool FOnlinePartyAccelByte::RemoveMember(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& RemovedUserId, const EMemberExitedReason& ExitReason)
{
	bool bIsMemberFound = false;
	// First, try and find the party member in the map of members, if we find them then we can remove
	for (auto UserIdToPartyMemberPair : UserIdToPartyMemberMap)
	{
		TSharedRef<const FUniqueNetIdAccelByteUser> UserIdKey = UserIdToPartyMemberPair.Key;
		if (UserIdKey->GetAccelByteId() == RemovedUserId->GetAccelByteId())
		{
			// Only commit to removing the user from the party data if we are the leader, otherwise there will be duplicate requests
			// to remove the user from the party storage on the backend
			if (LeaderId.ToSharedRef().Get() == LocalUserId.Get())
			{
				RemovePlayerCrossplayPreferenceAndPlatform(LocalUserId, RemovedUserId);
			}

			UserIdToPartyMemberMap.Remove(UserIdKey);
			bIsMemberFound = true;
			break;
		}
	}
	OwningInterface->TriggerOnPartyMemberExitedDelegates(LocalUserId.Get(), PartyId.Get(), RemovedUserId.Get(), ExitReason);
	return bIsMemberFound;
}

void FOnlinePartyAccelByte::RemoveInvite(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InvitedUserId, const EPartyInvitationRemovedReason& PartyInviteRemoveReason)
{
	int32 FoundInvitedUserIndex = INDEX_NONE;
	do
	{
		// First, try and find the invited user in the party invites array
		FoundInvitedUserIndex = InvitedPlayers.IndexOfByPredicate([InvitedUserId](const FInvitedPlayerPair& Pair) {
			return Pair.Value.Get() == InvitedUserId.Get();
		});
		// If we found the invited user, then we want to remove them from the party invites array and trigger delegates
		if (FoundInvitedUserIndex != INDEX_NONE)
		{
			const FInvitedPlayerPair InvitedPlayer = InvitedPlayers[FoundInvitedUserIndex];
			InvitedPlayers.RemoveAt(FoundInvitedUserIndex);
			
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
			OwningInterface->TriggerOnPartyInviteRemovedDelegates(LocalUserId.Get(), PartyId.Get(), InvitedPlayer.Key.Get(), PartyInviteRemoveReason);
#endif
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27)
			TArray<IOnlinePartyJoinInfoConstRef> Invites;
			FString SenderDisplayName;
			OwningInterface->GetPendingInvites(InvitedUserId.Get(), Invites);
			for (const auto& Invite : Invites)
			{
				if (Invite->GetSourceUserId().Get() == LocalUserId.Get())
				{
					SenderDisplayName = Invite->GetSourceDisplayName();
				}
			}
			TSharedRef<FOnlinePartyJoinInfoAccelByte> JoinInfoRef = MakeShared<FOnlinePartyJoinInfoAccelByte>(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId), InvitedPlayer.Key, SenderDisplayName);
			OwningInterface->TriggerOnPartyInviteRemovedExDelegates(LocalUserId.Get(), JoinInfoRef.Get(), PartyInviteRemoveReason);
#endif
		}
	}
	while (FoundInvitedUserIndex != INDEX_NONE);
}

TArray<FOnlinePartyMemberConstRef> FOnlinePartyAccelByte::GetAllMembers() const
{
	TArray<FOnlinePartyMemberConstRef> MembersArray;
	for (const TPair<TSharedRef<const FUniqueNetIdAccelByteUser>, TSharedRef<FOnlinePartyMemberAccelByte>>& KV : UserIdToPartyMemberMap)
	{
		MembersArray.Add(KV.Value);
	}
	return MembersArray;
}

TArray<TSharedRef<const FUniqueNetId>> FOnlinePartyAccelByte::GetAllPendingInvitedUsers() const
{
	TArray<TSharedRef<const FUniqueNetId>> OutArray;
	OutArray.Reserve(InvitedPlayers.Num());
	for (const FInvitedPlayerPair& InvitedPlayer : InvitedPlayers)
	{
		OutArray.Add(InvitedPlayer.Value);
	}
	return OutArray;
}

TSharedRef<const FOnlinePartyData> FOnlinePartyAccelByte::GetPartyData() const
{
	return PartyData;
}

void FOnlinePartyAccelByte::SetPartyData(TSharedRef<FOnlinePartyData> InPartyData)
{
	PartyData = InPartyData;
}

void FOnlinePartyAccelByte::AddPlayerCrossplayPreferenceAndPlatform(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId)
{
	// Get the crossplay attribute for the current user by grabbing their user account from the identity interface
	FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(IOnlineSubsystem::Get(ACCELBYTE_SUBSYSTEM));
	if (Subsystem == nullptr)
	{
		UE_LOG_AB(Warning, TEXT("Cannot associate current player's crossplay preference and platform to their current party as subsystem instance is invalid!"));
		return;
	}

	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Cannot associate current player's crossplay preference and platform to their current party as the identity interface is invalid!"));
		return;
	}

	TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(LocalUserId.Get());
	if (!UserAccount.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Cannot associate current player's crossplay preference and platform to their current party as their user account instance is invalid!"));
		return;
	}

	FString CrossplayBoolStr;
	if (!UserAccount->GetUserAttribute(TEXT("crossplay"), CrossplayBoolStr))
	{
		UE_LOG_AB(Warning, TEXT("Cannot associate current player's crossplay preference and platform to their current party as their crossplay preference could not be found on their user account!"));
		return;
	}

	// Create a JSON object to store the platform and crossplay preference for the current player
	TSharedRef<FJsonObject> CurrentPlayerPreferences = MakeShared<FJsonObject>();
	CurrentPlayerPreferences->SetStringField(CROSSPLAY_OBJECT_PLAYER_PLATFORM_FIELD, Subsystem->GetSimplifiedNativePlatformName());
	CurrentPlayerPreferences->SetBoolField(CROSSPLAY_OBJECT_PLAYER_CROSSPLAY_FIELD, CrossplayBoolStr == TEXT("true"));

	// Attempt to grab the current crossplay platform mapping for the current party members, if this already exists, then
	// we just want to grab the object from the variant data and set our crossplay prefs on that. Otherwise, we want to
	// create a new object.
	TSharedRef<const FOnlinePartyData> CurrentPartyData = GetPartyData();
	TSharedPtr<FJsonObject> CrossplayPlatformMapObject;
	FVariantData OutVariantData;
	if (CurrentPartyData->GetAttribute(CROSSPLAY_OBJECT_NAME, OutVariantData))
	{
		OutVariantData.GetValue(CrossplayPlatformMapObject);
	}
	else
	{
		CrossplayPlatformMapObject = MakeShared<FJsonObject>();
	}

	// Map the current user by their AccelByte ID to their platform and crossplay preference
	CrossplayPlatformMapObject->SetObjectField(LocalUserId->GetAccelByteId(), CurrentPlayerPreferences);

	// Create a copy of the old party data instance, add our updated crossplay platform map object to the copy
	// and then set the copy as the new party data
	TSharedRef<FOnlinePartyData> NewPartyData = MakeShared<FOnlinePartyData>(CurrentPartyData.Get());
	NewPartyData->SetAttribute(CROSSPLAY_OBJECT_NAME, FVariantData(CrossplayPlatformMapObject.ToSharedRef()));
	SetPartyData(NewPartyData);

	// Finally, send a request to update the party data on the backend
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), NAME_Game, PartyData.Get());
#else
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), PartyData.Get());
#endif
}

void FOnlinePartyAccelByte::RemovePlayerCrossplayPreferenceAndPlatform(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& UserToRemove)
{
	// Start by grabbing the current party data and attempting to get the crossplay platform map from it. If we cannot do that
	// then we want to abort the operation.
	TSharedRef<const FOnlinePartyData> CurrentPartyData = GetPartyData();
	TSharedPtr<FJsonObject> CrossplayPlatformMapObject;
	FVariantData OutVariantData;
	if (CurrentPartyData->GetAttribute(CROSSPLAY_OBJECT_NAME, OutVariantData))
	{
		OutVariantData.GetValue(CrossplayPlatformMapObject);
	}
	else
	{
		// Cannot remove from the mapping, abort
		return;
	}

	// Remove the crossplay/platform preference mapped to the user specified
	CrossplayPlatformMapObject->RemoveField(UserToRemove->GetAccelByteId());

	// Create a copy of the old party data instance, add our updated crossplay platform map object to the copy
	// and then set the copy as the new party data
	TSharedRef<FOnlinePartyData> NewPartyData = MakeShared<FOnlinePartyData>(CurrentPartyData.Get());
	NewPartyData->SetAttribute(CROSSPLAY_OBJECT_NAME, FVariantData(CrossplayPlatformMapObject.ToSharedRef()));
	SetPartyData(NewPartyData);

	// Finally, send a request to update the party data on the backend
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), NAME_Game, PartyData.Get());
#else
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), PartyData.Get());
#endif
}

void FOnlinePartyAccelByte::AddPlayerAcceptedTicketId(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const FString& TicketId, const FString& MatchId)
{
	TSharedRef<const FOnlinePartyData> CurrentPartyData = GetPartyData();
	TSharedRef<FOnlinePartyData> NewPartyData = MakeShared<FOnlinePartyData>(CurrentPartyData.Get());
	SetPartyData(NewPartyData);

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), NAME_Game, PartyData.Get());
#else
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), PartyData.Get());
#endif
}

void FOnlinePartyAccelByte::RemovePlayerAcceptedTicketId(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId)
{
	TSharedRef<const FOnlinePartyData> CurrentPartyData = GetPartyData();
	TSharedRef<FOnlinePartyData> NewPartyData = MakeShared<FOnlinePartyData>(CurrentPartyData.Get());
	SetPartyData(NewPartyData);

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), NAME_Game, PartyData.Get());
#else
	OwningInterface->UpdatePartyData(LocalUserId.Get(), PartyId.Get(), PartyData.Get());
#endif
}

void FOnlinePartyAccelByte::SetPartyCode(const FString& PartyCode)
{
	// Create a copy of the old party data instance, and append our party code to it, then set it
	TSharedRef<FOnlinePartyData> NewPartyData = MakeShared<FOnlinePartyData>(PartyData.Get());
	NewPartyData->SetAttribute(PARTYDATA_PARTYCODE_ATTR, PartyCode);
	SetPartyData(NewPartyData);
}

bool FOnlinePartyAccelByte::IsCrossplayParty()
{
	FVariantData OutVariantData;
	if (!PartyData->GetAttribute(CROSSPLAY_OBJECT_NAME, OutVariantData))
	{
		// Cannot confirm that this party is crossplay, so just return false
		return false;
	}

	TSharedPtr<FJsonObject> CrossplayObject;
	OutVariantData.GetValue(CrossplayObject);
	if (!CrossplayObject.IsValid())
	{
		// Still cannot confirm that this party is crossplay, so return false
		return false;
	}

	// Iterate through each value in the crossplay object to get the object associated with the user and then to
	// subsequently check their crossplay preference, if that preference is false, then this party isn't crossplay
	for (const TPair<FString, TSharedPtr<FJsonValue>>& KV : CrossplayObject->Values)
	{
		TSharedPtr<FJsonObject> PrefObject = KV.Value->AsObject();
		if (!PrefObject.IsValid())
		{
			// We cannot validate if this party is crossplay if we cannot grab preferences for the user, so return false
			return false;
		}

		bool bWantsCrossplay = false;
		if (!PrefObject->TryGetBoolField(CROSSPLAY_OBJECT_PLAYER_CROSSPLAY_FIELD, bWantsCrossplay))
		{
			// We cannot validate if this user wanted crossplay, so just return false for no crossplay
			return false;
		}

		// Finally, if this user explicitly doesn't want crossplay, then the whole party cannot play crossplay
		if (!bWantsCrossplay)
		{
			return false;
		}
	}

	// We have gotten through each member and they all want crossplay, so return true
	return true;
}

TArray<FString> FOnlinePartyAccelByte::GetUniquePlatformsForParty()
{
	TArray<FString> OutPlatforms;
	FVariantData OutVariantData;
	if (!PartyData->GetAttribute(CROSSPLAY_OBJECT_NAME, OutVariantData))
	{
		return OutPlatforms;
	}

	TSharedPtr<FJsonObject> CrossplayObject;
	OutVariantData.GetValue(CrossplayObject);
	if (!CrossplayObject.IsValid())
	{
		return OutPlatforms;
	}

	for (const TPair<FString, TSharedPtr<FJsonValue>>& KV : CrossplayObject->Values)
	{
		TSharedPtr<FJsonObject> PrefObject = KV.Value->AsObject();
		if (!PrefObject.IsValid())
		{
			return OutPlatforms;
		}

		FString Platform;
		if (!PrefObject->TryGetStringField(CROSSPLAY_OBJECT_PLAYER_PLATFORM_FIELD, Platform))
		{
			return OutPlatforms;
		}

		OutPlatforms.AddUnique(Platform);
	}

	return OutPlatforms;
}

FOnlinePartyMemberAccelByte::FOnlinePartyMemberAccelByte(const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FString& InDisplayName)
	: UserId(InUserId)
	, DisplayName(InDisplayName)
{
}

TSharedRef<const FUniqueNetId> FOnlinePartyMemberAccelByte::GetUserId() const
{
	return UserId;
}

FString FOnlinePartyMemberAccelByte::GetRealName() const
{
	return DisplayName;
}

FString FOnlinePartyMemberAccelByte::GetDisplayName(const FString& Platform) const
{
	return DisplayName;
}

bool FOnlinePartyMemberAccelByte::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr != nullptr)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}
	return false;
}

FOnlinePartyJoinInfoAccelByte::FOnlinePartyJoinInfoAccelByte(const IOnlinePartyJoinInfo& BaseInfo)
	: PartyId(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(BaseInfo.GetPartyId()))
	, SourceUserId(FUniqueNetIdAccelByteUser::CastChecked(BaseInfo.GetSourceUserId()))
	, SourceUserDisplayName(BaseInfo.GetSourceDisplayName())
	, SourcePlatform(BaseInfo.GetSourcePlatform())
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	, PlatformData(BaseInfo.GetPlatformData())
#endif
	, AppId(BaseInfo.GetAppId())
	, BuildId(BaseInfo.GetBuildId())
{
}

FOnlinePartyJoinInfoAccelByte::FOnlinePartyJoinInfoAccelByte(const TSharedRef<const FOnlinePartyIdAccelByte>& InPartyId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InSourceUserId, const FString& InSourceUserDisplayName)
	: PartyId(InPartyId)
	, SourceUserId(InSourceUserId)
	, SourceUserDisplayName(InSourceUserDisplayName)
{
}

bool FOnlinePartyJoinInfoAccelByte::IsValid() const
{
	// This is a valid join info structure if we have a party ID, and a user ID for the user that invited us
	return PartyId->IsValid() && SourceUserId->IsValid();
}

TSharedRef<const FOnlinePartyId> FOnlinePartyJoinInfoAccelByte::GetPartyId() const
{
	return PartyId;
}

FOnlinePartyTypeId FOnlinePartyJoinInfoAccelByte::GetPartyTypeId() const
{
	// FOnlinePartyTypeId is not used by our platform as we have no distinction between primary and secondary parties, so
	// just return a default party type ID here.
	return FOnlinePartyTypeId();
}

TSharedRef<const FUniqueNetId> FOnlinePartyJoinInfoAccelByte::GetSourceUserId() const
{
	return SourceUserId;
}

const FString& FOnlinePartyJoinInfoAccelByte::GetSourceDisplayName() const
{
	return SourceUserDisplayName;
}

const FString& FOnlinePartyJoinInfoAccelByte::GetSourcePlatform() const
{
	// We don't have a way currently to get the platform that a party invite originated from
	return SourcePlatform;
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
const FString& FOnlinePartyJoinInfoAccelByte::GetPlatformData() const
{
	// We currently don't have a way to attach platform specific data to a party invite
	return PlatformData;
}
#endif

bool FOnlinePartyJoinInfoAccelByte::HasKey() const
{
	// Since we have an invite token associated with this join info, we have a key for the invite
	return true;
}

bool FOnlinePartyJoinInfoAccelByte::HasPassword() const
{
	// However, we do not support passwords for parties, just invites and their tokens
	return false;
}

bool FOnlinePartyJoinInfoAccelByte::IsAcceptingMembers() const
{
	// If we have a join info, it means that we have a party currently that has a pending invite for us, meaning that
	// it is accepting new members
	return true;
}

bool FOnlinePartyJoinInfoAccelByte::IsPartyOfOne() const
{
	// There is no way from a party invite to query how many members the party has
	return false;
}

int32 FOnlinePartyJoinInfoAccelByte::GetNotAcceptingReason() const
{
	// If we are at this point, we don't have any reason why we wouldn't accept members, as this is our invite to the party
	return 0;
}

const FString& FOnlinePartyJoinInfoAccelByte::GetAppId() const
{
	// No way to get current App ID (namespace) for the invite
	return AppId;
}

const FString& FOnlinePartyJoinInfoAccelByte::GetBuildId() const
{
	// Invites do not send along build IDs with them
	return BuildId;
}

bool FOnlinePartyJoinInfoAccelByte::CanJoin() const
{
	// Since this represents a party invite, we can join this party
	return true;
}

bool FOnlinePartyJoinInfoAccelByte::CanJoinWithPassword() const
{
	// However since there is no support for passwords, we cannot join with a password
	return false;
}

bool FOnlinePartyJoinInfoAccelByte::CanRequestAnInvite() const
{
	// This already is a party invite, so we cannot accept an invite from a party invite
	return false;
}

#define ONLINE_ERROR_NAMESPACE "FOnlinePartySystemAccelByte"

FOnlinePartySystemAccelByte::FOnlinePartySystemAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
{
	Init();
}

bool FOnlinePartySystemAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlinePartySystemAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlinePartySystemAccelByte::GetFromWorld(const UWorld* World, FOnlinePartySystemAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

void FOnlinePartySystemAccelByte::Init()
{
	// Grab whether or not V2 sessions are enabled, will enable warnings for this interface if so
	GConfig->GetBool(TEXT("OnlineSubsystemAccelByte"), AB_USE_V2_SESSIONS_CONFIG_KEY, bUsingV2Sessions, GEngineIni);
}

TSharedPtr<FOnlinePartyAccelByte> FOnlinePartySystemAccelByte::GetFirstPartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId)
{
	FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(UserId);
	if (FoundPartyMap != nullptr)
	{
		for (const TPair<TSharedRef<const FOnlinePartyIdAccelByte>, TSharedRef<FOnlinePartyAccelByte>>& KV : *FoundPartyMap)
		{
			return KV.Value;
		}
	}
	return nullptr;
}

TSharedPtr<const FOnlinePartyId> FOnlinePartySystemAccelByte::GetFirstPartyIdForUser(const FUniqueNetId& UserId) {
	TSharedRef<const FUniqueNetIdAccelByteUser> AccelbyteId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(AccelbyteId);
	if (Party.IsValid()) {
		return Party->PartyId;
	}

	return nullptr;
}

void FOnlinePartySystemAccelByte::OnReceivedPartyInviteNotification(const FAccelByteModelsPartyGetInvitedNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; Inviter: %s"), *UserId->ToDebugString(), *Notification.PartyId, *Notification.From);

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo>(AccelByteSubsystem, UserId, Notification);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Invite to party '%s' recieved from user '%s'!"), *Notification.PartyId, *Notification.From)

	AB_OSS_INTERFACE_TRACE_END(TEXT("Fired off async task to get recieved invite information."));
}

void FOnlinePartySystemAccelByte::OnPartyInviteSentNotification(const FAccelByteModelsInvitationNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; InviterId: %s; InviteeId: %s"), *UserId->ToDebugString(), *Notification.InviterID, *Notification.InviteeID);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Invite to party sent by user '%s' to user '%s'!"), *Notification.InviterID, *Notification.InviteeID);

	// This may feel a little odd, but since this notification doesn't send back a party ID that identifies which party
	// generated this notification, I'm just going to grab the first party the player is in and use that to add the
	// invited player. This would work currently as we only support being in one party at a time. However, this will
	// break down if we ever support multiple parties for one user.
	TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(UserId);
	if (Party.IsValid())
	{
		FAccelByteUniqueIdComposite InviterCompositeId;
		InviterCompositeId.Id = Notification.InviterID;

		FAccelByteUniqueIdComposite InviteeCompositeId;
		InviteeCompositeId.Id = Notification.InviteeID;

		Party->AddUserToInvitedPlayers(UserId, FUniqueNetIdAccelByteUser::Create(InviterCompositeId)
			, FUniqueNetIdAccelByteUser::Create(InviteeCompositeId));
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Added invited user to invited players."));
}

void FOnlinePartySystemAccelByte::OnPartyJoinNotification(const FAccelByteModelsPartyJoinNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; JoinedUser: %s"), *UserId->ToDebugString(), *Notification.UserId);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("User '%s' has joined the party!"), *Notification.UserId);

	// Just like the previous notification, we don't get a party ID back from it. So we need to get the first party the map
	// and then add the joined user to it through an async task. This is super hacky, but this also future proofs
	// us if we ever need to support multiple parties for one user.
	TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(UserId);
	if (Party.IsValid())
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember>(AccelByteSubsystem, UserId, Party.ToSharedRef(), Notification.UserId);
	}
	else
	{
		// Local user is still joining party, will add party member after party join complete
		FString JoinedUserId = Notification.UserId;
		UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Party is not ready, party member %s will join after local user join party complete"), *JoinedUserId);
		RunOnPartyJoinedComplete(FOnPartyJoinedDelegate::CreateLambda([this, JoinedUserId, UserId](const FUniqueNetId& /*LocalUserId*/, const FOnlinePartyId& /*PartyId*/)
		{
			UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Party member %s joining party"), *JoinedUserId);
			TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(UserId);
			if (Party.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember>(AccelByteSubsystem, UserId, Party.ToSharedRef(), JoinedUserId);
			}
			else
			{
				UE_LOG(LogAccelByteOSSParty, Warning, TEXT("Party member %s unable to join party: party not found"), *JoinedUserId);
			}
		}));
	}

	// Remove party invitation from the joined party member.
	FPartyInviteArray& InvitesArray = UserIdToPartyInvitesMap.FindOrAdd(UserId);
	InvitesArray.RemoveAll([Notification](const TSharedRef<const FAccelBytePartyInvite>& ExistingInvite)
	{
		return Notification.UserId == ExistingInvite->InviterId->GetAccelByteId();
	});
	TriggerOnPartyInvitesChangedDelegates(UserId.Get());

	AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatched async task to add new joined party member to local party."));
}

void FOnlinePartySystemAccelByte::OnPartyMemberLeaveNotification(const FAccelByteModelsLeavePartyNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; LeavingUser: %s"), *UserId->ToDebugString(), *Notification.UserID);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("User '%s' has left the party!"), *Notification.UserID);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());

	if (IdentityInterface.IsValid())
	{
		TSharedPtr<const FUniqueNetId> LocalPlayerUserId = IdentityInterface->GetUniquePlayerId(AccelByteSubsystem->GetLocalUserNumCached());
		TSharedRef<const FUniqueNetIdAccelByteUser> LocalAccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalPlayerUserId.ToSharedRef());

		TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(LocalAccelByteUserId);
		if (Party.IsValid())
		{
			FAccelByteUniqueIdComposite LeftUserCompositeId;
			LeftUserCompositeId.Id = Notification.UserID;

			TSharedRef<const FUniqueNetIdAccelByteUser> LeftUserId = FUniqueNetIdAccelByteUser::Create(LeftUserCompositeId);
			Party->RemoveMember(UserId, LeftUserId, EMemberExitedReason::Left);
			RemovePartyFromInterface(LeftUserId, Party.ToSharedRef());
		}
		else
		{
			// If we're unable to retrieve the party for the local user, the local user may be in the progress of joining one, 
			// so we'll need to wait until we're in a party before removing the target user from the party
			FOnPartyJoinedDelegate PartyJoinedDelegate = FOnPartyJoinedDelegate::CreateRaw(this, &FOnlinePartySystemAccelByte::OnPartyJoinedCompleteMemberLeaveParty, Notification, UserId, LocalAccelByteUserId);
			RunOnPartyJoinedComplete(PartyJoinedDelegate);
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Removed user from party as they left."));
}

void FOnlinePartySystemAccelByte::OnPartyKickNotification(const FAccelByteModelsGotKickedFromPartyNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; KickedUser: %s"), *UserId->ToDebugString(), *Notification.PartyId, *Notification.UserId);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("User '%s' has been kicked from party '%s'!"), *Notification.UserId, *Notification.PartyId);

	// We are able to get these party kick notifications for ourselves, which is how we get notified that we have been
	// kicked from the party handle this case specially to leave the party.
	if (UserId->GetAccelByteId() == Notification.UserId)
	{
		if (RemovePartyForUser(UserId, MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId))) 
		{
			// We will also want to fire this delegate so party for local user is updated
			TriggerOnPartyExitedDelegates(UserId.Get(), MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId).Get());
		}

		AB_OSS_INTERFACE_TRACE_END(TEXT("Removing local user from party as they have been kicked."));
	}
	else
	{
		TSharedPtr<FOnlinePartyAccelByte> Party = GetPartyForUser(UserId, MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId));
		if (Party.IsValid())
		{
			FAccelByteUniqueIdComposite KickedUserCompositeId;
			KickedUserCompositeId.Id = Notification.UserId;
			Party->RemoveMember(UserId, FUniqueNetIdAccelByteUser::Create(KickedUserCompositeId), EMemberExitedReason::Kicked);
		}

		AB_OSS_INTERFACE_TRACE_END(TEXT("Removing remote user from party as they have been kicked."));
	}
}

void FOnlinePartySystemAccelByte::OnPartyMemberConnectNotification(const FAccelByteModelsPartyMemberConnectionNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	TSharedPtr<FOnlinePartyAccelByte> Party = GetPartyForUser(UserId, MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId));
	if (Party.IsValid())
	{
		for(TPair<TSharedRef<const FUniqueNetIdAccelByteUser>, TSharedRef<FOnlinePartyMemberAccelByte>>& UserIdToPartyPair : Party->UserIdToPartyMemberMap)
		{
			if(UserIdToPartyPair.Key->GetAccelByteId().Equals(Notification.UserId))
			{
				if(UserIdToPartyPair.Value->MemberConnectionStatus != EMemberConnectionStatus::Connected)
				{
					UserIdToPartyPair.Value->SetMemberConnectionStatus(EMemberConnectionStatus::Connected);
				}
				break;
			}
		}
	}
}

void FOnlinePartySystemAccelByte::OnPartyMemberDisconnectNotification(const FAccelByteModelsPartyMemberConnectionNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	TSharedPtr<FOnlinePartyAccelByte> Party = GetPartyForUser(UserId, MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId));
	if (Party.IsValid())
	{
		for(TPair<TSharedRef<const FUniqueNetIdAccelByteUser>, TSharedRef<FOnlinePartyMemberAccelByte>>& UserIdToPartyPair : Party->UserIdToPartyMemberMap)
		{
			if(UserIdToPartyPair.Key->GetAccelByteId().Equals(Notification.UserId))
			{
				if(UserIdToPartyPair.Value->MemberConnectionStatus != EMemberConnectionStatus::Disconnected)
				{
					UserIdToPartyPair.Value->SetMemberConnectionStatus(EMemberConnectionStatus::Disconnected);
				}
				break;
			}
		}
	}
}

void FOnlinePartySystemAccelByte::OnPartyDataChangeNotification(const FAccelByteModelsPartyDataNotif& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s"), *UserId->ToDebugString(), *Notification.PartyId);

	FString NotificationString;
	FJsonObjectConverter::UStructToJsonObjectString(Notification, NotificationString);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Updated party information recieved! Data: %s"), *NotificationString);

	// First, check if the party leader ID has changed and if so, set the current leader ID to be the new one from the notification
	TSharedPtr<FOnlinePartyAccelByte> Party = GetPartyForUser(UserId, MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId));
	if (!Party.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to update party data as we could not find a party with the ID specified. Probably need to call RestoreParties first."));
		return;
	} 
	
	TSharedPtr<const FUniqueNetIdAccelByteUser> PreviousLeaderId = FUniqueNetIdAccelByteUser::CastChecked(Party->LeaderId.ToSharedRef());
	if (!Notification.Leader.IsEmpty() && Notification.Leader != PreviousLeaderId->GetAccelByteId())
	{
		UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("User '%s' has been promoted to leader of party '%s' replacing user '%s'!"), *Notification.Leader, *Party->PartyId->ToString(), *PreviousLeaderId->GetAccelByteId());

		// Get the leader ID without the platform information first, use it to get the actual ID for the leader and set that
		FAccelByteUniqueIdComposite LeaderCompositeId;
		LeaderCompositeId.Id = Notification.Leader;

		TSharedPtr<const FUniqueNetIdAccelByteUser> ABUserId = FUniqueNetIdAccelByteUser::Create(LeaderCompositeId);
		TSharedPtr<const FOnlinePartyMemberAccelByte> LeaderMember = Party->GetMember(ABUserId.ToSharedRef());
		if (LeaderMember.IsValid())
		{
			Party->LeaderId = LeaderMember->GetUserId();

			const TSharedRef<const FUniqueNetIdAccelByteUser> LeaderMemberId = FUniqueNetIdAccelByteUser::CastChecked(LeaderMember->GetUserId());
			FString LeaderMemberIdStr = LeaderMemberId->GetAccelByteId();

			TSharedPtr<const FUniqueNetIdAccelByteUser> PartyLeaderId = FUniqueNetIdAccelByteUser::CastChecked(Party->LeaderId.ToSharedRef());
			FString PartyLeaderIdStr = PartyLeaderId->GetAccelByteId();
			//Send the notif for all local users that have the same party object
			for (auto Member : Party->GetAllMembers())
			{
				if (Member->GetUserId().Get() != *PreviousLeaderId.Get())
				{
					FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(FUniqueNetIdAccelByteUser::CastChecked(Member->GetUserId()));
					if (FoundPartyMap != nullptr)
					{
						TSharedRef<FOnlinePartyAccelByte>* FoundPartyRef = FoundPartyMap->Find(MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId));
						if(FoundPartyRef != nullptr && *FoundPartyRef == Party)
						{
							TriggerOnPartyMemberPromotedDelegates(Member->GetUserId().Get(), Party->PartyId.Get(), *Party->LeaderId.Get());
						}
					}
				}

				// If the Leader Change, then the leader should request for PartyCode
				const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
				if (IdentityInterface.IsValid())
				{
					if (Member->GetUserId() == LeaderMember->GetUserId() && IdentityInterface->GetApiClient(LeaderMemberId.Get()).IsValid())
					{
						FOnPartyCodeGenerated Delegate = FOnPartyCodeGenerated::CreateRaw(this, &FOnlinePartySystemAccelByte::OnPromotedLeaderGetPartyCode);
						AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetV1PartyCode>(AccelByteSubsystem, LeaderMemberId, Notification.PartyId, Delegate);
					}
				}
			}
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("LeaderMember is not valid!"));
		}
	}

	// Now from here, this also could be a notification for when party storage changes, so we want to try and update that
	// First, check if we have a valid JSON object in the first place
	if (!Notification.Custom_attribute.JsonObject.IsValid())
	{
		return;
	}

	// Now, we can use the JSON wrapper util to convert our response to a string
	FString JSONString;
	if (!Notification.Custom_attribute.JsonObjectToString(JSONString))
	{
		return;
	}

	// If empty then don't update
	if (JSONString.Len() == 2)
	{
		UE_LOG_AB(Log, TEXT("FOnlinePartySystemAccelByte::OnPartyDataChangeNotification there is no party storage update"));
		return;
	}

	// Add attrs field for the use of FOnlinePartyData::FromJson()
	TSharedRef<FJsonObject> MainJsonObj = MakeShared<FJsonObject>();
	TSharedRef<FJsonValueObject> JsonValue = MakeShared<FJsonValueObject>(Notification.Custom_attribute.JsonObject);
	MainJsonObj->SetField("Attrs", JsonValue);

	FString PartyDataStr;
	FJsonObjectWrapper PartyStorageData;
	PartyStorageData.JsonObject = MainJsonObj;
	PartyStorageData.JsonObjectToString(JSONString);

	// Finally, we can provide this JSON string to the FromJson method of our PartyData instance which will populate our values
	TSharedRef<FOnlinePartyData> PartyData = MakeShared<FOnlinePartyData>();
	PartyData->FromJson(JSONString);
	Party->SetPartyData(PartyData);

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	TriggerOnPartyDataReceivedDelegates(UserId.Get(), *Party->PartyId, NAME_Game, *PartyData);
#else
	TriggerOnPartyDataReceivedDelegates(UserId.Get(), *Party->PartyId, *PartyData);
#endif

	AB_OSS_INTERFACE_TRACE_END(TEXT("Finished updating data on party."));
}

void FOnlinePartySystemAccelByte::RunOnPartyJoinedComplete(const FOnPartyJoinedDelegate& Delegate)
{
	OnPartyJoinedPendingTasks.Add(Delegate);
}

void FOnlinePartySystemAccelByte::OnPartyJoinedComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
{
	for (const auto& PartyJoinedDelegate : OnPartyJoinedPendingTasks)
	{
		PartyJoinedDelegate.ExecuteIfBound(LocalUserId, PartyId);
	}
	OnPartyJoinedPendingTasks.Empty();
}

void FOnlinePartySystemAccelByte::OnPartyJoinedCompleteMemberLeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FAccelByteModelsLeavePartyNotice Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId, TSharedRef<const FUniqueNetIdAccelByteUser> LocalAccelByteUserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; LeavingUser: %s"), *UserId->ToDebugString(), *Notification.UserID);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("User '%s' has left the party!"), *Notification.UserID);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());

	if (IdentityInterface.IsValid())
	{
		TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(LocalAccelByteUserId);
		if (Party.IsValid())
		{
			FAccelByteUniqueIdComposite LeftUserCompositeId;
			LeftUserCompositeId.Id = Notification.UserID;

			TSharedRef<const FUniqueNetIdAccelByteUser> LeftUserId = FUniqueNetIdAccelByteUser::Create(LeftUserCompositeId);
			Party->RemoveMember(UserId, LeftUserId, EMemberExitedReason::Left);
			RemovePartyFromInterface(LeftUserId, Party.ToSharedRef());
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Removed user from party as they left."));
}

void FOnlinePartySystemAccelByte::OnPromotedLeaderGetPartyCode(bool bWasSuccessful, const FString& PartyCode, const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& PartyId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; PartyCode: %s"), *UserId->ToDebugString(), *PartyId, *PartyCode);
	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("User '%s' request for PartyCode was responded!"), *UserId->ToDebugString());

	if (bWasSuccessful == false)
	{
		return;
	}
	auto FoundUser = UserIdToPartiesMap.Find(UserId);
	if (FoundUser == nullptr)
	{
		return;
	}
	auto FoundParty = FoundUser->Find(MakeShared<const FOnlinePartyIdAccelByte>(PartyId));
	if (FoundParty == nullptr)
	{
		return;
	}
	FoundParty->Get().SetPartyCode(PartyCode);

	AB_OSS_INTERFACE_TRACE_END(TEXT("Successfully obtaining PartyCode."));
}

void FOnlinePartySystemAccelByte::RegisterRealTimeLobbyDelegates(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId)
{
	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		return;
	}

	// Get our identity interface to retrieve the API client for this user
	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(UserId.Get());
	if (!ApiClient.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to register real-time lobby as an API client could not be retrieved for user '%s'"), *(UserId->ToDebugString()))
		return;
	}

	AccelByte::Api::Lobby::FPartyGetInvitedNotif OnReceivedPartyInviteNotifDelegate = AccelByte::Api::Lobby::FPartyGetInvitedNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnReceivedPartyInviteNotification, UserId);
	ApiClient->Lobby.SetPartyGetInvitedNotifDelegate(OnReceivedPartyInviteNotifDelegate);

	AccelByte::Api::Lobby::FPartyInviteNotif OnPartyInviteSentNotifDelegate = AccelByte::Api::Lobby::FPartyInviteNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnPartyInviteSentNotification, UserId);
	ApiClient->Lobby.SetPartyInviteNotifDelegate(OnPartyInviteSentNotifDelegate);

	AccelByte::Api::Lobby::FPartyJoinNotif OnPartyJoinNotificationDelegate = AccelByte::Api::Lobby::FPartyJoinNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnPartyJoinNotification, UserId);
	ApiClient->Lobby.SetPartyJoinNotifDelegate(OnPartyJoinNotificationDelegate);

	AccelByte::Api::Lobby::FPartyMemberLeaveNotif OnPartyMemberLeaveNotificationDelegate = AccelByte::Api::Lobby::FPartyMemberLeaveNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnPartyMemberLeaveNotification, UserId);
	ApiClient->Lobby.SetPartyMemberLeaveNotifDelegate(OnPartyMemberLeaveNotificationDelegate);

	AccelByte::Api::Lobby::FPartyKickNotif OnPartyKickNotificationDelegate = AccelByte::Api::Lobby::FPartyKickNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnPartyKickNotification, UserId);
	ApiClient->Lobby.SetPartyKickNotifDelegate(OnPartyKickNotificationDelegate);

	AccelByte::Api::Lobby::FPartyDataUpdateNotif OnPartyDataChangeNotificationDelegate = AccelByte::Api::Lobby::FPartyDataUpdateNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnPartyDataChangeNotification, UserId);
	ApiClient->Lobby.SetPartyDataUpdateResponseDelegate(OnPartyDataChangeNotificationDelegate);

	AccelByte::Api::Lobby::FPartyMemberConnectNotif OnPartyMemberConnectNotificationDelegate = AccelByte::Api::Lobby::FPartyMemberConnectNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnPartyMemberConnectNotification, UserId);
	ApiClient->Lobby.SetPartyMemberConnectNotifDelegate(OnPartyMemberConnectNotificationDelegate);
	
	AccelByte::Api::Lobby::FPartyMemberConnectNotif OnPartyMemberDisconnectNotificationDelegate = AccelByte::Api::Lobby::FPartyMemberDisconnectNotif::CreateThreadSafeSP(AsShared(), &FOnlinePartySystemAccelByte::OnPartyMemberDisconnectNotification, UserId);
	ApiClient->Lobby.SetPartyMemberDisconnectNotifDelegate(OnPartyMemberDisconnectNotificationDelegate);
	
	FOnPartyJoinedDelegate PartyJoinedDelegate = FOnPartyJoinedDelegate::CreateThreadSafeSP(this, &FOnlinePartySystemAccelByte::OnPartyJoinedComplete);
	AddOnPartyJoinedDelegate_Handle(PartyJoinedDelegate);
}

void FOnlinePartySystemAccelByte::AddPartyToInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<FOnlinePartyAccelByte>& Party)
{
	FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(UserId);
	if (FoundPartyMap != nullptr)
	{
		FoundPartyMap->Add(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(Party->PartyId), Party);
	}
	else
	{
		FPartyIDToPartyMap NewPartyMap;
		NewPartyMap.Add(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(Party->PartyId), Party);
		UserIdToPartiesMap.Add(UserId, NewPartyMap);
	}
}

bool FOnlinePartySystemAccelByte::RemovePartyFromInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId)
{
	TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(UserId);
	if (Party.IsValid())
	{
		return RemovePartyFromInterface(UserId, Party.ToSharedRef());
	}
	return false;
}

bool FOnlinePartySystemAccelByte::RemovePartyFromInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<FOnlinePartyAccelByte>& Party)
{
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId = StaticCastSharedRef<const FOnlinePartyIdAccelByte>(Party->PartyId);
	return RemovePartyFromInterface(UserId, PartyId);
}

bool FOnlinePartySystemAccelByte::RemovePartyFromInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId)
{
	FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(UserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(PartyId);
		if (FoundParty != nullptr)
		{
			FoundPartyMap->Remove(PartyId);
			return true;
		}
	}
	return false;
}

bool FOnlinePartySystemAccelByte::IsPlayerInParty(const FUniqueNetId& UserId, const FOnlinePartyId& PartyId)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	if (!AccelByteId->IsValid())
	{
		return false;
	}

	FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(AccelByteId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<const FOnlinePartyIdAccelByte> AccelBytePartyId = StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared());
		if (!AccelBytePartyId->IsValid())
		{
			return false;
		}

		for (auto FoundPartyPair : *FoundPartyMap)
		{
			if(FoundPartyPair.Key->ToString() == AccelBytePartyId->ToString())
			{
				return true;
			}
		}
	}
	return false;
}

bool FOnlinePartySystemAccelByte::IsPlayerInAnyParty(const FUniqueNetId& UserId)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	if (!AccelByteId->IsValid())
	{
		return false;
	}

	FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(AccelByteId);
	if (FoundPartyMap != nullptr)
	{
		return FoundPartyMap->Num() > 0;
	}
	return false;
}

int32 FOnlinePartySystemAccelByte::GetCurrentPartyMemberCount(const FUniqueNetId& UserId)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	if (!AccelByteId->IsValid())
	{
		return -1;
	}

	TSharedPtr<FOnlinePartyAccelByte> Party = GetFirstPartyForUser(AccelByteId);
	if (!Party.IsValid())
	{
		return -1;
	}

	return Party->GetMemberCount();
}

TSharedPtr<const FAccelBytePartyInvite> FOnlinePartySystemAccelByte::GetInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId)
{
	FPartyInviteArray* FoundInvitesArray = UserIdToPartyInvitesMap.Find(UserId);
	if (FoundInvitesArray != nullptr)
	{
		TSharedRef<const FAccelBytePartyInvite>* FoundInvite = FoundInvitesArray->FindByPredicate([&PartyId](const TSharedRef<const FAccelBytePartyInvite>& Invite) {
			return Invite->PartyId.Get() == PartyId.Get();
		});

		if (FoundInvite != nullptr)
		{
			return *FoundInvite;
		}
	}
	return nullptr;
}

TSharedPtr<const FAccelBytePartyInvite> FOnlinePartySystemAccelByte::GetInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InviterId)
{
	FPartyInviteArray* FoundInvitesArray = UserIdToPartyInvitesMap.Find(UserId);
	if (FoundInvitesArray != nullptr)
	{
		TSharedRef<const FAccelBytePartyInvite>* FoundInvite = FoundInvitesArray->FindByPredicate([&InviterId](const TSharedRef<const FAccelBytePartyInvite>& Invite) {
			return Invite->InviterId.Get() == InviterId.Get();
		});

		if (FoundInvite != nullptr)
		{
			return *FoundInvite;
		}
	}
	return nullptr;
}

void FOnlinePartySystemAccelByte::AddPartyInvite(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<FAccelBytePartyInvite>& Invite)
{
	FPartyInviteArray& InvitesArray = UserIdToPartyInvitesMap.FindOrAdd(UserId);
	// Remove existing invite with same party Id, so it won't duplicates
	InvitesArray.RemoveAll([Invite](const TSharedRef<const FAccelBytePartyInvite>& ExistingInvite)
	{
		return Invite->PartyId->ToString() == ExistingInvite->PartyId->ToString()
			|| Invite->InviterId->ToString() == ExistingInvite->InviterId->ToString();
	});
	InvitesArray.Add(Invite);
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	TriggerOnPartyInviteReceivedDelegates(UserId.Get(), Invite->PartyId.Get(), Invite->InviterId.Get());
#endif
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27)
	TSharedRef<FOnlinePartyJoinInfoAccelByte> JoinInfoRef = MakeShared<FOnlinePartyJoinInfoAccelByte>(Invite->PartyId, Invite->InviterId, Invite->InviterDisplayName);
	TriggerOnPartyInviteReceivedExDelegates(UserId.Get(), JoinInfoRef.Get());
#endif
}

bool FOnlinePartySystemAccelByte::RemoveInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId, const EPartyInvitationRemovedReason& InvitationRemovalReason)
{
	FPartyInviteArray* FoundInvitesArray = UserIdToPartyInvitesMap.Find(UserId);
	if (FoundInvitesArray != nullptr)
	{
		int32 FoundInviteIndex = FoundInvitesArray->IndexOfByPredicate([&PartyId](const TSharedRef<const FAccelBytePartyInvite>& Invite) {
			return Invite->PartyId.Get() == PartyId.Get();
		});

		if (FoundInviteIndex != INDEX_NONE)
		{
			const TSharedRef<const FUniqueNetIdAccelByteUser> SenderId = (*FoundInvitesArray)[FoundInviteIndex]->InviterId;
			const FString SenderDisplayName = (*FoundInvitesArray)[FoundInviteIndex]->InviterDisplayName;
			
			FoundInvitesArray->RemoveAt(FoundInviteIndex);
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
			TriggerOnPartyInviteRemovedDelegates(UserId.Get(), PartyId.Get(), SenderId.Get(), InvitationRemovalReason);
#endif

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27)
			TSharedRef<FOnlinePartyJoinInfoAccelByte> JoinInfoRef = MakeShared<FOnlinePartyJoinInfoAccelByte>(PartyId, SenderId, SenderDisplayName);
			TriggerOnPartyInviteRemovedExDelegates(UserId.Get(), JoinInfoRef.Get(),InvitationRemovalReason);
#endif
			TriggerOnPartyInvitesChangedDelegates(UserId.Get());
			return true;
		}
	}
	return false;
}

bool FOnlinePartySystemAccelByte::RemoveInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InviterId, const EPartyInvitationRemovedReason& InvitationRemovalReason)
{
	FPartyInviteArray* FoundInvitesArray = UserIdToPartyInvitesMap.Find(UserId);
	if (FoundInvitesArray != nullptr)
	{
		int32 FoundInviteIndex = FoundInvitesArray->IndexOfByPredicate([&InviterId](const TSharedRef<const FAccelBytePartyInvite>& Invite) {
			return Invite->InviterId.Get() == InviterId.Get();
		});

		if (FoundInviteIndex != INDEX_NONE)
		{
			TSharedRef<const FAccelBytePartyInvite> Invite = (*FoundInvitesArray)[FoundInviteIndex];
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
			TriggerOnPartyInviteRemovedDelegates(UserId.Get(), Invite->PartyId.Get(), Invite->InviterId.Get(), InvitationRemovalReason);
#endif
			FoundInvitesArray->RemoveAt(FoundInviteIndex);
			return true;
		}
	}
	return false;
}

bool FOnlinePartySystemAccelByte::ClearInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedPtr<const FOnlinePartyId>& PartyId, const EPartyInvitationRemovedReason& InvitationRemovalReason)
{
	// ClearInvitiations will just clear the local cache for now, as we cannot restore invites between sessions
	FPartyInviteArray* FoundInvites = UserIdToPartyInvitesMap.Find(UserId);
	if (FoundInvites != nullptr)
	{
		// If we have a party ID associated with this request, then it means that we want to remove a specific party invite
		// With this in mind, we will iterate through all invites, check if the party ID matches, and if so, remove it.
		if (PartyId.IsValid())
		{
			FoundInvites->RemoveAll([&PartyId](const TSharedRef<const FAccelBytePartyInvite>& Invite) { return Invite->PartyId == PartyId; });
		}
		// Otherwise, just remove all invites we have cached
		else
		{
			FoundInvites->Empty();
		}
		return true;
	}
	return false;
}

bool FOnlinePartySystemAccelByte::RemovePartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId)
{
	TSharedPtr<const FOnlinePartyIdAccelByte> PartyId = StaticCastSharedPtr<const FOnlinePartyIdAccelByte>(GetFirstPartyIdForUser(UserId.Get()));
	if (PartyId.IsValid())
	{
		return RemovePartyForUser(UserId, PartyId.ToSharedRef());
	}
	return false;
}

bool FOnlinePartySystemAccelByte::RemovePartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId)
{
	bool bSuccess = RemovePartyFromInterface(UserId, PartyId);
	return bSuccess;
}

TSharedPtr<FOnlinePartyAccelByte> FOnlinePartySystemAccelByte::GetPartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId)
{
	FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(UserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(PartyId);
		if (FoundParty != nullptr)
		{
			return *FoundParty;
		}
	}
	return nullptr;
}

void FOnlinePartySystemAccelByte::RestoreParties(const FUniqueNetId& LocalUserId, const FOnRestorePartiesComplete& CompletionDelegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	// Note that this functionality really doesn't restore any party information on the backend, as that is not supported.
	// Rather, if you have not toggled "Auto Kick on Disconnect" on in the Lobby configuration in the admin portal, you will
	// still be in a party by this point, meaning that we can just send off a task to get party information when this is
	// called in an attempt to "restore" our party.
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRestoreV1Parties>(AccelByteSubsystem, LocalUserId, CompletionDelegate);
}

void FOnlinePartySystemAccelByte::RestoreInvites(const FUniqueNetId& LocalUserId, const FOnRestoreInvitesComplete& CompletionDelegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::RestoreInvites is not currently supported."));
	AccelByteSubsystem->ExecuteNextTick([UserId = LocalUserId.AsShared(), CompletionDelegate]() {
		CompletionDelegate.ExecuteIfBound(UserId.Get(), ONLINE_ERROR(EOnlineErrorResult::NotImplemented));
	});
}

void FOnlinePartySystemAccelByte::CleanupParties(const FUniqueNetId& LocalUserId, const FOnCleanupPartiesComplete& CompletionDelegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	// I don't think we really need to implement a separate method for cleaning up party data, as we really only support
	// one party at a time. With this in mind, if you just want to clean a party, you should be able to just leave that
	// party, which will also clear all cached state.
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::CleanupParties is not currently supported."));
	AccelByteSubsystem->ExecuteNextTick([UserId = LocalUserId.AsShared(), CompletionDelegate]() {
		CompletionDelegate.ExecuteIfBound(UserId.Get(), ONLINE_ERROR(EOnlineErrorResult::NotImplemented));
	});
}

bool FOnlinePartySystemAccelByte::CreateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyConfiguration& PartyConfig, const FOnCreatePartyComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	if (bIsAcceptingCustomGameInvitation)
	{
		FOnCreatePartyComplete OnCreatePartyComplete = FOnCreatePartyComplete::CreateLambda([this, Delegate](const FUniqueNetId& LocalUserId, const TSharedPtr<const FOnlinePartyId>& PartyId, const ECreatePartyCompletionResult Result)
		{
			OnCreatePartyBeforeJoinCustomGameComplete.ExecuteIfBound(LocalUserId, PartyId, Result);
			Delegate.ExecuteIfBound(LocalUserId, PartyId, Result);
			bIsAcceptingCustomGameInvitation = false;
		});
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateV1Party>(AccelByteSubsystem, LocalUserId, PartyTypeId, PartyConfig, OnCreatePartyComplete);
	}
	else
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateV1Party>(AccelByteSubsystem, LocalUserId, PartyTypeId, PartyConfig, Delegate);
	}
	return true;
}

bool FOnlinePartySystemAccelByte::UpdateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyConfiguration& PartyConfig, bool bShouldRegenerateReservationKey /*= false*/, const FOnUpdatePartyComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	// Most if not all config about parties must be updated on the admin portal, at least for now. With that in mind, we do not support updating the config.
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::UpdateParty is not supported!"));

	// Convert IDs to our internal ID types so that we can pass them as lambda captures
	const TSharedRef<const FUniqueNetIdAccelByteUser> NetId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const TSharedRef<const FOnlinePartyIdAccelByte> ABPartyId = StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared());
	AccelByteSubsystem->ExecuteNextTick([NetId, ABPartyId, Delegate]() {
		Delegate.ExecuteIfBound(NetId.Get(), ABPartyId.Get(), EUpdateConfigCompletionResult::UnknownClientFailure);
	});
	return false;
}

bool FOnlinePartySystemAccelByte::JoinParty(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnJoinPartyComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJoinV1Party>(AccelByteSubsystem, LocalUserId, OnlinePartyJoinInfo, Delegate);
	return true;
}

bool FOnlinePartySystemAccelByte::JoinParty(const FUniqueNetId& LocalUserId, const FString& InPartyCode, const FOnJoinPartyComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJoinV1Party>(AccelByteSubsystem, LocalUserId, InPartyCode, Delegate);
	return true;
}

bool FOnlinePartySystemAccelByte::GetPartyCode(FString& Output, const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
{
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	auto PartyInfo = GetPartyData(LocalUserId, PartyId, FName(""));
#else
	auto PartyInfo = GetPartyData(LocalUserId, PartyId);
#endif
	if (PartyInfo.IsValid() == false)
	{
		return false;
	}

	FVariantData Result;
	if (PartyInfo->GetAttribute(PARTYDATA_PARTYCODE_ATTR, Result) == false)
	{
		return false;
	}
	
	FString ResultString = Result.ToString();
	if (ResultString.IsEmpty())
	{
		return false;
	}

	Output = ResultString;
	return true;
}

bool FOnlinePartySystemAccelByte::JIPFromWithinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& PartyLeaderId)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::JIPFromWithinParty is not supported!"));
	return false;
}

bool FOnlinePartySystemAccelByte::RejoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyTypeId& PartyTypeId, const TArray<TSharedRef<const FUniqueNetId>>& FormerMembers, const FOnJoinPartyComplete& Delegate /*= FOnJoinPartyComplete()*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	// Since parties require an invite to join them, you cannot just rejoin a party with the ID unless you have an invite, by which point you'd just join via JoinParty.
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::RejoinParty is not supported!"));

	// Convert IDs to our internal ID types so that we can pass them as lambda captures
	const TSharedRef<const FUniqueNetIdAccelByteUser> NetId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const TSharedRef<const FOnlinePartyIdAccelByte> ABPartyId = StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared());
	AccelByteSubsystem->ExecuteNextTick([NetId, ABPartyId, Delegate]() {
		Delegate.ExecuteIfBound(NetId.Get(), ABPartyId.Get(), EJoinPartyCompletionResult::UnableToRejoin, UNSUPPORTED_METHOD_REASON);
	});

	return false;
}

bool FOnlinePartySystemAccelByte::LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnLeavePartyComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	// Defaulting synchronizing leaving a party on the backend to be true, as this is most likely what a dev wishes to do
	return LeaveParty(LocalUserId, PartyId, true, Delegate);
}

bool FOnlinePartySystemAccelByte::LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, bool bSynchronizeLeave, const FOnLeavePartyComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLeaveV1Party>(AccelByteSubsystem, LocalUserId, PartyId, bSynchronizeLeave, Delegate);
	return true;
}

bool FOnlinePartySystemAccelByte::ApproveJoinRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode /*= 0*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	// Parties do not currently have the concept of join requests, thus meaning that we cannot support approval of them
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::ApproveJoinRequest is not supported!"));
	return false;
}

bool FOnlinePartySystemAccelByte::ApproveJIPRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode /*= 0*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	// Parties don't have a concept of what session they are in by default, and there is no system for join in progress with requests on the backend, so we cannot support this
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::ApproveJIPRequest is not supported!"));
	return false;
}

#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
void FOnlinePartySystemAccelByte::QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityComplete& Delegate /*= FOnQueryPartyJoinabilityComplete()*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::QueryPartyJoinabilty is not supported as the only way to join a party is through an invite!"));
	AccelByteSubsystem->ExecuteNextTick([UserId = LocalUserId.AsShared(), Delegate]() {
		Delegate.ExecuteIfBound(UserId.Get(), MakeShared<FOnlinePartyIdAccelByte>().Get(), EJoinPartyCompletionResult::IncompatiblePlatform, 0);
	});
}

void FOnlinePartySystemAccelByte::RespondToQueryJoinability(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bCanJoin, int32 DeniedResultCode /*= 0*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	// Just like ApproveJoinRequest, parties don't have the ability to be joined without invite or have join requests, so we cannot support this
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::RespondToQueryJoinability is not supported!"));
}
#endif

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27)
void FOnlinePartySystemAccelByte::QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityCompleteEx& Delegate /*= FOnQueryPartyJoinabilityCompleteEx()*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::QueryPartyJoinabilty is not supported as the only way to join a party is through an invite!"));
	AccelByteSubsystem->ExecuteNextTick([UserId = LocalUserId.AsShared(), Delegate]() {
		FQueryPartyJoinabilityResult Result;
		Result.EnumResult = EJoinPartyCompletionResult::IncompatiblePlatform;
		Result.SubCode = 0;
		Delegate.ExecuteIfBound(UserId.Get(), MakeShared<FOnlinePartyIdAccelByte>().Get(), Result);
	});
}

void FOnlinePartySystemAccelByte::RespondToQueryJoinability(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bCanJoin, int32 DeniedResultCode, FOnlinePartyDataConstPtr PartyData)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::RespondToQueryJoinability is not supported!"));
}
#endif

#if ENGINE_MAJOR_VERSION == 5
void FOnlinePartySystemAccelByte::RequestToJoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyInvitationRecipient& Recipient, const FOnRequestToJoinPartyComplete& Delegate /*= FOnRequestToJoinPartyComplete()*/)
{
}

void FOnlinePartySystemAccelByte::ClearRequestToJoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& Sender, EPartyRequestToJoinRemovedReason Reason)
{
}

bool FOnlinePartySystemAccelByte::GetPendingRequestsToJoin(const FUniqueNetId& LocalUserId, TArray<IOnlinePartyRequestToJoinInfoConstRef>& OutRequestsToJoin) const
{
	return false;
}

#if ENGINE_MINOR_VERSION >= 1
void FOnlinePartySystemAccelByte::CancelInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& TargetUserId, const FOnlinePartyId& PartyId, const FOnCancelPartyInvitationComplete& Delegate /*= FOnCancelPartyInvitationComplete()*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	// Just like ApproveJoinRequest, parties don't have the ability to be joined without invite or have join requests, so we cannot support this
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::CancelInvitation is not implemented yet!"));
}
#endif

#if ENGINE_MINOR_VERSION >= 2
IOnlinePartyJoinInfoConstPtr FOnlinePartySystemAccelByte::MakeJoinInfo(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
{
	return nullptr;
}
#endif

#endif

bool FOnlinePartySystemAccelByte::SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnSendPartyInvitationComplete& Delegate /*= FOnSendPartyInvitationComplete()*/)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendV1PartyInvite>(AccelByteSubsystem, LocalUserId, PartyId, Recipient, Delegate);
	return true;
}

bool FOnlinePartySystemAccelByte::RejectInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	TSharedRef<const FUniqueNetIdAccelByteUser> LocalUserIdAccelByte = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	TSharedRef<const FUniqueNetIdAccelByteUser> SenderIdAccelByte = FUniqueNetIdAccelByteUser::CastChecked(SenderId);
	
	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		return false;
	}

	// Get API client for user to reject party invite
	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalUserId);
	if (!ApiClient.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to reject invitation as an API client could not be retrieved for user '%s'"), *(LocalUserId.ToDebugString()))
		return false;
	}

	// Note that this doesn't use an async task as it's trivial and doesn't even have a delegate that we can fire to notify
	// that the request was a success or not, seems to be intended to be a fire and forget type of method
	TSharedPtr<const FAccelBytePartyInvite> Invite = GetInviteForParty(LocalUserIdAccelByte, SenderIdAccelByte);
	if (Invite.IsValid())
	{
		// Send off a request to reject the invitation on the backend as well as remove the invite from our local cache
		ApiClient->Lobby.SendRejectInvitationRequest(Invite->PartyId->ToString(), Invite->InviteToken);
		RemoveInviteForParty(LocalUserIdAccelByte, SenderIdAccelByte, EPartyInvitationRemovedReason::Declined);
	}
	return true;
}

void FOnlinePartySystemAccelByte::ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return;
	}

	// ClearInvitations will just clear the local cache for now, as we cannot restore invites between sessions
	ClearInviteForParty(FUniqueNetIdAccelByteUser::CastChecked(LocalUserId), MakeShareable(PartyId), EPartyInvitationRemovedReason::Cleared);
}

bool FOnlinePartySystemAccelByte::KickMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnKickPartyMemberComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteKickV1PartyMember>(AccelByteSubsystem, LocalUserId, PartyId, TargetMemberId, Delegate);
	return true;
}

bool FOnlinePartySystemAccelByte::PromoteMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnPromotePartyMemberComplete& Delegate)
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelBytePromoteV1PartyLeader>(AccelByteSubsystem, LocalUserId, PartyId, TargetMemberId, Delegate);
	return true;
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
bool FOnlinePartySystemAccelByte::UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyData)
#else
bool FOnlinePartySystemAccelByte::UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyData)
#endif
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 25)
	FName Namespace{};
#endif
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateV1PartyData>(AccelByteSubsystem, LocalUserId, PartyId, Namespace, PartyData);
	return true;
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
bool FOnlinePartySystemAccelByte::UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyMemberData)
#else
bool FOnlinePartySystemAccelByte::UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyMemberData)
#endif
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	// No ability natively to add/update data on a party member. This could potentially be supported by adding per-member objects
	// to existing global party storage, however this data could get stale and linger in party storage. For these reasons, this
	// will remain unimplemented for the time being.
	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::UpdatePartyMemberData is not supported!"));
	return false;
}

bool FOnlinePartySystemAccelByte::IsMemberLeader(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}
	
	// Convert the LocalUserId to a shared reference to a FUniqueNetIdAccelByte for searching
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared()));
		if (FoundParty != nullptr)
		{
			if ((*FoundParty)->LeaderId.IsValid())
			{			
				bool bIsMemberLeader = (*((*FoundParty)->LeaderId.Get()) == MemberId);
				return bIsMemberLeader;
			}
		}
	}
	return false;
}

uint32 FOnlinePartySystemAccelByte::GetPartyMemberCount(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return 0;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared()));
		if (FoundParty != nullptr)
		{
			return (*FoundParty)->GetMemberCount();
		}
	}
	return 0;
}

FOnlinePartyConstPtr FOnlinePartySystemAccelByte::GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return nullptr;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared()));
		if (FoundParty != nullptr)
		{
			return *FoundParty;
		}
	}
	return nullptr;
}

FOnlinePartyConstPtr FOnlinePartySystemAccelByte::GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return nullptr;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		for (const TPair<TSharedRef<const FOnlinePartyIdAccelByte>, TSharedRef<FOnlinePartyAccelByte>>& Pair : *FoundPartyMap)
		{
			FOnlinePartyTypeId TypeId = Pair.Value->PartyTypeId;
			if (TypeId.GetValue() == PartyTypeId.GetValue())
			{
				if (Pair.Value->GetMember(SharedUserId).IsValid())
				{
					return Pair.Value;
				}
			}
		}
	}

	return nullptr;
}

FOnlinePartyMemberConstPtr FOnlinePartySystemAccelByte::GetPartyMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return nullptr;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared()));
		if (FoundParty != nullptr)
		{
			return (*FoundParty)->GetMember(FUniqueNetIdAccelByteUser::CastChecked(MemberId));
		}
	}
	return nullptr;
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
FOnlinePartyDataConstPtr FOnlinePartySystemAccelByte::GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace) const
#else
FOnlinePartyDataConstPtr FOnlinePartySystemAccelByte::GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const
#endif
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return nullptr;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared()));
		if (FoundParty != nullptr)
		{
			return (*FoundParty)->GetPartyData();
		}
	}
	return nullptr;
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
FOnlinePartyDataConstPtr FOnlinePartySystemAccelByte::GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const FName& Namespace) const
#else
FOnlinePartyDataConstPtr FOnlinePartySystemAccelByte::GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const
#endif
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return nullptr;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::GetPartyMemberData and storing party member specific data is not supported!"));
	return nullptr;
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemAccelByte::GetAdvertisedParty(const FUniqueNetId& LocalUserId, const FUniqueNetId& UserId, const FOnlinePartyTypeId PartyTypeId) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return nullptr;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::GetAdvertisedParty is not implemented as advertised parties are unsupported!"));
	return nullptr;
}

bool FOnlinePartySystemAccelByte::GetJoinedParties(const FUniqueNetId& LocalUserId, TArray<TSharedRef<const FOnlinePartyId>>& OutPartyIdArray) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		OutPartyIdArray.Empty(FoundPartyMap->Num());
		for (const TPair<TSharedRef<const FOnlinePartyIdAccelByte>, TSharedRef<FOnlinePartyAccelByte>>& KV : *FoundPartyMap)
		{
			OutPartyIdArray.Add(KV.Key);
		}
		return true;
	}
	return false;
}

bool FOnlinePartySystemAccelByte::GetPartyMembers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<FOnlinePartyMemberConstRef>& OutPartyMembersArray) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared()));
		if (FoundParty != nullptr)
		{
			OutPartyMembersArray = (*FoundParty)->GetAllMembers();
		}
		return true;
	}
	return false;
}

bool FOnlinePartySystemAccelByte::GetPendingInvites(const FUniqueNetId& LocalUserId, TArray<IOnlinePartyJoinInfoConstRef>& OutPendingInvitesArray) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	const FPartyInviteArray* FoundInvites = UserIdToPartyInvitesMap.Find(FUniqueNetIdAccelByteUser::CastChecked(LocalUserId));
	if (FoundInvites != nullptr)
	{
		// If we found an array of invites for this user, then we want to add all of these invites to the out array
		// using the join info structure that was created along with the invite structure with extra information
		OutPendingInvitesArray.Reserve(FoundInvites->Num());
		for (const TSharedRef<const FAccelBytePartyInvite>& Invite : *FoundInvites)
		{
			OutPendingInvitesArray.Add(Invite->JoinInfo);
		}
		return true;
	}
	return false;
}

bool FOnlinePartySystemAccelByte::GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<IOnlinePartyPendingJoinRequestInfoConstRef>& OutPendingJoinRequestArray) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	UE_LOG_AB(Warning, TEXT("FOnlinePartySystemAccelByte::GetPendingJoinRequests is not implemented as join requests are unsupported!"));
	return false;
}

bool FOnlinePartySystemAccelByte::GetPendingInvitedUsers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutPendingInvitedUserArray) const
{
	if (WarnForUsingV1PartyWithV2Sessions())
	{
		return false;
	}

	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId);
	const FPartyIDToPartyMap* FoundPartyMap = UserIdToPartiesMap.Find(SharedUserId);
	if (FoundPartyMap != nullptr)
	{
		const TSharedRef<FOnlinePartyAccelByte>* FoundParty = FoundPartyMap->Find(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(PartyId.AsShared()));
		if (FoundParty != nullptr)
		{
			OutPendingInvitedUserArray = (*FoundParty)->GetAllPendingInvitedUsers();
			return true;
		}
	}
	return false;
}

FString FOnlinePartySystemAccelByte::MakeJoinInfoJson(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
{
	return TEXT("");
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemAccelByte::MakeJoinInfoFromJson(const FString& JoinInfoJson)
{
	return nullptr;
}

FString FOnlinePartySystemAccelByte::MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo& JoinInfo) const
{
	return TEXT("");
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemAccelByte::MakeJoinInfoFromToken(const FString& Token) const
{
	return nullptr;
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemAccelByte::ConsumePendingCommandLineInvite()
{
	return nullptr;
}

void FOnlinePartySystemAccelByte::DumpPartyState()
{
}

#undef ONLINE_ERROR_NAMESPACE
