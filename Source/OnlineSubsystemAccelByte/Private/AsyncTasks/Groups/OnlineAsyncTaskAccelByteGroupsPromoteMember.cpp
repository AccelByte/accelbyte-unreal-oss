// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsPromoteMember.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsPromoteMember::FOnlineAsyncTaskAccelByteGroupsPromoteMember(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& GroupMemberUserId,
	const FAccelByteGroupsInfo& InGroupInfo,
	const FString& MemberRoleId,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, MemberId(FUniqueNetIdAccelByteUser::CastChecked(GroupMemberUserId))
	, GroupInfo(InGroupInfo)
	, RoleId(MemberRoleId)
	, Delegate(InDelegate)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsPromoteMember::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("MemberId: %s"), *MemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGetUserGroupInfoResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsPromoteMember::OnPromoteMemberSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsPromoteMember::OnPromoteMemberError);

	FAccelByteModelsUserIdWrapper MemberUserId;
	MemberUserId.UserId = MemberId->GetAccelByteId();

	ApiClient->Group.AssignV2MemberRole(RoleId, GroupInfo.ABGroupInfo.GroupId, MemberUserId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsPromoteMember::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsPromoteMember::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote member, groups interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful == false)
		return;

	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote member, not in a group!"));
		return;
	}

	TArray<FAccelByteModelsGroupMember> CurrentGroupMembers;
	GroupsInterface->GetCurrentGroupMembers(CurrentGroupMembers);
	if (CurrentGroupMembers.Num() == 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote member, group member list is empty!"));
		return;
	}

	FAccelByteGroupsInfoPtr CurrentGroupData = GroupsInterface->GetCurrentGroupData();
	if (!CurrentGroupData.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote member, current group data is invalid!"));
		return;
	}

	for (int i = 0; i < CurrentGroupData->ABGroupInfo.GroupMembers.Num(); i++)
	{
		if (CurrentGroupData->ABGroupInfo.GroupMembers[i].UserId != UserId->GetAccelByteId())
			continue;

		for (int x = 0; x < AccelByteModelsGetUserGroupInfoResponse.MemberRoleId.Num(); x++)
		{
			if (!CurrentGroupData->ABGroupInfo.GroupMembers[i].MemberRoleId.Contains(AccelByteModelsGetUserGroupInfoResponse.MemberRoleId[x]))
			{
				CurrentGroupData->ABGroupInfo.GroupMembers[i].MemberRoleId.Add(AccelByteModelsGetUserGroupInfoResponse.MemberRoleId[x]);
			}
		}
		break;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsPromoteMember::OnPromoteMemberSuccess(const FAccelByteModelsGetUserGroupInfoResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);
	httpStatus = 200;// Success = 200
	AccelByteModelsGetUserGroupInfoResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsPromoteMember::OnPromoteMemberError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}