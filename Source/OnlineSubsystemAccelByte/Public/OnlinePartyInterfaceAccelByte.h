// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#if 1 // MMv1 Deprecation

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelBytePackage.h"

/** Log category for extra logging regarding parties */
DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteOSSParty, Warning, All);

// #NOTE (Voltaire) Define flags/markers for party notifications so that party members can process payload accordingly 
#define PARTYNOTIF_CUSTOMIZATION_CHANGED TEXT("Customization Changed")
#define PARTYNOTIF_PARTY_LEADER_JOINED_CUSTOM_GAME TEXT("Party Leader Joined Custom Game")
#define PARTYNOTIF_PARTY_MEMBER_INVITED_TO_CUSTOM_GAME TEXT("Party Member Invited To Custom Game")
#define PARTYNOTIF_PARTY_LEADER_CANCEL_MATCHMAKING TEXT("Party Leader Canceled Matchmaking")
#define PARTYNOTIF_CANCEL_MATCHMAKING_DUE_TO_KICK TEXT("Party Member Has Been Kicked Cancel Matchmaking For Kicked User")

#define CROSSPLAY_OBJECT_NAME TEXT("crossplayPlatformMap")
#define CROSSPLAY_OBJECT_PLAYER_PLATFORM_FIELD TEXT("platform")
#define CROSSPLAY_OBJECT_PLAYER_CROSSPLAY_FIELD TEXT("crossplay")

#define PARTYDATA_PARTYCODE_ATTR TEXT("partyCode")

class FOnlineSubsystemAccelByte;
class FOnlinePartySystemAccelByte;

/**
 * Basic implementation of party IDs for the AccelByte platform. Stores the ID as an FString internally.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlinePartyIdAccelByte : public FOnlinePartyId
{
public:
	FOnlinePartyIdAccelByte() = default;
	FOnlinePartyIdAccelByte(const FString& InIdStr);

	//~ Begin FOnlinePartyId overrides
	virtual const uint8* GetBytes() const override;
	virtual int32 GetSize() const override;
	virtual bool IsValid() const override;
	virtual FString ToString() const override;
	virtual FString ToDebugString() const override;
	//~ End FOnlinePartyId overrides

private:

	/** Party ID as an FString, should be a valid UUID v4 without the hyphens */
	FString IdStr;

};

/**
 * Key functions for indexing a map with a shared reference to an AccelByte party ID as a key.
 */
template <typename ValueType>
struct ONLINESUBSYSTEMACCELBYTE_API TPartyIdConstSharedRefMapKeyFuncs : public TDefaultMapKeyFuncs<TSharedRef<const FOnlinePartyIdAccelByte>, ValueType, false>
{
	static TSharedRef<const FOnlinePartyIdAccelByte> GetSetKey(const TTuple<TSharedRef<const FOnlinePartyIdAccelByte>, ValueType> Element)
	{
		return Element.Key;
	}

	static uint32 GetKeyHash(const TSharedRef<const FOnlinePartyIdAccelByte>& Key)
	{
		return GetTypeHash(Key.Get());
	}

	static bool Matches(const TSharedRef<const FOnlinePartyIdAccelByte>& A, const TSharedRef<const FOnlinePartyIdAccelByte>& B)
	{
		return (A == B) || (A.Get() == B.Get());
	}
};

/**
 * Basic implementation of a party member that can be grabbed from the party interface
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlinePartyMemberAccelByte : public FOnlinePartyMember
{
public:
	FOnlinePartyMemberAccelByte(const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FString& InDisplayName);

	//~ Begin FOnlineUser overrides
	virtual TSharedRef<const FUniqueNetId> GetUserId() const override;
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	//~ End FOnlineUser overrides

private:

	/** ID of the user represented as a party member */
	TSharedRef<const FUniqueNetIdAccelByteUser> UserId;

	/** Display name of the user represented as a party member */
	FString DisplayName;

	/** Attributes of the user represented as a party member */
	TMap<FString, FString> UserAttributes;

};

/** TPair for an invited user, with the first element being the user that invited them, and the second element being the invited user */
using FInvitedPlayerPair = TPair<TSharedRef<const FUniqueNetIdAccelByteUser>, TSharedRef<const FUniqueNetIdAccelByteUser>>;

