// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlineUserCacheAccelByte.h"

class FOnlineSubsystemAccelByte;
struct FAccelByteModelsSessionBrowserRecentPlayerData;

/**
 * Implementation of a friend represented in the AccelByte backend
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineFriendAccelByte : public FOnlineFriend
{
PACKAGE_SCOPE:

	/** Constructor for creating a friend from a string display name and a shared ref to a friend ID, use if we already have an ID for a friend */
	FOnlineFriendAccelByte(const FString& InDisplayName, const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const EInviteStatus::Type& InInviteStatus);

	/** Method for the friends interface to update invite status if the friend changes state */
	void SetInviteStatus(const EInviteStatus::Type& InInviteStatus)
	{
		InviteStatus = InInviteStatus;
	}
	
public:
	
	//~ Begin FOnlineFriend
	virtual EInviteStatus::Type GetInviteStatus() const override;
	virtual const FOnlineUserPresence& GetPresence() const override;
	//~ End FOnlineFriend

	//~ Begin FOnlineUser
	virtual TSharedRef<const FUniqueNetId> GetUserId() const override;
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	//~ End FOnlineUser

private:

	/** Display name of this friend user */
	FString DisplayName;

	/** ID of this friend user */
	TSharedRef<const FUniqueNetIdAccelByteUser> UserId;
	
	/** Map of attributes for this friend user */
	TMap<FString, FString> UserAttributesMap;

	/**
	 * Status of our invite for this friend
	 * 
	 * Most other OSSes seem to just implement invite status as accepted, since you can manage invites through their platform.
	 * However, for our case, since friends are very game-by-game, at least for now, we want to be able to manage these friends
	 * from within the game, thus meaning that they will be returned in the friends list itself.
	 */
	EInviteStatus::Type InviteStatus;

	/** Presence info for this friend */
	FOnlineUserPresence Presence;

};

/**
 * Implementation of a blocked player represented in the AccelByte backend
 */
class FOnlineBlockedPlayerAccelByte : public FOnlineBlockedPlayer
{
PACKAGE_SCOPE:

	FOnlineBlockedPlayerAccelByte(const FString& InDisplayName, const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId);

public:

	//~ Begin FOnlineUser
	virtual TSharedRef<const FUniqueNetId> GetUserId() const override;
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	//~ End FOnlineUser

private:

	/** Display name of this blocked user */
	FString DisplayName;

	/** ID of this blocked user */
	TSharedRef<const FUniqueNetIdAccelByteUser> UserId;

	/** Map of attributes for this blocked user */
	TMap<FString, FString> UserAttributesMap;

};

using FBlockedPlayerArray = TArray<TSharedPtr<FOnlineBlockedPlayer>>;
using FUserIdToBlockedPlayersMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FBlockedPlayerArray, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FBlockedPlayerArray>>;

class FOnlineRecentPlayerAccelByte : public FOnlineRecentPlayer
{
public:
	// FOnlineUser

	virtual TSharedRef<const FUniqueNetId> GetUserId() const override;
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;

	// FOnlineRecentPlayer

	virtual FDateTime GetLastSeen() const override;
	virtual const FOnlineUserPresence& GetPresence() const;

	/** Init/default constructor */
	FOnlineRecentPlayerAccelByte(const FAccelByteUserInfo& InData);
	/** Platform Friend constructor */
	FOnlineRecentPlayerAccelByte(const TSharedRef<const FUniqueNetId>& InUserId, const TSharedRef<const FOnlineRecentPlayer>& InPlatformRecentPlayer)
		: UserId(InUserId)
		, PlatformRecentPlayer(InPlatformRecentPlayer)
	{
	}

	/** Virtual destructor to keep clang happy */
	virtual ~FOnlineRecentPlayerAccelByte() {};

PACKAGE_SCOPE:
	/** User Id represented as a FUniqueNetId */
	TSharedPtr<const FUniqueNetId> UserId;
	/** Cached Display Name - Used for Internal Friends List */
	FString DisplayName;
	/** Cached Last Seen - Used for Internal Friends List */
	FDateTime LastSeen;
	/** Cached Presence - Used for Internal Friends List */
	FOnlineUserPresence Presence;

	/** Platform Recent Player represented as a FOnlineRecentPlayer */
	TSharedPtr<const FOnlineRecentPlayer> PlatformRecentPlayer;

	/** Map of attributes for this blocked user */
	TMap<FString, FString> UserAttributesMap;
};

