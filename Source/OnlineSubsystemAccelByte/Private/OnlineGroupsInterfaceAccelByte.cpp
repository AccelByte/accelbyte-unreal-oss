#include "OnlineGroupsInterfaceAccelByte.h"

#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsCreateGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsFindGroups.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsQueryGroupInfo.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsJoinGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsLeaveGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsCancelJoinRequest.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsAcceptInvite.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDeclineInvite.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsGetGroupMembersByGroupId.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsCancelInvite.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsUpdateGroupInfo.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsAcceptUser.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDeclineUser.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsInviteUser.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsRemoveMember.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsPromoteMember.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDemoteMember.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsQueryGroupInvites.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDeleteGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDeletePredefinedRule.h"


/**
* Begin FAccelByteGroupsInfo
*/
/** Constructor */
FAccelByteGroupsInfo::FAccelByteGroupsInfo(
	const FUniqueNetIdRef& InSenderUserId,
	const FString InNamespace,
	const FUniqueNetId& InOwnerId,
	const FDateTime InTimeCreated,
	const FDateTime InTimeLastUpdated,
	const FString InABAdminRoleId,
	const FString InABMemberRoleId,
	const FAccelByteModelsGroupInformation InABGroupInfo)
	: Namespace(InNamespace)
	, SenderUserId(InSenderUserId)
	, TimeCreated(InTimeCreated)
	, TimeLastUpdated(InTimeLastUpdated)
	, ABGroupInfo(InABGroupInfo)
	, ABAdminRoleId(InABAdminRoleId)
	, ABMemberRoleId(InABMemberRoleId)
{
	OwnerId = FUniqueNetIdAccelByteUser::CastChecked(InOwnerId);
}

FAccelByteGroupsInfoRef FAccelByteGroupsInfo::Create(const FUniqueNetIdRef& InSenderUserId,
	const FString InNamespace,
	const FUniqueNetId& InOwnerId,
	const FAccelByteModelsGroupInformation InABGroupInfo,
	const FString AdminRoleId,
	const FString MemberRoleId)
{
	FDateTime InTimeCreated = FDateTime::Now();
	FDateTime InTimeLastUpdated = FDateTime::Now();

	return MakeShared<FAccelByteGroupsInfo>(
		InSenderUserId,
		InNamespace,
		InOwnerId,
		InTimeCreated,
		InTimeLastUpdated,
		AdminRoleId,
		MemberRoleId,
		InABGroupInfo);
}

FUniqueNetIdRef FAccelByteGroupsInfo::GetGroupId() const
{
	return SenderUserId;
}

const FString& FAccelByteGroupsInfo::GetNamespace() const
{
	return Namespace;
}

FUniqueNetIdRef FAccelByteGroupsInfo::GetOwner() const
{
	return OwnerId->AsShared();
}

const FGroupDisplayInfo& FAccelByteGroupsInfo::GetDisplayInfo() const
{
	FGroupDisplayInfo* GroupDisplayInfo = nullptr;
	return *GroupDisplayInfo;
}

uint32 FAccelByteGroupsInfo::GetSize() const
{
	return ABGroupInfo.GroupMembers.Num();
}

const FDateTime& FAccelByteGroupsInfo::GetCreatedAt() const
{
	return TimeCreated;
}

const FDateTime& FAccelByteGroupsInfo::GetLastUpdated() const
{
	return TimeLastUpdated;
}

void FAccelByteGroupsInfo::SetCachedABGroupInfo(const FAccelByteModelsGroupInformation& GroupInfo)
{
	ABGroupInfo = GroupInfo;
}

void FAccelByteGroupsInfo::SetCachedABAdminRoleId(const FString& AdminRoleId)
{
	ABAdminRoleId = AdminRoleId;
}

void FAccelByteGroupsInfo::SetCachedABMemberRoleId(const FString& MemberRoleId)
{
	ABMemberRoleId = MemberRoleId;
}
/**
* End FAccelByteGroupsInfo
*/