/** Map of user IDs to party member instances */
using FUserIdToPartyMemberMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, TSharedRef<FOnlinePartyMemberAccelByte>, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<TSharedRef<FOnlinePartyMemberAccelByte>>>;

/**
 * Representation of a party on the AccelByte backend
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlinePartyAccelByte : public FOnlineParty
{
public:
	FOnlinePartyAccelByte(
		const TSharedRef<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe>& InOwningInterface, 
		const FString& InPartyId, const FString& InInviteToken, 
		const FPartyConfiguration& InPartyConfiguration, 
		const TSharedRef<const FUniqueNetIdAccelByteUser>& InLeaderId, 
		const TSharedRef<FOnlinePartyData>& InPartyData = MakeShared<FOnlinePartyData>(),
		const FOnlinePartyTypeId InPartyTypeId = FOnlinePartyTypeId(static_cast<uint32>(EAccelBytePartyType::PRIMARY_PARTY)));

	//~ Begin FOnlineParty overrides
	virtual bool CanLocalUserInvite(const FUniqueNetId& LocalUserId) const override;
	virtual bool IsJoinable() const override;
	virtual TSharedRef<const FPartyConfiguration> GetConfiguration() const override;
	//~ End FOnlineParty overrides

	/**
	 * Method to get the amount of members that are currently in this party;
	 */
	const uint32 GetMemberCount() const;

	/**
	 * #SG Adds the specified user's crossplay preference and platform to the party's storage.
	 */
	void AddPlayerCrossplayPreferenceAndPlatform(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId);

	/**
	 * #SG Removes the specified user's crossplay preference and platform from the party's storage.
	 */
	void RemovePlayerCrossplayPreferenceAndPlatform(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& UserToRemove);

	/**
	 * #TICKETID Adds the specified Leader TicketID to the party's storage.
	 */
	void AddPlayerAcceptedTicketId(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const FString& TicketId, const FString& MatchId);

	/**
	 * #TICKETID Removes the specified Leader TicketID from the party's storage.
	 */
	void RemovePlayerAcceptedTicketId(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId);

PACKAGE_SCOPE:

	/**
	 * Internal method to construct a party instance from an AccelByte party join info structure as well as a map of user IDs
	 * to display names for each member of the party.
	 */
	static TSharedRef<FOnlinePartyAccelByte> CreatePartyFromPartyInfo(const TSharedRef<const FUniqueNetIdAccelByteUser> LocalUserId, const TSharedRef<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface, const FAccelByteModelsPartyJoinResponse& PartyInfo, const TArray<FAccelByteUserInfoRef>& PartyMemberInfo, const TSharedRef<FOnlinePartyData>& InPartyData = MakeShared<FOnlinePartyData>(), const FString& PartyCode=TEXT(""), const FAccelByteModelsBulkUserStatusNotif& InPartyMemberStatus = FAccelByteModelsBulkUserStatusNotif());

	/**
	 * Internal method to construct a party instance from an AccelByte party info structure as well as a map of user IDs
	 * to display names for each member of the party.
	 */
	static TSharedRef<FOnlinePartyAccelByte> CreatePartyFromPartyInfo(const TSharedRef<const FUniqueNetIdAccelByteUser> LocalUserId, const TSharedRef<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface, const FAccelByteModelsInfoPartyResponse& PartyInfo, const TArray<FAccelByteUserInfoRef>& PartyMemberInfo, const TSharedRef<FOnlinePartyData>& InPartyData = MakeShared<FOnlinePartyData>(), const FString& PartyCode = TEXT(""), const FAccelByteModelsBulkUserStatusNotif& InPartyMemberStatus = FAccelByteModelsBulkUserStatusNotif());

	/** Internal method for interface and async tasks to add members to this party object */
	void AddMember(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<FOnlinePartyMemberAccelByte>& Member);

	/** Internal method for interface and async tasks to add players that have been invited to this party */
	void AddUserToInvitedPlayers(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InviterUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InvitedUserId);

	/** Method internally for interface and other tasks to get a member of this party by their ID */
	TSharedPtr<const FOnlinePartyMemberAccelByte> GetMember(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId) const;

	/** Internal method to remove a party member from a party, usually used in notifications or on kick */
	bool RemoveMember(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& RemovedUserId, const EMemberExitedReason& ExitReason);

	/** Internal method to remove a party invite by the invited user ID */
	void RemoveInvite(const TSharedRef<const FUniqueNetIdAccelByteUser>& LocalUserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InvitedUserId, const EPartyInvitationRemovedReason& PartyInviteRemoveReason);

	/** Internal method for getting an array of all current party members */
	TArray<FOnlinePartyMemberConstRef> GetAllMembers() const;

	/** Internal method for getting an array of user IDs for all players with currently pending invites */
	TArray<TSharedRef<const FUniqueNetId>> GetAllPendingInvitedUsers() const;

	/** Internal method for getting a reference to our party data */
	TSharedRef<const FOnlinePartyData> GetPartyData() const;

	/** Internal method to update party data */
	void SetPartyData(TSharedRef<FOnlinePartyData> PartyData);

	/** Internal method to set party code associated with this party instance */
	void SetPartyCode(const FString& PartyCode);

	/**
	 * Check whether this party is a crossplay enabled party or not. This will check the preferences of all members to see
	 * if their crossplay flag is enabled.
	 *
	 * @return true if all members have crossplay enabled, false otherwise
	 */
	bool IsCrossplayParty();

	/**
	 * Gets an array of the unique platforms that each party member is on
	 */
	TArray<FString> GetUniquePlatformsForParty();

