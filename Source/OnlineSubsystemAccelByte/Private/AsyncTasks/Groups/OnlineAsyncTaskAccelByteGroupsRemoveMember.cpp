// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsRemoveMember.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsRemoveMember::FOnlineAsyncTaskAccelByteGroupsRemoveMember(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& GroupMemberUserId,
	const FString& InGroupId,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, MemberId(FUniqueNetIdAccelByteUser::CastChecked(GroupMemberUserId))
	, GroupId(InGroupId)
	, Delegate(InDelegate)
	, httpStatus(0)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsRemoveMember::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("MemberId: %s"), *MemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsKickGroupMemberResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsRemoveMember::OnRemoveMemberSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsRemoveMember::OnRemoveMemberError);

	API_FULL_CHECK_GUARD(Group, ErrorString);
	Group->KickV2GroupMember(MemberId->GetAccelByteId(), GroupId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsRemoveMember::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsRemoveMember::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(SubsystemPin.Get(),  GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove member, groups interface instance is not valid!"));
		return;
	}

	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove member, not in a group!"));
		return;
	}

	// Remove cached member
	GroupsInterface->RemoveCachedMember(AccelByteModelsKickGroupMemberResponse.KickedUserId);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupMemberKickedPayload GroupMemberKickedPayload{};
		GroupMemberKickedPayload.GroupId = GroupId;
		GroupMemberKickedPayload.AdminUserId = UserId->GetAccelByteId();
		GroupMemberKickedPayload.KickedUserId = AccelByteModelsKickGroupMemberResponse.KickedUserId;

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupMemberKickedPayload>(GroupMemberKickedPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsRemoveMember::OnRemoveMemberSuccess(const FAccelByteModelsKickGroupMemberResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsKickGroupMemberResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsRemoveMember::OnRemoveMemberError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}