/**
* Begin FOnlineGroupsAccelByte
*/



FOnlineGroupsAccelByte::FOnlineGroupsAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{}

bool FOnlineGroupsAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineGroupsAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineGroupsAccelByte>(Subsystem->GetGroupsInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineGroupsAccelByte::GetFromSubsystem(const FOnlineSubsystemAccelByte* Subsystem, TSharedPtr<FOnlineGroupsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	if (Subsystem == nullptr)
	{
		return false;
	}
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineGroupsAccelByte>(Subsystem->GetGroupsInterface());
	return OutInterfaceInstance.IsValid();
}

void FOnlineGroupsAccelByte::CreateGroup(const FUniqueNetId& UserIdCreatingGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserIdCreatingGroup: %s"), *UserIdCreatingGroup.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsCreateGroup>(AccelByteSubsystemPtr.Get(), UserIdCreatingGroup, InGroupInfo, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::CreateGroup(const FUniqueNetId& ContextUserId, const FGroupDisplayInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using FGroupDisplayInfo as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::FindGroups(const FUniqueNetId& SearchingUserId, const FAccelByteModelsGetGroupListRequest& RequestedContent, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingUserId: %s"), *SearchingUserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsFindGroups>(AccelByteSubsystemPtr.Get(), SearchingUserId, RequestedContent, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::FindGroups(const FUniqueNetId& ContextUserId, const FGroupSearchOptions& SearchOptions, const FOnFindGroupsCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using FGroupSearchOptions as its fields are not supported! Please supply an instance of FAccelByteModelsGetGroupListRequest instead!"));
}

FAccelByteModelsGetGroupListResponse FOnlineGroupsAccelByte::GetCachedFindGroupsRoster()
{
	return CachedGroupResults;
}

FAccelByteModelsGetGroupListResponse FOnlineGroupsAccelByte::FOnlineGroupsAccelByte::GetCachedFindGroupsByGroupIds()
{
	return CachedGroupListByGroupIdsResults;
}

void FOnlineGroupsAccelByte::FindGroupsByGroupIds(const FUniqueNetId& SearchingUserId, const TArray<FString> GroupIds, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("SearchingUserId: %s"), *SearchingUserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds>(AccelByteSubsystemPtr.Get(), SearchingUserId, GroupIds, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::QueryGroupInfo(const FUniqueNetId& CurrentUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("CurrentUserId: %s"), *CurrentUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsQueryGroupInfo>(AccelByteSubsystemPtr.Get(), CurrentUserId, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::QueryGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using FGroupSearchOptions as its fields are not supported! Please supply an instance of FAccelByteModelsGetGroupListRequest instead!"));
}

void FOnlineGroupsAccelByte::QueryGroupNameExist(const FUniqueNetId& ContextUserId, const FString& GroupName, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use FindGroups()!"));
}

FAccelByteGroupsInfoPtr FOnlineGroupsAccelByte::GetCachedGroupInfo()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return CachedCurrentGroup;
}

TSharedPtr<const IGroupInfo> FOnlineGroupsAccelByte::GetCachedGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use GetCachedGroupInfo()!"));
	return nullptr;
}

void FOnlineGroupsAccelByte::SetCachedGroupInfo(const FAccelByteGroupsInfoRef& InGroupInfo)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedCurrentGroupDataLock);
	CachedCurrentGroup = InGroupInfo;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeleteCachedGroupInfo()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (CachedCurrentGroup.IsValid())
		CachedCurrentGroup.Reset();

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::JoinGroup(const FUniqueNetId& UserIdJoiningGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserIdJoiningGroup: %s"), *UserIdJoiningGroup.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsJoinGroup>(AccelByteSubsystemPtr.Get(), UserIdJoiningGroup, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::JoinGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::LeaveGroup(const FUniqueNetId& UserIdLeavingGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserIdLeavingGroup: %s"), *UserIdLeavingGroup.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsLeaveGroup>(AccelByteSubsystemPtr.Get(), UserIdLeavingGroup, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::LeaveGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::CancelRequest(const FUniqueNetId& UserIdCancelingRequest, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserIdCancelingRequest: %s"), *UserIdCancelingRequest.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsCancelJoinRequest>(AccelByteSubsystemPtr.Get(), UserIdCancelingRequest, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::CancelRequest(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"))
}

void FOnlineGroupsAccelByte::AcceptInvite(const FUniqueNetId& UserIdAcceptedIntoGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserIdAcceptedIntoGroup: %s"), *UserIdAcceptedIntoGroup.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsAcceptInvite>(AccelByteSubsystemPtr.Get(), UserIdAcceptedIntoGroup, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::AcceptInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::DeclineInvite(const FUniqueNetId& UserIdToDecline, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserIdToDecline: %s"), *UserIdToDecline.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDeclineInvite>(AccelByteSubsystemPtr.Get(), UserIdToDecline, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeclineInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::QueryGroupRoster(const FUniqueNetId& RequestingUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsGetGroupMembersListByGroupIdRequest& RequestedContent, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("RequestingUserId: %s"), *RequestingUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsGetGroupMembersByGroupId>(AccelByteSubsystemPtr.Get(), RequestingUserId, InGroupInfo.ABGroupInfo.GroupId, RequestedContent, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::QueryGroupRoster(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

FAccelByteModelsGetGroupMemberListResponse FOnlineGroupsAccelByte::GetCachedGroupRoster()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return CachedMembersByGroupIdResults;
}

TSharedPtr<const IGroupRoster> FOnlineGroupsAccelByte::GetCachedGroupRoster(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use GetCachedGroupRoster()!"));
	return nullptr;
}

void FOnlineGroupsAccelByte::QueryUserMembership(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use QueryConfigMembership(TArray<FAccelByteModelsGroupMember>& GroupMembers)!"));
}

void FOnlineGroupsAccelByte::GetCachedUserMembership(TArray<FAccelByteModelsGroupMember>& GroupRoster)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsGroupValid() == false)
		return;

	FScopeLock ScopeLock(&CachedCurrentGroupDataLock);
	GroupRoster = CachedCurrentGroup->ABGroupInfo.GroupMembers;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

TSharedPtr<const IUserMembership> FOnlineGroupsAccelByte::GetCachedUserMembership(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId) 
{ 
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use GetCachedUserMembership()!"));
	return nullptr;
}

void FOnlineGroupsAccelByte::QueryOutgoingApplications(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("QueryOutgoingApplications is not supported by AccelByte!"));
};

TSharedPtr<const IApplications> FOnlineGroupsAccelByte::GetCachedApplications(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId) 
{ 
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use GetCachedGroupRequests()!"));
	return nullptr;
}

void FOnlineGroupsAccelByte::QueryOutgoingInvitations(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted) 
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use QueryGroupInvites()!"));
}

void FOnlineGroupsAccelByte::QueryIncomingInvitations(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("QueryIncomingInvitations is not supported by AccelByte!"));
}

