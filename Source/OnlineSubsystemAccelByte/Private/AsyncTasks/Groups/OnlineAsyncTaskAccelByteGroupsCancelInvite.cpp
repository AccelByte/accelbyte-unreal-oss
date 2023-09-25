// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsCancelInvite.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsCancelInvite::FOnlineAsyncTaskAccelByteGroupsCancelInvite(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& AdminUserId,
	const FString& InUserIdToCancel,
	const FString& InGroupId,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, UserIdToCancel(InUserIdToCancel)
	, GroupId(InGroupId)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(AdminUserId);
}

void FOnlineAsyncTaskAccelByteGroupsCancelInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsMemberRequestGroupResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsCancelInvite::OnCancelInviteSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsCancelInvite::OnCancelInviteError);

	ApiClient->Group.CancelGroupMemberInvitation(UserIdToCancel, GroupId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCancelInvite::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCancelInvite::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to CancelInvite, groups interface instance is not valid!"));
		return;
	}

	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to CancelInvite, not in a group!"));
		return;
	}

	// Remove the invite
	GroupsInterface->RemoveCachedInvites(AccelByteModelsMemberRequestGroupResponse.UserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCancelInvite::OnCancelInviteSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsMemberRequestGroupResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCancelInvite::OnCancelInviteError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}