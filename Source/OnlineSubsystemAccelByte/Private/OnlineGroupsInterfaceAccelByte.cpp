#include "OnlineGroupsInterfaceAccelByte.h"

#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsCreateGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsJoinGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsLeaveGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsAcceptInvite.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDeclineInvite.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsCancelInvite.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsAcceptUser.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDeclineUser.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsInviteUser.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsRemoveMember.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsPromoteMember.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDemoteMember.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsQueryGroupInvites.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsDeleteGroup.h"


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
	: SenderUserId(InSenderUserId)
	, Namespace(InNamespace)
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

FString FAccelByteGroupsInfo::GetAdminRoleId()
{
	return ABAdminRoleId;
}

FString FAccelByteGroupsInfo::GetMemberRoleId()
{
	return ABMemberRoleId;
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

const FAccelByteModelsGroupInformation& FAccelByteGroupsInfo::GetABDisplayInfo() const
{
	return ABGroupInfo;
}
/**
* End FAccelByteGroupsInfo
*/

/**
* Begin FOnlineGroupsAccelByte
*/
bool FOnlineGroupsAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineGroupsAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineGroupsAccelByte>(Subsystem->GetGroupsInterface());
	return OutInterfaceInstance.IsValid();
}

void FOnlineGroupsAccelByte::CreateGroup(const FUniqueNetId& UserIdCreatingGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsCreateGroup>(AccelByteSubsystem, UserIdCreatingGroup, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::CreateGroup(const FUniqueNetId& ContextUserId, const FGroupDisplayInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using FGroupDisplayInfo as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::JoinGroup(const FUniqueNetId& UserIdJoiningGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsJoinGroup>(AccelByteSubsystem, UserIdJoiningGroup, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::JoinGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::LeaveGroup(const FUniqueNetId& UserIdLeavingGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsLeaveGroup>(AccelByteSubsystem, UserIdLeavingGroup, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::LeaveGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::AcceptInvite(const FUniqueNetId& UserIdAcceptedIntoGroup, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsAcceptInvite>(AccelByteSubsystem, UserIdAcceptedIntoGroup, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::AcceptInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::DeclineInvite(const FUniqueNetId& UserIdToDecline, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDeclineInvite>(AccelByteSubsystem, UserIdToDecline, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeclineInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::CancelInvite(const FUniqueNetId& AdminUserId, const FString& UserIdToCancel, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsCancelInvite>(AccelByteSubsystem, AdminUserId, UserIdToCancel, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::CancelInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::AcceptUser(const int32 AdminLocalUserNum, const FUniqueNetId& UserIdToAccept, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsAcceptUser>(AccelByteSubsystem, AdminLocalUserNum, UserIdToAccept, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::AcceptUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::DeclineUser(const int32 AdminLocalUserNum, const FUniqueNetId& UserIdToDecline, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDeclineUser>(AccelByteSubsystem, AdminLocalUserNum, UserIdToDecline, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeclineUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::InviteUser(const FUniqueNetId& InviterUserId, const FUniqueNetId& InvitedUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsInviteUser>(AccelByteSubsystem, InviterUserId, InvitedUserId, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::InviteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, bool bAllowBlocked, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::RemoveUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToKick, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsRemoveMember>(AccelByteSubsystem, AdminLocalUserNum, MemberUserIdToKick, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::RemoveUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::PromoteUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToPromote, const FAccelByteGroupsInfo& InGroupInfo, const FString& MemberRoleId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsPromoteMember>(AccelByteSubsystem, AdminLocalUserNum, MemberUserIdToPromote, InGroupInfo, MemberRoleId, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::PromoteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::DemoteUser(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserIdToDemote, const FAccelByteGroupsInfo& InGroupInfo, const FString& MemberRoleId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDemoteMember>(AccelByteSubsystem, AdminLocalUserNum, MemberUserIdToDemote, InGroupInfo, MemberRoleId, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DemoteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::QueryGroupInvites(const FUniqueNetId& ContextUserId, const FAccelByteModelsLimitOffsetRequest& AccelByteModelsLimitOffsetRequest, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites>(AccelByteSubsystem, ContextUserId, AccelByteModelsLimitOffsetRequest, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::QueryGroupInvites(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



void FOnlineGroupsAccelByte::DeleteGroup(const int32 AdminLocalUserNum, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (!VerifyGroupInfo(InGroupInfo))
		return;

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsDeleteGroup>(AccelByteSubsystem, AdminLocalUserNum, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::DeleteGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}



bool FOnlineGroupsAccelByte::IsGroupValid() const
{
	if (CurrentGroup.IsValid())
		return true;

	return false;
}

void FOnlineGroupsAccelByte::GetCurrentGroupMembers(TArray<FAccelByteModelsGroupMember>& GroupMembers) const
{
	if (IsGroupValid())
	{
		GroupMembers = CurrentGroup->ABGroupInfo.GroupMembers;
	}
}

FAccelByteGroupsInfoPtr FOnlineGroupsAccelByte::GetCurrentGroupData()
{
	return CurrentGroup;
}

void FOnlineGroupsAccelByte::SetCurrentGroupData(const FAccelByteGroupsInfoRef& InGroupInfo)
{
	CurrentGroup = InGroupInfo;
}

void FOnlineGroupsAccelByte::DeleteLocalGroupData()
{
	if (CurrentGroup.IsValid())
		CurrentGroup.Reset();
}

bool FOnlineGroupsAccelByte::VerifyGroupInfo(const FAccelByteGroupsInfo& InGroupInfo)
{
	if (InGroupInfo.ABGroupInfo.ConfigurationCode.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupName.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupRegion.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupType == EAccelByteGroupType::NONE)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Missing mandatory group details. ConfigurationCode, GroupName, GroupRegion, & GroupType must be filled."));
		return false;
	}

	return true;
}
/**
* End FOnlineGroupsAccelByte
*/