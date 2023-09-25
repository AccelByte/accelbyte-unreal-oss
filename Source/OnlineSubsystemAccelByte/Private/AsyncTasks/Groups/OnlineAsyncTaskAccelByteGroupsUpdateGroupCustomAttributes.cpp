// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& InAdminUserId,
	const FString& InGroupId,
	const FAccelByteModelsUpdateGroupCustomAttributesRequest& InRequestContent,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, AdminMemberId(FUniqueNetIdAccelByteUser::CastChecked(InAdminUserId))
	, GroupId(InGroupId)
	, Delegate(InDelegate)
{
	LocalUserNum = GroupAdmin;
	RequestedContent = InRequestContent;
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("AdminMemberId: %s"), *AdminMemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::OnUpdateGroupCustomAttributesSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::OnUpdateGroupCustomAttributesError);

	ApiClient->Group.UpdateV2GroupCustomAttributes(GroupId, RequestedContent, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to UpdateGroupCustomAttributes, groups interface instance is not valid!"));
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

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::OnUpdateGroupCustomAttributesSuccess(const FAccelByteModelsGroupInformation& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsGroupInformation = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomAttributes::OnUpdateGroupCustomAttributesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}