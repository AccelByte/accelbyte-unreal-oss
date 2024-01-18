// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteGroupModels.h"
#include "Interfaces/OnlineGroupsInterface.h"
#include "OnlineSubsystemAccelBytePackage.h"

class FAccelByteGroupsInfo;

typedef TSharedRef<FAccelByteGroupsInfo> FAccelByteGroupsInfoRef;
typedef TSharedPtr<FAccelByteGroupsInfo> FAccelByteGroupsInfoPtr;

/**
 * Container for local AccelByte FAccelByteModelsGroupInformation and Unreal IGroupInfo
 */
class ONLINESUBSYSTEMACCELBYTE_API FAccelByteGroupsInfo : public IGroupInfo
{
public:

	/** Constructor */
	explicit FAccelByteGroupsInfo(
		const FUniqueNetIdRef& InSenderUserId,
		const FString InNamespace,
		const FUniqueNetId& InOwnerId,
		const FDateTime InTimeCreated,
		const FDateTime InTimeLastUpdated,
		const FString InABAdminRoleId,
		const FString InABMemberRoleId,
		const FAccelByteModelsGroupInformation InABGroupInfo);

	/** Destructor */
	virtual ~FAccelByteGroupsInfo() {};

	static FAccelByteGroupsInfoRef Create(const FUniqueNetIdRef& InSenderUserId,
		const FString InNamespace,
		const FUniqueNetId& InOwnerId,
		const FAccelByteModelsGroupInformation InABGroupInfo,
		const FString AdminRoleId,
		const FString MemberRoleId);

	//~ Begin IGroupInfo overrides

	/* Deprecated. Use ABGroupInfo.GroupId */
	virtual FUniqueNetIdRef GetGroupId() const override;
	virtual const FString& GetNamespace() const override;
	virtual FUniqueNetIdRef GetOwner() const override;

	/* Deprecated. Use ABGroupInfo */
	virtual const FGroupDisplayInfo& GetDisplayInfo() const override;

	/* Deprecated. Use ABAdminRoleId */
	virtual FString GetAdminRoleId() { return ""; }

	/* Deprecated. Use ABMemberRoleId */
	virtual FString GetMemberRoleId() { return ""; }

	virtual uint32 GetSize() const override;
	virtual const FDateTime& GetCreatedAt() const override;
	virtual const FDateTime& GetLastUpdated() const override;
	//~ End IGroupInfo overrides

	/**
	 * Method that sets a local (non-Server) copy of FAccelByteModelsGroupInformation
	 *
	 * @param GroupInfo - Sets the local copy of ABGroupInfo
	 */
	virtual void SetCachedABGroupInfo(const FAccelByteModelsGroupInformation& GroupInfo);

	/**
	 * Method that sets a local (non-Server) copy of FString for the Admin Role Id
	 *
	 * @param AdminRoleId - Sets the local copy of ABAdminRoleId
	 */
	virtual void SetCachedABAdminRoleId(const FString& AdminRoleId);

	/**
	 * Method that sets a local (non-Server) copy of FString for the Member Role Id
	 *
	 * @param AdminRoleId - Sets the local copy of ABMemberRoleId
	 */
	virtual void SetCachedABMemberRoleId(const FString& MemberRoleId);

	//~ Begin IGroupInfo variables
	FString Namespace{};

PACKAGE_SCOPE:

	FUniqueNetIdRef SenderUserId;
	TSharedPtr<const FUniqueNetIdAccelByteUser> OwnerId = nullptr;
	FDateTime TimeCreated{};
	FDateTime TimeLastUpdated{};
	//~ End IGroupInfo variables

	//~ Begin AccelByte variables
	FAccelByteModelsGroupInformation ABGroupInfo{};
	FString ABAdminRoleId{};
	FString ABMemberRoleId{};
	//~ End AccelByte variables
};

