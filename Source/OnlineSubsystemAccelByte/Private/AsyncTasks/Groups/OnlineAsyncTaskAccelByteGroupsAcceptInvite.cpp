// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsAcceptInvite.h"

FOnlineAsyncTaskAccelByteGroupsAcceptInvite::FOnlineAsyncTaskAccelByteGroupsAcceptInvite(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& GroupMemberUserId,
	const FAccelByteGroupsInfo& InGroupInfo,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, MemberId(FUniqueNetIdAccelByteUser::CastChecked(GroupMemberUserId))
	, GroupInfo(InGroupInfo)
	, Delegate(InDelegate)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsMemberRequestGroupResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteError);

	ApiClient->Group.AcceptV2GroupJoinRequest(MemberId->GetAccelByteId(), GroupInfo.ABGroupInfo.GroupId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to accept invite, user ID is not valid!"));
		return;
	}

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to accept invite, groups interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful == false)
		return;

	// Cannot accept invite if user is not in a group
	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to accept invite, not in a group!"));
		return;
	}
		
	TArray<FAccelByteModelsGroupMember> CurrentGroupMembers;
	GroupsInterface->GetCurrentGroupMembers(CurrentGroupMembers);
	if (CurrentGroupMembers.Num() == 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to accept invite, group member list is empty!"));
		return;
	}

	FAccelByteGroupsInfoPtr CurrentGroupData = GroupsInterface->GetCurrentGroupData();
	if (!CurrentGroupData.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to accept invite, current group data is invalid!"));
		return;
	}

	bool found = false;
	for (int i = 0; i < CurrentGroupData->ABGroupInfo.GroupMembers.Num(); i++)
	{
		if (CurrentGroupData->ABGroupInfo.GroupMembers[i].UserId == AccelByteModelsMemberRequestGroupResponse.UserId)
		{
			found = true;
			break;
		}
	}

	// Setup group member
	FAccelByteModelsGroupMember AccelByteModelsGroupMember;
	AccelByteModelsGroupMember.MemberRoleId.Add(CurrentGroupData->ABMemberRoleId);
	AccelByteModelsGroupMember.UserId = AccelByteModelsMemberRequestGroupResponse.UserId;

	if (found)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to accept invite, player already exists in group!"));
		return;
	}
	
	// Add the player to the current group
	CurrentGroupData->ABGroupInfo.GroupMembers.Add(AccelByteModelsGroupMember);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);

	// Success = 200
	httpStatus = 200;

	AccelByteModelsMemberRequestGroupResponse = Result;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	// Failure = not 200
	httpStatus = 0;
}