private:

	/** Interface that owns this party instance, used to fire delegates on member changes */
	TSharedRef<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> OwningInterface;

	/** String representing the token that is sent to users to invite them to the party */
	FString InviteToken;

	/** Configuration used to set up the party */
	TSharedRef<const FPartyConfiguration> PartyConfiguration;

	/** Map of users that are currently in this party */
	FUserIdToPartyMemberMap UserIdToPartyMemberMap;

	/** Array of user IDs representing players that we have invited to this party */
	TArray<FInvitedPlayerPair> InvitedPlayers;

	/** Instance of party data that is grabbed from the backend and modified locally */
	TSharedRef<FOnlinePartyData> PartyData;

	friend class FOnlinePartySystemAccelByte;
};

class ONLINESUBSYSTEMACCELBYTE_API FOnlinePartyJoinInfoAccelByte : public IOnlinePartyJoinInfo
{
public:
	/** Default constructor for party join info, gives invalid for all members */
	FOnlinePartyJoinInfoAccelByte()
		: PartyId(MakeShared<const FOnlinePartyIdAccelByte>(TEXT("INVALID")))
		, SourceUserId(FUniqueNetIdAccelByteUser::Invalid())
		, SourceUserDisplayName(TEXT("INVALID"))
	{
	}

	/** Constructor for building a new FOnlinePartyJoinInfoAccelByte instance */
	FOnlinePartyJoinInfoAccelByte(const TSharedRef<const FOnlinePartyIdAccelByte>& InPartyId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InSourceUserId, const FString& InSourceUserDisplayName);

	/** Constructor for building a FOnlinePartyJoinInfoAccelByte from a IOnlinePartyJoinInfo reference */
	FOnlinePartyJoinInfoAccelByte(const IOnlinePartyJoinInfo& BaseInfo);

	//~ Begin IOnlinePartyJoinInfo overrides
	virtual bool IsValid() const override;
	virtual TSharedRef<const FOnlinePartyId> GetPartyId() const override;
	virtual FOnlinePartyTypeId GetPartyTypeId() const override;
	virtual TSharedRef<const FUniqueNetId> GetSourceUserId() const override;
	virtual const FString& GetSourceDisplayName() const override;
	virtual const FString& GetSourcePlatform() const override;
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	virtual const FString& GetPlatformData() const override;
#endif
	virtual bool HasKey() const override;
	virtual bool HasPassword() const override;
	virtual bool IsAcceptingMembers() const override;
	virtual bool IsPartyOfOne() const override;
	virtual int32 GetNotAcceptingReason() const override;
	virtual const FString& GetAppId() const override;
	virtual const FString& GetBuildId() const override;
	virtual bool CanJoin() const override;
	virtual bool CanJoinWithPassword() const override;
	virtual bool CanRequestAnInvite() const override;
	//~ End IOnlinePartyJoinInfo overrides

private:

	/** ID of the party that this join info is representing */
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId;

	/** ID of the user that sent the invite to us */
	TSharedRef<const FUniqueNetIdAccelByteUser> SourceUserId;

	/** Display name of the user that sent the invite to us */
	FString SourceUserDisplayName;

