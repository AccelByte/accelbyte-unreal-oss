// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsQueryGroupInvites.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& ContextUserId,
	const FAccelByteModelsLimitOffsetRequest& InRequestContent,
	const FAccelByteGroupsInfo& InGroupInfo,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RequestContent(InRequestContent)
	, GroupInfo(InGroupInfo)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(ContextUserId);
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGetMemberRequestsListResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::OnQueryGroupInvitesSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::OnQueryGroupInvitesError);

	ApiClient->Group.GetGroupInvitationRequests(RequestContent, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to QueryGroupInvites, groups interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful == false)
		return;

	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to QueryGroupInvites, not in a group!"));
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

	CurrentGroupData->QueryGroupInviteReults = AccelByteModelsGetMemberRequestsListResponse;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::OnQueryGroupInvitesSuccess(const FAccelByteModelsGetMemberRequestsListResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	// Success = 200
	httpStatus = 200;
	AccelByteModelsGetMemberRequestsListResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites::OnQueryGroupInvitesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	ErrorString = ErrorMessage;
	httpStatus = ErrorCode;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
