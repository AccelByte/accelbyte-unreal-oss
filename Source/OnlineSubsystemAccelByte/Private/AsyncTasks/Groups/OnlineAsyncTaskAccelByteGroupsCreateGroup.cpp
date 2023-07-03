// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsCreateGroup.h"

FOnlineAsyncTaskAccelByteGroupsCreateGroup::FOnlineAsyncTaskAccelByteGroupsCreateGroup(
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

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	FAccelByteModelsCreateGroupRequest NewGroupRequest;
	NewGroupRequest.ConfigurationCode = GroupInfo.ABGroupInfo.ConfigurationCode;
	NewGroupRequest.CustomAttributes = GroupInfo.ABGroupInfo.CustomAttributes;
	NewGroupRequest.GroupDescription = GroupInfo.ABGroupInfo.GroupDescription;
	NewGroupRequest.GroupIcon = GroupInfo.ABGroupInfo.GroupIcon;
	NewGroupRequest.GroupMaxMember = GroupInfo.ABGroupInfo.GroupMaxMember;
	NewGroupRequest.GroupName = GroupInfo.ABGroupInfo.GroupName;
	NewGroupRequest.GroupRegion = GroupInfo.ABGroupInfo.GroupRegion;
	NewGroupRequest.GroupRules = GroupInfo.ABGroupInfo.GroupRules;
	NewGroupRequest.GroupType = GroupInfo.ABGroupInfo.GroupType;

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupError);

	ApiClient->Group.CreateV2Group(NewGroupRequest, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create group as our user ID is not valid!"));
		return;
	}

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create group as our groups interface instance is not valid!"));
		return;
	}

	if (GroupsInterface->IsGroupValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create group as we are already in a group!"));
		return;
	}

	// Bail if we don't have any user/leader data
	if (GroupInfo.GetABDisplayInfo().GroupMembers.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create group as no group user information was received!"));
		return;
	}

	FAccelByteModelsGroupInformation ABGroupInfo;
	ABGroupInfo.ConfigurationCode = GroupInfo.ABGroupInfo.ConfigurationCode;
	ABGroupInfo.CustomAttributes = GroupInfo.ABGroupInfo.CustomAttributes;
	ABGroupInfo.GroupDescription = GroupInfo.ABGroupInfo.GroupDescription;
	ABGroupInfo.GroupIcon = GroupInfo.ABGroupInfo.GroupIcon;
	ABGroupInfo.GroupId = GroupInfo.ABGroupInfo.GroupId;
	ABGroupInfo.GroupMaxMember = GroupInfo.ABGroupInfo.GroupMaxMember;

	// Add leader to group
	ABGroupInfo.GroupMembers.Add(GroupInfo.GetABDisplayInfo().GroupMembers[0]);
	ABGroupInfo.GroupName = GroupInfo.ABGroupInfo.GroupName;
	ABGroupInfo.GroupRegion = GroupInfo.ABGroupInfo.GroupRegion;
	ABGroupInfo.GroupRules = GroupInfo.ABGroupInfo.GroupRules;
	ABGroupInfo.GroupType = GroupInfo.ABGroupInfo.GroupType;

	const FAccelByteGroupsInfoRef AccelByteGroupsInfo = FAccelByteGroupsInfo::Create(
		GroupInfo.SenderUserId,
		GroupInfo.Namespace,
		GroupInfo.OwnerId->AsShared().Get(),
		ABGroupInfo);

	GroupsInterface->SetCurrentGroupData(AccelByteGroupsInfo);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupSuccess(const FAccelByteModelsGroupInformation& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);

	// Success = 201
	httpStatus = 201;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	httpStatus = ErrorCode;
}