	/** Platform that this party invite originated from, not supported */
	FString SourcePlatform = TEXT("");

	/** Platform data for this party invite, not supported */
	FString PlatformData = TEXT("");

	/** App ID that the invite originated from, not supported */
	FString AppId = TEXT("");

	/** Build ID that the invite originated from, not supported */
	FString BuildId = TEXT("");

};

/** Typedef for a map of shared references to FOnlinePartyIdAccelByte objects to shared references to FOnlinePartyAccelByte objects */
using FPartyIDToPartyMap = TMap<TSharedRef<const FOnlinePartyIdAccelByte>, TSharedRef<FOnlinePartyAccelByte>, FDefaultSetAllocator, TPartyIdConstSharedRefMapKeyFuncs<TSharedRef<FOnlinePartyAccelByte>>>;

/** Typedef for a map of user IDs to maps of party IDs and party objects */
using FUserIDToPartiesMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FPartyIDToPartyMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FPartyIDToPartyMap>>;

/**
 * Structure representing information needed to act on an invite to an AccelByte party
 */
struct ONLINESUBSYSTEMACCELBYTE_API FAccelBytePartyInvite
{
	FAccelBytePartyInvite(const TSharedRef<const FOnlinePartyIdAccelByte>& InPartyId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InInviterId, const FString& InInviterDisplayName, const FString& InInviteToken)
		: PartyId(InPartyId)
		, InviterId(InInviterId)
		, InviterDisplayName(InInviterDisplayName)
		, InviteToken(InInviteToken)
		, JoinInfo(MakeShared<const FOnlinePartyJoinInfoAccelByte>(PartyId, InviterId, InviterDisplayName))
	{
	}

	/** ID of the party that this invitation is for */
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId;

	/** ID of the user that sent this party invitation */
	TSharedRef<const FUniqueNetIdAccelByteUser> InviterId;

	/** Display name of the user that sent this party invitation */
	FString InviterDisplayName;

	/** Token that corresponds to this invite to validate that it was a real invite */
	FString InviteToken;

	/** Join info for this party invite, will be returned to accessors of invites through the OSS */
	TSharedRef<const FOnlinePartyJoinInfoAccelByte> JoinInfo;
};

/** Array of party invite structures */
using FPartyInviteArray = TArray<TSharedRef<const FAccelBytePartyInvite>>;

/** Map of user IDs to an array of invite structures */
using FUserIdToPartyInvitesMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FPartyInviteArray, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FPartyInviteArray>>;

class ONLINESUBSYSTEMACCELBYTE_API FOnlinePartySystemAccelByte : public IOnlinePartySystem, public TSharedFromThis<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a friend interface instance */
	FOnlinePartySystemAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/** Initialize data relating to party interface */
	void Init();

	/**
	 * Method used by the Identity interface to register delegates for friend notifications to this interface to get
	 * real-time updates from the Lobby websocket.
	 */
	virtual void RegisterRealTimeLobbyDelegates(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId);

	/** Convenience method for async tasks to add a party object to the party map. */
	void AddPartyToInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<FOnlinePartyAccelByte>& Party);

	/** Convenience method for async tasks to remove a party object from the party map. */
	bool RemovePartyFromInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId);

	/** Convenience method for async tasks to remove a party object from the party map. */
	bool RemovePartyFromInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<FOnlinePartyAccelByte>& Party);

	/** Convenience method for async tasks to remove a party object from the party map. */
	bool RemovePartyFromInterface(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId);

	/**
	 * Method to get a pending invite for the user by a party ID.
	 *
	 * @return Shared pointer to an invite structure to act on the invite, or nullptr if not found
	 */
	TSharedPtr<const FAccelBytePartyInvite> GetInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId);

	/**
	 * Method to get a pending invite for the user by the ID of the user that invited them.
	 *
	 * @return Shared pointer to an invite structure to act on the invite, or nullptr if not found
	 */
	TSharedPtr<const FAccelBytePartyInvite> GetInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InviterId);

	/** Adds an invite to a party */
	void AddPartyInvite(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<FAccelBytePartyInvite>& Invite);

	/**
	 * Removes an invite to a party by the party's ID.
	 *
	 * @return boolean that is true if the removal was successful, or false if not
	 */
	bool RemoveInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId, const EPartyInvitationRemovedReason& InvitationRemovalReason);

	/**
	 * Removes an invite to a party by the ID of the user that invited them.
	 *
	 * @return boolean that is true if the removal was successful, or false if not
	 */
	bool RemoveInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FUniqueNetIdAccelByteUser>& InviterId, const EPartyInvitationRemovedReason& InvitationRemovalReason);

	/**
	 * Clear all invitations to a party.
	 *
	 * @return boolean that is true if the removal was successful, or false if not
	 */
	bool ClearInviteForParty(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedPtr<const FOnlinePartyId>& PartyId, const EPartyInvitationRemovedReason& InvitationRemovalReason);

	/**
	 * Convenience method for async tasks to remove the first party instance from a user's local cache.
	 *
	 * @return boolean that is true if the removal was successful, or false if not
	 */
	bool RemovePartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId);

	/**
	 * Convenience method for async tasks to remove a party instance from a user's local cache
	 *
	 * @return boolean that is true if the removal was successful, or false if not
	 */
	bool RemovePartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId);

	/** Convenience method to get first party for user. */
	TSharedPtr<FOnlinePartyAccelByte> GetFirstPartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId);

	/** Internal method to get a non-const AccelByte party object for operating on */
	TSharedPtr<FOnlinePartyAccelByte> GetPartyForUser(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TSharedRef<const FOnlinePartyIdAccelByte>& PartyId);

	/**
	 * Another method to join party using the shared party code
	 * 
	 * @param InPartyCode The party invitation code that is required to join a party.
	 * @return boolean that is true if the removal was successful, or false if not
	*/
	bool JoinParty(const FUniqueNetId& LocalUserId, const FString& InPartyCode, const FOnJoinPartyComplete& Delegate = FOnJoinPartyComplete());

