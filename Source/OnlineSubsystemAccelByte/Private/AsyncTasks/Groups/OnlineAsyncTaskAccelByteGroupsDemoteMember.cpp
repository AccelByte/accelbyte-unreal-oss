// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsDemoteMember.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsDemoteMember::FOnlineAsyncTaskAccelByteGroupsDemoteMember(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& GroupMemberUserId,
	const FString& InGroupId,
	const FString& MemberRoleId,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, MemberId(FUniqueNetIdAccelByteUser::CastChecked(GroupMemberUserId))
	, GroupId(InGroupId)
	, RoleId(MemberRoleId)
	, Delegate(InDelegate)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsDemoteMember::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("MemberId: %s"), *MemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsDemoteMember::OnDemoteMemberSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsDemoteMember::OnDemoteMemberError);

	FAccelByteModelsUserIdWrapper MemberUserId;
	MemberUserId.UserId = MemberId->GetAccelByteId();

	ApiClient->Group.DeleteV2MemberRole(RoleId, GroupId, MemberUserId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDemoteMember::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDemoteMember::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to demote member, groups interface instance is not valid!"));
		return;
	}

	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to demote member, not in a group!"));
		return;
	}

	// Remove cached group member
	GroupsInterface->RemoveCachedMember(RoleId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDemoteMember::OnDemoteMemberSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(GroupId);
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsDemoteMember::OnDemoteMemberError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}