FAccelByteModelsGetMemberRequestsListResponse FOnlineGroupsAccelByte::GetCachedInvitations()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return CachedGroupInviteResults;
}

TSharedPtr<const IInvitations> FOnlineGroupsAccelByte::GetCachedInvitations(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId) 
{ 
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use GetCachedInvitations()!"));
	return nullptr;
}

void FOnlineGroupsAccelByte::CancelInvite(const FUniqueNetId& AdminUserId, const FString& UserIdToCancel, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminUserId: %s"), *AdminUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsCancelInvite>(AccelByteSubsystemPtr.Get(), AdminUserId, UserIdToCancel, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::CancelInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::UpdateGroupInfo(const int32& AdminLocalUserNum, const FUniqueNetId& AdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsGroupUpdatable& RequestContent, const bool CompletelyReplace, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, AdminUserId: %s"), AdminLocalUserNum, *AdminUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, AdminUserId, InGroupInfo.ABGroupInfo.GroupId, CompletelyReplace, RequestContent, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::UpdateGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FGroupDisplayInfo& GroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::AcceptUser(const int32 AdminLocalUserNum, const FUniqueNetId& UserIdToAccept, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, UserIdToAccept: %s"), AdminLocalUserNum, *UserIdToAccept.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsAcceptUser>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, UserIdToAccept, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::AcceptUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::DeclineUser(const int32 AdminLocalUserNum, const FUniqueNetId& UserIdToDecline, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, UserIdToDecline: %s"), AdminLocalUserNum, *UserIdToDecline.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDeclineUser>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, UserIdToDecline, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeclineUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::InviteUser(const FUniqueNetId& InviterUserId, const FUniqueNetId& InvitedUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("InviterUserId: %s, InvitedUserId: %s"), *InviterUserId.ToDebugString(), *InvitedUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsInviteUser>(AccelByteSubsystemPtr.Get(), InviterUserId, InvitedUserId, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::InviteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, bool bAllowBlocked, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::RemoveUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToKick, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, MemberUserIdToKick: %s"), AdminLocalUserNum, *MemberUserIdToKick.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsRemoveMember>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, MemberUserIdToKick, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::RemoveUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::PromoteUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToPromote, const FAccelByteGroupsInfo& InGroupInfo, const FString& MemberRoleId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, MemberUserIdToPromote: %s"), AdminLocalUserNum, *MemberUserIdToPromote.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsPromoteMember>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, MemberUserIdToPromote, InGroupInfo.ABGroupInfo.GroupId, MemberRoleId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::PromoteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::DemoteUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToDemote, const FAccelByteGroupsInfo& InGroupInfo, const FString& MemberRoleId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, MemberUserIdToDemote: %s"), AdminLocalUserNum, *MemberUserIdToDemote.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDemoteMember>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, MemberUserIdToDemote, InGroupInfo.ABGroupInfo.GroupId, MemberRoleId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DemoteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::QueryGroupInvites(const FUniqueNetId& ContextUserId, const FAccelByteModelsLimitOffsetRequest& AccelByteModelsLimitOffsetRequest, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("ContextUserId: %s"), *ContextUserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites>(AccelByteSubsystemPtr.Get(), ContextUserId, AccelByteModelsLimitOffsetRequest, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::QueryGroupInvites(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

FAccelByteModelsGetMemberRequestsListResponse FOnlineGroupsAccelByte::GetCachedGroupInvites()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return CachedGroupInviteResults;
}