public:

	virtual ~FOnlinePartySystemAccelByte() override = default;

	//~ Begin IOnlinePartySystem methods
	virtual void RestoreParties(const FUniqueNetId& LocalUserId, const FOnRestorePartiesComplete& CompletionDelegate) override;
	virtual void RestoreInvites(const FUniqueNetId& LocalUserId, const FOnRestoreInvitesComplete& CompletionDelegate) override;
	virtual void CleanupParties(const FUniqueNetId& LocalUserId, const FOnCleanupPartiesComplete& CompletionDelegate) override;
	virtual bool CreateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyConfiguration& PartyConfig, const FOnCreatePartyComplete& Delegate = FOnCreatePartyComplete()) override;
	virtual bool UpdateParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyConfiguration& PartyConfig, bool bShouldRegenerateReservationKey = false, const FOnUpdatePartyComplete& Delegate = FOnUpdatePartyComplete()) override;
	virtual bool JoinParty(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnJoinPartyComplete& Delegate = FOnJoinPartyComplete()) override;
	virtual bool RejoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyTypeId& PartyTypeId, const TArray<TSharedRef<const FUniqueNetId>>& FormerMembers, const FOnJoinPartyComplete& Delegate = FOnJoinPartyComplete()) override;
	virtual bool LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnLeavePartyComplete& Delegate = FOnLeavePartyComplete()) override;
	virtual bool LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, bool bSynchronizeLeave, const FOnLeavePartyComplete& Delegate = FOnLeavePartyComplete()) override;
	virtual bool ApproveJoinRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode = 0) override;

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
	virtual bool JIPFromWithinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& PartyLeaderId);
	virtual bool ApproveJIPRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode = 0);
#else
	virtual bool JIPFromWithinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& PartyLeaderId) override;
	virtual bool ApproveJIPRequest(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bIsApproved, int32 DeniedResultCode = 0) override;
#endif

#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	virtual void QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityComplete& Delegate = FOnQueryPartyJoinabilityComplete()) override;
	virtual void RespondToQueryJoinability(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, bool bCanJoin, int32 DeniedResultCode = 0) override;
