// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsJoinGroup.h"

FOnlineAsyncTaskAccelByteGroupsJoinGroup::FOnlineAsyncTaskAccelByteGroupsJoinGroup(
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

void FOnlineAsyncTaskAccelByteGroupsJoinGroup::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsJoinGroupResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsJoinGroup::OnJoinGroupSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsJoinGroup::OnJoinGroupError);

	ApiClient->Group.JoinV2Group(GroupInfo.ABGroupInfo.GroupId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsJoinGroup::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join group, user ID is not valid!"));
		return;
	}

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsJoinGroup::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to join group, groups interface instance is not valid!"));
		return;
	}

	// Nothing to do here? Backend receives request from player to join group.

	if (bWasSuccessful == false)
		return;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsJoinGroup::OnJoinGroupSuccess(const FAccelByteModelsJoinGroupResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);

	// Success = 200
	httpStatus = 200;

	AccelByteModelsJoinGroupResponse = Result;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsJoinGroup::OnJoinGroupError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	// Failure = not 200
	httpStatus = 0;
}