TSharedPtr<const IGroupInvites> FOnlineGroupsAccelByte::GetCachedGroupInvites(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use GetCachedGroupInvites()!"));
	return nullptr;
}

void FOnlineGroupsAccelByte::QueryGroupRequests(const FUniqueNetId& ContextUserId, const FAccelByteModelsLimitOffsetRequest& AccelByteModelsLimitOffsetRequest, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("ContextUserId: %s"), *ContextUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests>(AccelByteSubsystemPtr.Get(), ContextUserId, AccelByteModelsLimitOffsetRequest, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::QueryGroupRequests(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

FAccelByteModelsGetMemberRequestsListResponse FOnlineGroupsAccelByte::GetCachedGroupRequests()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return CachedGroupRequests;
}

TSharedPtr<const IGroupRequests> FOnlineGroupsAccelByte::GetCachedGroupRequests(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use GetCachedGroupRequests()!"));
	return nullptr;
}

void FOnlineGroupsAccelByte::QueryIncomingApplications(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{ 
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use QueryGroupRequests()!"));
}

int FOnlineGroupsAccelByte::QueryConfigHeadcount()
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (IsGroupValid() == false)
		return 0;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return CachedCurrentGroup->ABGroupInfo.GroupMaxMember;
}

void FOnlineGroupsAccelByte::QueryConfigHeadcount(const FUniqueNetId& ContextUserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Deprecated! Please use QueryConfigHeadcount()!"));
}

void FOnlineGroupsAccelByte::QueryConfigMembership(const FUniqueNetId& ContextUserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("QueryConfigMembership is not supported by AccelByte!"));
}

void FOnlineGroupsAccelByte::DeleteGroup(const int32 AdminLocalUserNum, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d"), AdminLocalUserNum);

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDeleteGroup>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, InGroupInfo.ABGroupInfo.GroupId, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeleteGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));
	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::UpdateGroupCustomAttributes(const int32& AdminLocalUserNum, const FUniqueNetId& AdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsUpdateGroupCustomAttributesRequest& RequestContent, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, AdminUserId: %s"), AdminLocalUserNum, *AdminUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, AdminUserId, InGroupInfo.ABGroupInfo.GroupId, RequestContent, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::UpdateGroupCustomRule(const int32& AdminLocalUserNum, const FUniqueNetId& AdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const FAccelByteModelsUpdateCustomRulesRequest& RequestContent, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, AdminUserId: %s"), AdminLocalUserNum, *AdminUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, AdminUserId, InGroupInfo.ABGroupInfo.GroupId, RequestContent, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::UpdatePredefinedRule(const int32& AdminLocalUserNum, const FUniqueNetId& AdminUserId, const FAccelByteGroupsInfo& InGroupInfo, const EAccelByteAllowedAction& InAllowedAction, const FAccelByteModelsUpdateGroupPredefinedRuleRequest& RequestContent, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d, AdminUserId: %s"), AdminLocalUserNum, *AdminUserId.ToDebugString());

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, AdminUserId, InGroupInfo.ABGroupInfo.GroupId, InAllowedAction, RequestContent, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeletePredefinedRule(const int32& AdminLocalUserNum, const FAccelByteGroupsInfo& InGroupInfo, const EAccelByteAllowedAction& AllowedAction, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("AdminLocalUserNum: %d"), AdminLocalUserNum);

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDeletePredefinedRule>(AccelByteSubsystemPtr.Get(), AdminLocalUserNum, InGroupInfo.ABGroupInfo.GroupId, AllowedAction, OnCompleted);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

bool FOnlineGroupsAccelByte::IsGroupValid() const
{
	if (CachedCurrentGroup.IsValid())
		return true;

	return false;
}

bool FOnlineGroupsAccelByte::VerifyGroupInfo(const FAccelByteGroupsInfo& InGroupInfo)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FAccelByteModelsGroupInformation GroupInfo = InGroupInfo.ABGroupInfo;
	if (GroupInfo.ConfigurationCode.IsEmpty() ||
		GroupInfo.GroupName.IsEmpty() ||
		GroupInfo.GroupRegion.IsEmpty() ||
		GroupInfo.GroupType == EAccelByteGroupType::NONE)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Missing mandatory group details. ConfigurationCode, GroupName, GroupRegion, & GroupType must be filled."));
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
		return false;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

void FOnlineGroupsAccelByte::SetCachedGroupResults(FAccelByteModelsGetGroupListResponse& AccelByteModelsGetGroupListResponse)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedGroupResultsDataLock);
	CachedGroupResults = AccelByteModelsGetGroupListResponse;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::SetCachedGroupListByGroupIdsResults(FAccelByteModelsGetGroupListResponse& AccelByteModelsGetGroupListResponse)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedGroupListByGroupIdsResultsDataLock);
	CachedGroupListByGroupIdsResults = AccelByteModelsGetGroupListResponse;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::SetCachedMembersByGroupIdResults(FAccelByteModelsGetGroupMemberListResponse& AccelByteModelsGetGroupMemberListResponse)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedMembersByGroupIdResultsDataLock);
	CachedMembersByGroupIdResults = AccelByteModelsGetGroupMemberListResponse;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::SetCachedGroupInviteResults(FAccelByteModelsGetMemberRequestsListResponse& AccelByteModelsGetMemberRequestsListResponse)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedGroupInviteResultsDataLock);
	CachedGroupInviteResults = AccelByteModelsGetMemberRequestsListResponse;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::SetCachedGroupRequests(FAccelByteModelsGetMemberRequestsListResponse& AccelByteModelsGetMemberRequestsListResponse)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedGroupRequestsDataLock);
	CachedGroupRequests = AccelByteModelsGetMemberRequestsListResponse;

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::AddCachedGroupMember(const FString& RoleId, const FString& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (CachedCurrentGroup.IsValid() == false)
		return;
	
	FScopeLock ScopeLock(&CachedCurrentGroupDataLock);
	
	FAccelByteModelsGroupInformation GroupInfo = CachedCurrentGroup->ABGroupInfo;
	
	FAccelByteModelsGroupMember AccelByteModelsGroupMember;
	AccelByteModelsGroupMember.MemberRoleId.Add(RoleId);
	AccelByteModelsGroupMember.UserId = UserId;
	
	GroupInfo.GroupMembers.Add(AccelByteModelsGroupMember);
	CachedCurrentGroup->SetCachedABGroupInfo(GroupInfo);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::RemoveCachedMember(FString& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (CachedCurrentGroup.IsValid() == false)
		return;
	
	FScopeLock ScopeLock(&CachedCurrentGroupDataLock);
	int32 RemoveResult = CachedCurrentGroup->ABGroupInfo.GroupMembers.RemoveAll([UserId](const FAccelByteModelsGroupMember& Member)
	{
		return Member.UserId == UserId;
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::PromoteCachedMember(FString& UserId, TArray<FString>& NewMemberRoleIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (CachedCurrentGroup.IsValid() == false)
		return;
	
	FScopeLock ScopeLock(&CachedCurrentGroupDataLock);
	FAccelByteModelsGroupMember* FoundMember = CachedCurrentGroup->ABGroupInfo.GroupMembers.FindByPredicate([UserId](const FAccelByteModelsGroupMember& Member)
	{
		return Member.UserId == UserId;
	});
	
	if (FoundMember == nullptr)
		return;
	
	for (int i = 0; i < NewMemberRoleIds.Num(); i++)
	{
		if (!FoundMember->MemberRoleId.Contains(NewMemberRoleIds[i]))
		{
			FoundMember->MemberRoleId.Add(NewMemberRoleIds[i]);
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::RemoveCachedInvites(FString& UserIdToRemove)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedGroupInviteResultsDataLock);
	int32 RemoveResult = CachedGroupInviteResults.Data.RemoveAll([UserIdToRemove](const FAccelByteModelsMemberRequestResponse& MemberRequest)
	{
		return MemberRequest.UserId == UserIdToRemove;
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::RemoveCachedRequests(FString& UserIdToRemove)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FScopeLock ScopeLock(&CachedGroupRequestsDataLock);
	int32 RemoveResult = CachedGroupRequests.Data.RemoveAll([UserIdToRemove](const FAccelByteModelsMemberRequestResponse& MemberRequest)
	{
		return MemberRequest.UserId == UserIdToRemove;
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::RemoveCachedPredefinedRule(EAccelByteAllowedAction& AllowedAction)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (CachedCurrentGroup.IsValid() == false)
		return;

	FScopeLock ScopeLock(&CachedCurrentGroupDataLock);

	int32 RemoveResult = CachedCurrentGroup->ABGroupInfo.GroupRules.GroupPredefinedRules.RemoveAll([AllowedAction](FAccelByteModelsRules& Rule)
	{
		return Rule.AllowedAction == AllowedAction;
	});

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}
/**
* End FOnlineGroupsAccelByte
*/