#endif

	virtual bool SendInvitation(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyInvitationRecipient& Recipient, const FOnSendPartyInvitationComplete& Delegate = FOnSendPartyInvitationComplete()) override;
	virtual bool RejectInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId) override;
	virtual void ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId = nullptr) override;
	virtual bool KickMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnKickPartyMemberComplete& Delegate = FOnKickPartyMemberComplete()) override;
	virtual bool PromoteMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& TargetMemberId, const FOnPromotePartyMemberComplete& Delegate = FOnPromotePartyMemberComplete()) override;
	virtual bool IsMemberLeader(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	virtual uint32 GetPartyMemberCount(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual FOnlinePartyConstPtr GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual FOnlinePartyConstPtr GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId) const override;
	virtual FOnlinePartyMemberConstPtr GetPartyMember(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	virtual IOnlinePartyJoinInfoConstPtr GetAdvertisedParty(const FUniqueNetId& LocalUserId, const FUniqueNetId& UserId, const FOnlinePartyTypeId PartyTypeId) const override;
	virtual bool GetJoinedParties(const FUniqueNetId& LocalUserId, TArray<TSharedRef<const FOnlinePartyId>>& OutPartyIdArray) const override;
	virtual bool GetPartyMembers(const FUniqueNetId & LocalUserId, const FOnlinePartyId & PartyId, TArray<FOnlinePartyMemberConstRef>&OutPartyMembersArray) const override;
	virtual bool GetPendingInvites(const FUniqueNetId & LocalUserId, TArray<IOnlinePartyJoinInfoConstRef>&OutPendingInvitesArray) const override;
	virtual bool GetPendingJoinRequests(const FUniqueNetId & LocalUserId, const FOnlinePartyId & PartyId, TArray<IOnlinePartyPendingJoinRequestInfoConstRef>&OutPendingJoinRequestArray) const override;
	virtual bool GetPendingInvitedUsers(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<TSharedRef<const FUniqueNetId>>& OutPendingInvitedUserArray) const override;
	virtual FString MakeJoinInfoJson(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) override;
	virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromJson(const FString& JoinInfoJson) override;
	virtual FString MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo& JoinInfo) const override;
	virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromToken(const FString& Token) const override;
	virtual IOnlinePartyJoinInfoConstPtr ConsumePendingCommandLineInvite() override;
	virtual void DumpPartyState() override;

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	virtual bool UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyData) override;
	virtual FOnlinePartyDataConstPtr GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace) const override;
	virtual FOnlinePartyDataConstPtr GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const FName& Namespace) const override;
	virtual bool UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyMemberData) override;
#else
	virtual bool UpdatePartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyData) override;
	virtual FOnlinePartyDataConstPtr GetPartyData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) const override;
	virtual FOnlinePartyDataConstPtr GetPartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) const override;
	virtual bool UpdatePartyMemberData(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FOnlinePartyData& PartyMemberData) override;
#endif

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27) 
	virtual void QueryPartyJoinability(const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& OnlinePartyJoinInfo, const FOnQueryPartyJoinabilityCompleteEx& Delegate = FOnQueryPartyJoinabilityCompleteEx()) override;
	virtual void RespondToQueryJoinability(const FUniqueNetId & LocalUserId, const FOnlinePartyId & PartyId, const FUniqueNetId & RecipientId, bool bCanJoin, int32 DeniedResultCode, FOnlinePartyDataConstPtr PartyData) override;
#endif

#if (ENGINE_MAJOR_VERSION == 5)
	virtual void RequestToJoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId PartyTypeId, const FPartyInvitationRecipient& Recipient, const FOnRequestToJoinPartyComplete& Delegate = FOnRequestToJoinPartyComplete()) override;
	virtual void ClearRequestToJoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& Sender, EPartyRequestToJoinRemovedReason Reason) override;
	virtual bool GetPendingRequestsToJoin(const FUniqueNetId& LocalUserId, TArray<IOnlinePartyRequestToJoinInfoConstRef>& OutRequestsToJoin) const override;
#if (ENGINE_MINOR_VERSION >= 1)
	virtual void CancelInvitation(const FUniqueNetId& LocalUserId, const FUniqueNetId& TargetUserId, const FOnlinePartyId& PartyId, const FOnCancelPartyInvitationComplete& Delegate = FOnCancelPartyInvitationComplete()) override;
#endif // (ENGINE_MINOR_VERSION >= 1)
#if (ENGINE_MINOR_VERSION >= 2)
	virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfo(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) override;