/**
 * Implementation of the IOnlineFriends interface using AccelByte services.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineFriendsAccelByte : public IOnlineFriends, public TSharedFromThis<FOnlineFriendsAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	
	/** Constructor that is invoked by the Subsystem instance to create a friend interface instance */
	FOnlineFriendsAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/**
	 * Method used by the Identity interface to register delegates for friend notifications to this interface to get
	 * real-time updates from the Lobby websocket.
	 */
	virtual void RegisterRealTimeLobbyDelegates(int32 LocalUserNum);

	/** Method used by async tasks to add an array of friends to the friend list */
	virtual void AddFriendsToList(int32 LocalUserNum, const TArray<TSharedPtr<FOnlineFriend>>& NewFriends);

	/** Method used by async tasks to add a single friend to the friends list */
	virtual void AddFriendToList(int32 LocalUserNum, const TSharedPtr<FOnlineFriend>& NewFriend);

	/** Method used by async tasks to remove a single friend from the friends list */
	virtual void RemoveFriendFromList(int32 LocalUserNum, const TSharedRef<const FUniqueNetIdAccelByteUser>& FriendId);

	/** Method used by async tasks to add an array of blocked users to the blocked users list */
	void AddBlockedPlayersToList(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TArray<TSharedPtr<FOnlineBlockedPlayer>>& NewBlockedPlayers);

	/** Method used by async tasks to remove a single blocked user to the blocked users list */
	void AddBlockedPlayerToList(int32 LocalUserNum, const TSharedPtr<FOnlineBlockedPlayer>& NewBlockedPlayer);

	/** Method used by async tasks to remove a single blocked player from the blocked players list */
	void RemoveBlockedPlayerFromList(int32 LocalUserNum, const TSharedRef<const FUniqueNetIdAccelByteUser>& PlayerId);

public:

	virtual ~FOnlineFriendsAccelByte() override = default;

	//~ Begin IOnlineFriends async methods
	virtual bool ReadFriendsList(int32 LocalUserNum, const FString& ListName, const FOnReadFriendsListComplete& Delegate = FOnReadFriendsListComplete()) override;
	virtual bool DeleteFriendsList(int32 LocalUserNum, const FString& ListName, const FOnDeleteFriendsListComplete& Delegate = FOnDeleteFriendsListComplete()) override;
	virtual bool SendInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnSendInviteComplete& Delegate = FOnSendInviteComplete()) override;
	virtual bool AcceptInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnAcceptInviteComplete& Delegate = FOnAcceptInviteComplete()) override;
	virtual bool RejectInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
	virtual bool DeleteFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
	virtual void SetFriendAlias(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FString& Alias, const FOnSetFriendAliasComplete& Delegate = FOnSetFriendAliasComplete()) override;
	virtual void DeleteFriendAlias(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnDeleteFriendAliasComplete& Delegate = FOnDeleteFriendAliasComplete()) override;
	virtual void AddRecentPlayers(const FUniqueNetId& UserId, const TArray<FReportPlayedWithUser>& InRecentPlayers, const FString& ListName, const FOnAddRecentPlayersComplete& InCompletionDelegate) override;
	virtual bool QueryRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace) override;
	virtual bool BlockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId) override;
	virtual bool UnblockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId) override;
	virtual bool QueryBlockedPlayers(const FUniqueNetId& UserId) override;
	//~ End IOnlineFriends async methods

	//~ Begin IOnlineFriends cached methods
	virtual bool GetFriendsList(int32 LocalUserNum, const FString& ListName, TArray<TSharedRef<FOnlineFriend>>& OutFriends) override;
	virtual TSharedPtr<FOnlineFriend> GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
	virtual bool IsFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName) override;
	virtual bool GetRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineRecentPlayer>>& OutRecentPlayers) override;
	virtual bool GetBlockedPlayers(const FUniqueNetId& UserId, TArray<TSharedRef<FOnlineBlockedPlayer>>& OutBlockedPlayers) override;
	virtual void DumpRecentPlayers() const override;
	virtual void DumpBlockedPlayers() const override;
	//~ End IOnlineFriends cached methods

PACKAGE_SCOPE:
	/** Map of UniqueId -> Recent Players List */
	TUniqueNetIdMap<TArray<TSharedRef<FOnlineRecentPlayerAccelByte>>> RecentPlayersMap;

protected:

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineFriendsAccelByte()
		: AccelByteSubsystem(nullptr) {}

	/** Map of user IDs representing local users to an array of FOnlineFriend instances */
	TMap<int32, TArray<TSharedPtr<FOnlineFriend>>> LocalUserNumToFriendsMap;

	/** Map of user IDs representing local users to an array of FOnlineBlockedPlayer instances */
	FUserIdToBlockedPlayersMap UserIdToBlockedPlayersMap;

	/** Delegate handler for when another user accepts our friend request */
	void OnFriendRequestAcceptedNotificationReceived(const FAccelByteModelsAcceptFriendsNotif& Notification, int32 LocalUserNum);

	/** Delegate handler for when we receive a friend request from another user */
	void OnFriendRequestReceivedNotificationReceived(const FAccelByteModelsRequestFriendsNotif& Notification, int32 LocalUserNum);

	/** Delegate handler for when another user removes us as a friend */
	void OnUnfriendNotificationReceived(const FAccelByteModelsUnfriendNotif& Notification, int32 LocalUserNum);

	/** Delegate handler for when another user rejects our friend request */
	void OnRejectFriendRequestNotificationReceived(const FAccelByteModelsRejectFriendsNotif& Notification, int32 LocalUserNum);

	/** Delegate handler for when another user cancels their friend request to us */
	void OnCancelFriendRequestNotificationReceived(const FAccelByteModelsCancelFriendsNotif& Notification, int32 LocalUserNum);

};