/**
 * AccelByte service implementation of the Groups Interface
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineGroupsAccelByte : public IOnlineGroups, public TSharedFromThis<FOnlineGroupsAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineGroupsAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
		: AccelByteSubsystem(InSubsystem)
	{}

public:

	virtual ~FOnlineGroupsAccelByte() override = default;

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineGroupsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	//~ Start - AccelByte Methods
	/**
	 * Used for creating a group for the first time
	 *
	 * @param UserIdCreatingGroup - User Id that is creating the group. This user automatically becomes the Group Admin.
	 * @param InGroupInfo - Initial group information supplied to create the group.
	 * @param OnCompleted - Return delegate indicating the success of this method.
	 */
	virtual void CreateGroup(const FUniqueNetId& UserIdCreatingGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Finds a number of previously created Groups to be found given the provided content
	 *
	 * @param SearchingUserId - UserId requesting the search
	 * @param RequestedContent - Content the user is requesting from the search
	 * @param OnCompleted - Return delegate indicating the success of this method.
	 */
	virtual void FindGroups(const FUniqueNetId& SearchingUserId, const FAccelByteModelsGetGroupListRequest& RequestedContent, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * This method returns a cache of data generated by the 'FindGroups' method
	 *
	 * @returns FAccelByteModelsGetGroupListResponse - List of Groups found
	 */
	virtual FAccelByteModelsGetGroupListResponse GetCachedFindGroupsRoster();

	/**
	 * This method returns a cache of data generated by the 'FindGroupsByGroupIds' method
	 *
	 * @returns FAccelByteModelsGetGroupListResponse - List of Groups found
	 */
	virtual FAccelByteModelsGetGroupListResponse GetCachedFindGroupsByGroupIds();

	/**
	 * Finds a number of previously created Groups to be found by X number of Group IDs
	 *
	 * @param SearchingUserId - UserId requesting the search
	 * @param GroupIds - Array of Group IDs to find
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void FindGroupsByGroupIds(const FUniqueNetId& SearchingUserId, const TArray<FString> GroupIds, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Gets up to date FAccelByteModelsGroupInformation for the provided group information. Use GetDisplayInfo() to retrieve queried data.
	 *
	 * @param CurrentUserId - User ID requesting the group information
	 * @param InGroupInfo - Group information to query
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void QueryGroupInfo(const FUniqueNetId& CurrentUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * After calling 'CreateGroup' this method can be used to retrieve the cached data of the group
	 *
	 * @returns FAccelByteGroupsInfoPtr - Complete data cache of the recently created group called 'CurrentGroup'
	 */
	virtual FAccelByteGroupsInfoPtr GetCachedGroupInfo();

	/**
	 * Used by 'CreateGroup' to set the cached 'CurrentGroup' data. Used to cache local data, but does not change server data.
	 *
	 * @param InGroupInfo - Incoming data to overwrite the existing data cache
	 */
	virtual void SetCachedGroupInfo(const FAccelByteGroupsInfoRef& InGroupInfo);

	/**
	 * Used by 'DeleteGroup' to not only delete server data, but also locally cached 'CurrentGroup' data.
	 */
	virtual void DeleteCachedGroupInfo();

	/**
	 * Used by a non-Group user to request to join the current group
	 *
	 * @param UserIdJoiningGroup - User ID requesting to join the group
	 * @param InGroupInfo - Group the user is requesting to join
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void JoinGroup(const FUniqueNetId& UserIdJoiningGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used by any Group user to Leave the Group. Last remaining Group Admin must be the last to leave a group.
	 *
	 * @param UserIdLeavingGroup - User ID requesting to leave the group
	 * @param InGroupInfo - Group the user is requesting to leave
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void LeaveGroup(const FUniqueNetId& UserIdLeavingGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used by a non-Group user to cancel a request to join a group
	 *
	 * @param UserIdCancelingRequest - User ID requesting to cancel their request to join
	 * @param InGroupInfo- Group the user is canceling their request from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void CancelRequest(const FUniqueNetId& UserIdCancelingRequest, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);	

	/**
	 * After calling 'InviteUser', the user invited uses this method to accept the group invite request
	 *
	 * @param UserIdAcceptedIntoGroup - User ID accepting the invite
	 * @param InGroupInfo - Group the user is accepting the invite from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void AcceptInvite(const FUniqueNetId& UserIdAcceptedIntoGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * After calling 'InviteUser', the user uses this method to decline an group invite request
	 *
	 * @param UserIdToDecline - User ID declining the invite
	 * @param InGroupInfo - Group the user is declining the invite from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void DeclineInvite(const FUniqueNetId& UserIdToDecline, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Retrieves a complete list of members in the group
	 *
	 * @param RequestingUserId - User ID requesting the list of members
	 * @param InGroupInfo - Group the user wants the list of members from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void QueryGroupRoster(const FUniqueNetId& RequestingUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsGetGroupMembersListByGroupIdRequest& RequestedContent, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * After calling 'QueryGroupRoster' this method can be used to retrieve the list of cached group members requested
	 *
	 * @returns FAccelByteModelsGetGroupMemberListResponse - Complete list of group members queried from 'QueryGroupRoster'
	 */
	virtual FAccelByteModelsGetGroupMemberListResponse GetCachedGroupRoster();

	/**
	 * Used to get a list of cached group members from the 'CurrentGroup'
	 *
	 * @param GroupRoster - Out parameter of the list of members in 'CurrentGroup'
	 */
	virtual void GetCachedUserMembership(TArray<FAccelByteModelsGroupMember>& GroupRoster);

	/**
	 * After calling 'QueryGroupInvites' this method can be used to retrieve the list of cached group invites requested
	 *
	 * @returns FAccelByteModelsGetMemberRequestsListResponse - Complete list of group invites queried from 'QueryGroupInvites'
	 */
	virtual FAccelByteModelsGetMemberRequestsListResponse GetCachedInvitations();

	/**
	 * Used to cancel an invite sent to a user
	 *
	 * @param AdminUserId - Must be a group Admin with permissions to delete invites
	 * @param UserIdToCancel - User ID used to find the invite in which to cancel
	 * @param InGroupInfo - Group to cancel the invite from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void CancelInvite(const FUniqueNetId& AdminUserId, const FString& UserIdToCancel, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Updates specific group data contained in 'FAccelByteModelsGroupUpdatable' on the server
	 *
	 * @param GroupAdmin - Required local Admin user number to perform this action
	 * @param AdminUserId - Required Admin with permissions to update group information
	 * @param InGroupInfo - Group that will be receiving the updated information
	 * @param RequestContent - Content to change in the group information
	 * @param CompletelyReplace - True, will overwrite all data to the incoming data. False, will overwrite only the filled data.
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void UpdateGroupInfo(const int32& GroupAdmin, const FUniqueNetId& AdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsGroupUpdatable& RequestContent, const bool CompletelyReplace, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * After non-Member user calls "JoinGroup", this method can be used to accept their join request
	 *
	 * @param AdminLocalUserNum - Local Admin user number accepting the request
	 * @param UserIdToAccept - User ID to accept into the group
	 * @param InGroupInfo - Group the user is joining
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void AcceptUser(const int32 AdminLocalUserNum, const FUniqueNetId& UserIdToAccept, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * After non-Member user calls "JoinGroup", this method can be used to decline their join request
	 *
	 * @param AdminLocalUserNum - Local Admin user number declining the request
	 * @param UserIdToDecline - User ID to decline into the group
	 * @param InGroupInfo - Group the user is being declined from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void DeclineUser(const int32 AdminLocalUserNum, const FUniqueNetId& UserIdToDecline, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to send an invite to a non-Member user to join the group
	 *
	 * @param InviterUserId - Group Member with permission to invite another user into the group
	 * @param InvitedUserId - Non-Member user receiving the invitation
	 * @param InGroupInfo - Group the invite is coming from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void InviteUser(const FUniqueNetId& InviterUserId, const FUniqueNetId& InvitedUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to remove/kick a Member or Admin from the group
	 *
	 * @param AdminLocalUserNum - Local Admin user number with permissions to remove a user
	 * @param MemberUserIdToKick - User ID to kick/remove from the group
	 * @param InGroupInfo - Group to kick the user from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void RemoveUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToKick, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to promote a group Member to a group Admin
	 *
	 * @param AdminLocalUserNum - Local Admin user number with permissions to promote a user
	 * @param MemberUserIdToPromote - User ID to promote
	 * @param InGroupInfo - Group to promote the user in
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void PromoteUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToPromote, const FAccelByteGroupsInfo& InGroupInfo, const FString& MemberRoleId, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to demote a group Member to a group Admin
	 *
	 * @param AdminLocalUserNum - Local Admin user number with permissions to demote a user
	 * @param MemberUserIdToDemote - User ID to demote
	 * @param InGroupInfo - Group to demote the user in
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void DemoteUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToDemote, const FAccelByteGroupsInfo& InGroupInfo, const FString& MemberRoleId, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used by a non-Member user to get all their group invites
	 *
	 * @param ContextUserId - User ID requesting the query
	 * @param AccelByteModelsLimitOffsetRequest - Requested content of the query
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void QueryGroupInvites(const FUniqueNetId& ContextUserId, const FAccelByteModelsLimitOffsetRequest& AccelByteModelsLimitOffsetRequest, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * After calling 'QueryGroupInvites' this method can be used to retrieve the list of cached group invites requested
	 *
	 * @returns FAccelByteModelsGetMemberRequestsListResponse - List of all invites sent to a given user requested from 'QueryGroupInvites'
	 */
	virtual FAccelByteModelsGetMemberRequestsListResponse GetCachedGroupInvites();

	/**
	 * Used by a Member user to get all their group requests sent out to other non-Member users
	 *
	 * @param ContextUserId - User ID requesting the query
	 * @param AccelByteModelsLimitOffsetRequest - Requested content of the query
	 * @param InGroupInfo - Group to retrieve the requests from
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void QueryGroupRequests(const FUniqueNetId& ContextUserId, const FAccelByteModelsLimitOffsetRequest& AccelByteModelsLimitOffsetRequest, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * After calling 'QueryGroupRequests' this method can be used to retrieve the list of cached group requests requested
	 *
	 * @param FAccelByteModelsGetMemberRequestsListResponse - List of all requests sent by the given group after calling 'QueryGroupRequests'
	 */
	virtual FAccelByteModelsGetMemberRequestsListResponse GetCachedGroupRequests();

	/**
	 * Used to get the maximum allowed user population of the local 'CurrentGroup' cache
	 *
	 * @returns int - Maximum allowed user population
	 */
	virtual int QueryConfigHeadcount();

	/**
	 * Used to Delete the local and server instance of a given group
	 *
	 * @param AdminLocalUserNum - Admin group member with permission to delete a group
	 * @param InGroupInfo - Group information to delete
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void DeleteGroup(const int32 AdminLocalUserNum, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to get the current Namespace
	 *
	 * @returns const FString& - Current namespace
	 */
	virtual const FString& GetNamespace() const override { return CachedCurrentGroup->Namespace; }

	/**
	 * Used to update a groups FAccelByteModelsGroupInformation data
	 *
	 * @param AdminLocalUserNum - Local Admin number requesting to change group data
	 * @param InAdminUserId - Admin ID with permissions to change group data
	 * @param InGroupInfo - Group in which to change information
	 * @param RequestContent - Requested group information to change
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void UpdateGroupCustomAttributes(const int32& AdminLocalUserNum, const FUniqueNetId& InAdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsUpdateGroupCustomAttributesRequest& RequestContent, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to CREATE or UPDATE a custom rule. Custom Rules are not enforced by AccelByte.
	 *
	 * @param AdminLocalUserNum - Local Admin number requesting to update a group custom rule
	 * @param InAdminUserId - Admin ID with permissions to change custom rules
	 * @param InGroupInfo - Group receiving the custom rule
	 * @param RequestContent - Custom rule data to add or update
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void UpdateGroupCustomRule(const int32& AdminLocalUserNum, const FUniqueNetId& InAdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsUpdateCustomRulesRequest& RequestContent, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to CREATE or UPDATE a predefined rule.
	 *
	 * @param AdminLocalUserNum - Local Admin number requesting to update a group predefined rule
	 * @param InAdminUserId - Admin ID with permissions to change predefined rules
	 * @param InGroupInfo - Group to create or update the predefined rule from
	 * @param InAllowedAction - Allowed action to update in the predefined rule
	 * @param RequestContent - Content to create or update in the predefined rule
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void UpdatePredefinedRule(const int32& AdminLocalUserNum, const FUniqueNetId& AdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const EAccelByteAllowedAction& InAllowedAction, const FAccelByteModelsUpdateGroupPredefinedRuleRequest& RequestContent, const FOnGroupsRequestCompleted& OnCompleted);

	/**
	 * Used to DELETE a predefined rule.
	 *
	 * @param AdminLocalUserNum - Local Admin number requesting to delete a group predefined rule
	 * @param InGroupInfo - Group to delete the predefined rule from
	 * @param AllowedAction - Allowed action to delete
	 * @param OnCompleted - Return delegate indicating the success of this method
	 */
	virtual void DeletePredefinedRule(const int32& AdminLocalUserNum, const FAccelByteGroupsInfo& InGroupInfo, const EAccelByteAllowedAction& AllowedAction, const FOnGroupsRequestCompleted& OnCompleted);
	//~ End - AccelByte Methods

	//~ End - Deprecated Methods
	virtual void CreateGroup(const FUniqueNetId& ContextUserId, const FGroupDisplayInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void FindGroups(const FUniqueNetId& ContextUserId, const FGroupSearchOptions& SearchOptions, const FOnFindGroupsCompleted& OnCompleted) override;
	virtual void QueryGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void QueryGroupNameExist(const FUniqueNetId& ContextUserId, const FString& GroupName, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual TSharedPtr<const IGroupInfo> GetCachedGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId) override;
	virtual void JoinGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void LeaveGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void CancelRequest(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void AcceptInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void DeclineInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual TSharedPtr<const IGroupRoster> GetCachedGroupRoster(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId) override;
	virtual void QueryGroupRoster(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void QueryUserMembership(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual TSharedPtr<const IUserMembership> GetCachedUserMembership(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId) override;
	virtual void QueryOutgoingInvitations(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual TSharedPtr<const IApplications> GetCachedApplications(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId) override;
	virtual TSharedPtr<const IInvitations> GetCachedInvitations(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId) override;
	virtual void CancelInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void UpdateGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FGroupDisplayInfo& GroupInfo, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void AcceptUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void DeclineUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void InviteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, bool bAllowBlocked, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void InviteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override { InviteUser(ContextUserId, GroupId, UserId, false, OnCompleted); }
	virtual void RemoveUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void PromoteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void DemoteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void QueryGroupInvites(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual TSharedPtr<const IGroupInvites> GetCachedGroupInvites(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId) override;
	virtual void QueryGroupRequests(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual TSharedPtr<const IGroupRequests> GetCachedGroupRequests(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId) override;
	virtual void QueryConfigHeadcount(const FUniqueNetId& ContextUserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void DeleteGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override;
	//~ End - Deprecated Methods

	// Start - No AccelByte Support
	virtual void QueryOutgoingApplications(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void QueryIncomingInvitations(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void BlockUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override {}
	virtual void UnblockUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override {}
#if ENGINE_MAJOR_VERSION >= 5
	virtual void QueryGroupDenylist(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override {}
	virtual TSharedPtr<const IGroupDenylist> GetCachedGroupDenylist(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId) override { return nullptr; }
#else
	virtual void QueryGroupBlacklist(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted) override {}
	virtual TSharedPtr<const IGroupBlacklist> GetCachedGroupBlacklist(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId) override { return nullptr; }
#endif
	virtual void QueryIncomingApplications(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual void QueryConfigMembership(const FUniqueNetId& ContextUserId, const FOnGroupsRequestCompleted& OnCompleted) override;
	virtual TSharedPtr<const FGroupConfigEntryInt> GetCachedConfigInt(const FString& Key) override { return nullptr; }
	virtual TSharedPtr<const FGroupConfigEntryBool> GetCachedConfigBool(const FString& Key) override { return nullptr; }
	virtual void TransferGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& NewOwnerId, const FOnGroupsRequestCompleted& OnCompleted) override {}
	virtual void SetNamespace(const FString& Ns) override {}
	// End - No AccelByte Support

	// Helper Functions --
	/**
	 * Ensures that local group data is not null
	 *
	 * @returns boolean that is true group data is not null
	 */
	bool IsGroupValid() const;

	/**
	 * Ensures that the incoming group data is valid and ready to be used
	 *
	 * @param InGroupInfo - Group data to be verified
	 * @returns boolean that is true if group data is valid
	 */
	bool VerifyGroupInfo(const FAccelByteGroupsInfo& InGroupInfo);

	/**
	 * Sets cached group data after a query of group information
	 *
	 * @param AccelByteModelsGetGroupListResponse - Sets the local copy of CachedGroupResults
	 */
	void SetCachedGroupResults(FAccelByteModelsGetGroupListResponse& AccelByteModelsGetGroupListResponse);

	/**
	 * Sets cached group data after a query of group information
	 *
	 * @param AccelByteModelsGetGroupListResponse - Sets the local copy of CachedGroupListByGroupIdsResults
	 */
	void SetCachedGroupListByGroupIdsResults(FAccelByteModelsGetGroupListResponse& AccelByteModelsGetGroupListResponse);

	/**
	 * Sets cached group data after a query of group information
	 *
	 * @param AccelByteModelsGetGroupMemberListResponse - Sets the local copy of CachedMembersByGroupIdResults
	 */
	void SetCachedMembersByGroupIdResults(FAccelByteModelsGetGroupMemberListResponse& AccelByteModelsGetGroupMemberListResponse);

	/**
	 * Sets cached group data after a query of group information
	 *
	 * @param AccelByteModelsGetMemberRequestsListResponse - Sets the local copy of CachedGroupInviteResults
	 */
	void SetCachedGroupInviteResults(FAccelByteModelsGetMemberRequestsListResponse& AccelByteModelsGetMemberRequestsListResponse);

	/**
	 * Sets cached group data after a query of group information
	 *
	 * @param AccelByteModelsGetMemberRequestsListResponse - Sets the local copy of CachedGroupRequests
	 */
	void SetCachedGroupRequests(FAccelByteModelsGetMemberRequestsListResponse& AccelByteModelsGetMemberRequestsListResponse);

	/**
	 * Adds a group member to the cached data
	 *
	 * @param RoleId - Role of the new member
	 * @param RoleId - User Id of the new member
	 */
	void AddCachedGroupMember(const FString& RoleId, const FString& UserId);

	/**
	* Removes a group member from the cached data
	*
	* @param RoleId - User Id of the member to be removed
	*/
	void RemoveCachedMember(FString& UserId);

	/**
	* Promotes a group member inside the cached data
	*
	* @param RoleId - User Id of the member to be promoted
	* @param NewMemberRoleIds - New role id for the newly promoted member
	*/
	void PromoteCachedMember(FString& UserId, TArray<FString>& NewMemberRoleIds);

	/**
	* Removes a group invite from the cached data
	*
	* @param RoleId - User Id of the invite to be removed
	*/
	void RemoveCachedInvites(FString& UserIdToRemove);

	/**
	* Removes a group request from the cached data
	*
	* @param RoleId - User Id of the request to be removed
	*/
	void RemoveCachedRequests(FString& UserIdToRemove);

	/**
	* Removes a group predefined rule from the cached data
	*
	* @param RoleId - Allowed action of the predefined rule to be removed
	*/
	void RemoveCachedPredefinedRule(EAccelByteAllowedAction& AllowedAction);

protected:

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineGroupsAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:

	/** Cached Group Information created by CreateGroup() */
	FAccelByteGroupsInfoPtr CachedCurrentGroup;

	/** Cached Results for FindGroups() */
	FAccelByteModelsGetGroupListResponse CachedGroupResults{};

	/** Cached Results for GetCachedFindGroupsByGroupIds() */
	FAccelByteModelsGetGroupListResponse CachedGroupListByGroupIdsResults{};

	/** Cached Results for QueryGroupRoster() */
	FAccelByteModelsGetGroupMemberListResponse CachedMembersByGroupIdResults{};

	/** Cached Results for QueryGroupInvites() */
	FAccelByteModelsGetMemberRequestsListResponse CachedGroupInviteResults{};

	/** Cached Results for QueryGroupRequests() */
	FAccelByteModelsGetMemberRequestsListResponse CachedGroupRequests{};

	/** Critical sections for thread safe operation for modifying CachedCurrentGroup */
	mutable FCriticalSection CachedCurrentGroupDataLock;

	/** Critical sections for thread safe operation for modifying CachedGroupResults */
	mutable FCriticalSection CachedGroupResultsDataLock;

	/** Critical sections for thread safe operation for modifying CachedGroupListByGroupIdsResults */
	mutable FCriticalSection CachedGroupListByGroupIdsResultsDataLock;

	/** Critical sections for thread safe operation for modifying CachedMembersByGroupIdResults */
	mutable FCriticalSection CachedMembersByGroupIdResultsDataLock;

	/** Critical sections for thread safe operation for modifying CachedGroupInviteResults */
	mutable FCriticalSection CachedGroupInviteResultsDataLock;

	/** Critical sections for thread safe operation for modifying CachedGroupRequests */
	mutable FCriticalSection CachedGroupRequestsDataLock;

};