#endif // (ENGINE_MINOR_VERSION >= 2)
#endif // (ENGINE_MAJOR_VERSION == 5)
	
	//~ End IOnlinePartySystem methods

	/**
	 * Convenience method for async tasks to check if the user passed in is currently in a party with the specified ID
	 */
	virtual bool IsPlayerInParty(const FUniqueNetId& UserId, const FOnlinePartyId& PartyId);

	/**
	 * Convenience method to check if the current user is the member of any party currently
	 */
	virtual bool IsPlayerInAnyParty(const FUniqueNetId& UserId);

	/**
	 * Convenience method to get the current party member count for the player's current party
	 *
	 * @return an integer representing the amount of players in the user's party, or -1 if not in a party
	 */
	int32 GetCurrentPartyMemberCount(const FUniqueNetId& UserId);

	/**
	 * Convenience method to get first party for user.
	 */
	TSharedPtr<const FOnlinePartyId> GetFirstPartyIdForUser(const FUniqueNetId & UserId);
	/**
	 * @return party type id for the primary party - the primary party is the party that will be addressable via the social panel
	 */
	static const FOnlinePartyTypeId GetAccelBytePartyTypeId() { return FOnlinePartyTypeId(static_cast<uint32>(EAccelBytePartyType::PRIMARY_PARTY)); }

	/**
	* Get a PartyCode from the created party for invitation and joining purpose 
	* Related to JoinParty function that requires const FString& InPartyCode
	* @param Output Reference to the returned result if operation is success/true.
	* @param LocalUserId
	* @param PartyId
	* 
	* @return if the operation is success.
	*/
	bool GetPartyCode(FString& Output, const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId);

protected:

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlinePartySystemAccelByte()
		: AccelByteSubsystem(nullptr) {}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

	/** Map of user IDs to a map of party IDs and their associated party objects */
	FUserIDToPartiesMap UserIdToPartiesMap;

	/** Use this lock to prevent racing condition when accessing the UserIdToPartiesMap */
	FCriticalSection UserIdToPartiesMapLock;

	/** Map of user IDs associated with an array of party invites */
	FUserIdToPartyInvitesMap UserIdToPartyInvitesMap;

	/** Store an array of delegates to execute when party join complete */
	TArray<FOnPartyJoinedDelegate> OnPartyJoinedPendingTasks;

	FOnCreatePartyComplete OnCreatePartyBeforeJoinCustomGameComplete;
	bool bIsAcceptingCustomGameInvitation = false;

private:
	/** Delegate handler for when we receive a party invite */
	void OnReceivedPartyInviteNotification(const FAccelByteModelsPartyGetInvitedNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);

	/** Delegate handler for when a party invite is sent by a remote user to a remote user */
	void OnPartyInviteSentNotification(const FAccelByteModelsInvitationNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);

	/** Delegate handler for when a remote user joins our party */
	void OnPartyJoinNotification(const FAccelByteModelsPartyJoinNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);

	/** Delegate handler for when a remote user leaves our party */
	void OnPartyMemberLeaveNotification(const FAccelByteModelsLeavePartyNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);

	/** Delegate handler for when a remote user is kicked from our party */
	void OnPartyKickNotification(const FAccelByteModelsGotKickedFromPartyNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);

	void OnPartyMemberConnectNotification(const FAccelByteModelsPartyMemberConnectionNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);

	void OnPartyMemberDisconnectNotification(const FAccelByteModelsPartyMemberConnectionNotice& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);
	
	/** Delegate handler for when party data changes */
	void OnPartyDataChangeNotification(const FAccelByteModelsPartyDataNotif& Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId);
	
	/** Convenience function for executing code after party joined complete. Used for when local user is still joining a party */
	void RunOnPartyJoinedComplete(const FOnPartyJoinedDelegate& Delegate);

	/** Delegate handler for when the local user joins a party */
	void OnPartyJoinedComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId);

	/** Delegate handler for when the local user joins a party. Used for cases to successfully retrieve the party for the local user where local user is still joining a party */
	void OnPartyJoinedCompleteMemberLeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FAccelByteModelsLeavePartyNotice Notification, TSharedRef<const FUniqueNetIdAccelByteUser> UserId, TSharedRef<const FUniqueNetIdAccelByteUser> LocalAccelByteUserId);

	/** Delegate handler to handle new leader's requests for the PartyCode from an AsyncTask. Therefore, it can set PartyCode to the Party */
	void OnPromotedLeaderGetPartyCode(bool bWasSuccessful, const FString& PartyCode, const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& PartyId);

};
#endif