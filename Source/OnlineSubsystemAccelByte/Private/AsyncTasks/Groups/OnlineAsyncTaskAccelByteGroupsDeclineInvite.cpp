// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsDeclineInvite.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsDeclineInvite::FOnlineAsyncTaskAccelByteGroupsDeclineInvite(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& UserIdRejectingInvite,
	const FString& InGroupId,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, GroupId(InGroupId)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(UserIdRejectingInvite);
}

void FOnlineAsyncTaskAccelByteGroupsDeclineInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsMemberRequestGroupResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsDeclineInvite::OnDeclineInviteSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsDeclineInvite::OnDeclineInviteError);

	ApiClient->Group.RejectV2GroupInvitation(GroupId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDeclineInvite::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDeclineInvite::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to DeclineInvite, groups interface instance is not valid!"));
		return;
	}

	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to DeclineInvite, not in a group!"));
		return;
	}

	// Remove the invite
	GroupsInterface->RemoveCachedInvites(AccelByteModelsMemberRequestGroupResponse.UserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDeclineInvite::OnDeclineInviteSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsMemberRequestGroupResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDeclineInvite::OnDeclineInviteError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}