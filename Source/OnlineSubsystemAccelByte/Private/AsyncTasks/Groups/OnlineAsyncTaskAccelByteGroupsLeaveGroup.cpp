// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsLeaveGroup.h"

FOnlineAsyncTaskAccelByteGroupsLeaveGroup::FOnlineAsyncTaskAccelByteGroupsLeaveGroup(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InContextUserId,
	const FAccelByteGroupsInfo& InGroupInfo,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, GroupInfo(InGroupInfo)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InContextUserId);
}

void FOnlineAsyncTaskAccelByteGroupsLeaveGroup::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsMemberRequestGroupResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsLeaveGroup::OnLeaveGroupSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsLeaveGroup::OnLeaveGroupError);

	ApiClient->Group.LeaveV2Group(GroupInfo.ABGroupInfo.GroupId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsLeaveGroup::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to leave group, user ID is not valid!"));
		return;
	}

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsLeaveGroup::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to leave group, groups interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful == false)
		return;

	// Cannot leave group if user is not in a group
	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to leave group, not in a group!"));
		return;
	}
		
	TArray<FAccelByteModelsGroupMember> CurrentGroupMembers;
	GroupsInterface->GetCurrentGroupMembers(CurrentGroupMembers);
	if (CurrentGroupMembers.Num() == 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to leave group, group member list is empty!"));
		return;
	}

	FAccelByteGroupsInfoPtr CurrentGroupData = GroupsInterface->GetCurrentGroupData();
	if (!CurrentGroupData.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to leave group, current group data is invalid!"));
		return;
	}

	for (int i = 0; i < CurrentGroupData->ABGroupInfo.GroupMembers.Num(); i++)
	{
		if (CurrentGroupData->ABGroupInfo.GroupMembers[i].UserId == AccelByteModelsMemberRequestGroupResponse.UserId)
		{
			CurrentGroupData->ABGroupInfo.GroupMembers.RemoveAt(i);
			break;
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsLeaveGroup::OnLeaveGroupSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);

	// Success = 200
	httpStatus = 200;

	AccelByteModelsMemberRequestGroupResponse = Result;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsLeaveGroup::OnLeaveGroupError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	// Failure = not 200
	httpStatus = 0;
}
