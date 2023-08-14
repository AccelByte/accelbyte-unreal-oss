#include "OnlineGroupsInterfaceAccelByte.h"

#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsCreateGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsJoinGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsLeaveGroup.h"
#include "AsyncTasks/Groups/OnlineAsyncTaskAccelByteGroupsAcceptInvite.h"

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

void FOnlineGroupsAccelByte::CreateGroup(const FUniqueNetId& ContextUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (InGroupInfo.ABGroupInfo.ConfigurationCode.IsEmpty() || 
		InGroupInfo.ABGroupInfo.GroupName.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupRegion.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupType == EAccelByteGroupType::NONE)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Missing mandatory group details. ConfigurationCode, GroupName, GroupRegion, & GroupType must be filled."));
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsCreateGroup>(AccelByteSubsystem, ContextUserId, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::CreateGroup(const FUniqueNetId& ContextUserId, const FGroupDisplayInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, as none of the fields in FGroupDisplayInfo are supported by the backend. So a call to this method would be redundant

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using FGroupDisplayInfo as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::JoinGroup(const FUniqueNetId& ContextUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (InGroupInfo.ABGroupInfo.ConfigurationCode.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupName.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupRegion.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupType == EAccelByteGroupType::NONE)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Missing mandatory group details. ConfigurationCode, GroupName, GroupRegion, & GroupType must be filled."));
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsJoinGroup>(AccelByteSubsystem, ContextUserId, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::JoinGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::LeaveGroup(const FUniqueNetId& ContextUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	if (InGroupInfo.ABGroupInfo.ConfigurationCode.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupName.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupRegion.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupType == EAccelByteGroupType::NONE)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Missing mandatory group details. ConfigurationCode, GroupName, GroupRegion, & GroupType must be filled."));
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsLeaveGroup>(AccelByteSubsystem, ContextUserId, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::LeaveGroup(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, GroupId is not enough information to identify a user, group configuration, and group roles

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure group using GroupId as its fields are not supported! Please supply an instance of FAccelByteGroupsInfo instead!"));
}

void FOnlineGroupsAccelByte::AcceptInvite(const int32 AdminLocalUserNum, const FUniqueNetId& MemberUserId, const FAccelByteGroupsInfo& InGroupInfo, const FOnGroupsRequestCompleted& OnCompleted)
{
	if (InGroupInfo.ABGroupInfo.ConfigurationCode.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupName.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupRegion.IsEmpty() ||
		InGroupInfo.ABGroupInfo.GroupType == EAccelByteGroupType::NONE)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Missing mandatory group details. ConfigurationCode, GroupName, GroupRegion, & GroupType must be filled."));
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGroupsAcceptInvite>(AccelByteSubsystem, AdminLocalUserNum, MemberUserId, InGroupInfo, OnCompleted);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineGroupsAccelByte::AcceptInvite(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FOnGroupsRequestCompleted& OnCompleted)
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

void FOnlineGroupsAccelByte::SetCurrentGroupData(const FAccelByteGroupsInfoRef& InGroupInfo)
{
	CurrentGroup = InGroupInfo;
}

void FOnlineGroupsAccelByte::DeleteLocalGroupData()
{
	if (CurrentGroup.IsValid())
		CurrentGroup.Reset();
}

FAccelByteGroupsInfoPtr FOnlineGroupsAccelByte::GetCurrentGroupData()
{
	return CurrentGroup;
}
/**
* End FOnlineGroupsAccelByte
*/