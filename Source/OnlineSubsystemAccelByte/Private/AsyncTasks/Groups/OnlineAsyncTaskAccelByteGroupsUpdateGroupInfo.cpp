// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsUpdateGroupInfo.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& InAdminUserId,
	const FString& InGroupId,
	const bool& InCompletelyReplace,
	const FAccelByteModelsGroupUpdatable& InRequestedContent,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, AdminMemberId(FUniqueNetIdAccelByteUser::CastChecked(InAdminUserId))
	, GroupId(InGroupId)
	, CompletelyReplace(InCompletelyReplace)
	, RequestedContent(InRequestedContent)
	, Delegate(InDelegate)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("AdminMemberId: %s"), *AdminMemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::OnUpdateGroupInfoSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::OnUpdateGroupInfoError);

	ApiClient->Group.UpdateGroup(GroupId, CompletelyReplace, RequestedContent, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to UpdateGroupInfo, groups interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful == false)
		return;

	FAccelByteGroupsInfoPtr CurrentGroupData = GroupsInterface->GetCachedGroupInfo();
	if (!CurrentGroupData.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove member, current group data is invalid!"));
		return;
	}

	CurrentGroupData->SetCachedABGroupInfo(AccelByteModelsGroupInformation);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::OnUpdateGroupInfoSuccess(const FAccelByteModelsGroupInformation& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsGroupInformation = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::OnUpdateGroupInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}