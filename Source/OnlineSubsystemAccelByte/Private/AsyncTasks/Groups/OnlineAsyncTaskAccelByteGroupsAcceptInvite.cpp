// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsAcceptInvite.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsAcceptInvite::FOnlineAsyncTaskAccelByteGroupsAcceptInvite(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& UserIdInviteToAccept,
	const FString& InGroupId,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, GroupId(InGroupId)
	, Delegate(InDelegate)
	, httpStatus(0)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(UserIdInviteToAccept);
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsMemberRequestGroupResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteError);

	API_FULL_CHECK_GUARD(Group, ErrorString);
	Group->AcceptV2GroupInvitation(GroupId, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(SubsystemPin.Get(),  GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to AcceptInvite, groups interface instance is not valid!"));
		return;
	}

	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to AcceptInvite, not in a group!"));
		return;
	}

	FAccelByteGroupsInfoPtr CurrentGroupData = GroupsInterface->GetCachedGroupInfo();
	if (!CurrentGroupData.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to AcceptInvite, current group data is invalid!"));
		return;
	}
	
	// Add the player to the current group
	GroupsInterface->AddCachedGroupMember(CurrentGroupData->ABMemberRoleId, AccelByteModelsMemberRequestGroupResponse.UserId);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupInviteAcceptedPayload GroupInviteAcceptedPayload{};
		GroupInviteAcceptedPayload.GroupId = GroupId;
		GroupInviteAcceptedPayload.UserId = UserId->GetAccelByteId();

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupInviteAcceptedPayload>(GroupInviteAcceptedPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsMemberRequestGroupResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsAcceptInvite::OnAcceptInviteError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}