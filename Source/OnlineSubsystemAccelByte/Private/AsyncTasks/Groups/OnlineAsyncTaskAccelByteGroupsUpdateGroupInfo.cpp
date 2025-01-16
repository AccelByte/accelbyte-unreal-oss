// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsUpdateGroupInfo.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

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
	, httpStatus(0)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("AdminMemberId: %s"), *AdminMemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::OnUpdateGroupInfoSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupInfo::OnUpdateGroupInfoError);

	API_CLIENT_CHECK_GUARD(ErrorString);
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
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(SubsystemPin.Get(),  GroupsInterface)))
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

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupUpdatedPayload GroupUpdatedPayload{};
		GroupUpdatedPayload.ConfigurationCode = AccelByteModelsGroupInformation.ConfigurationCode;
		GroupUpdatedPayload.GroupId = AccelByteModelsGroupInformation.GroupId;
		GroupUpdatedPayload.GroupMaxMember = AccelByteModelsGroupInformation.GroupMaxMember;
		GroupUpdatedPayload.GroupName = AccelByteModelsGroupInformation.GroupName;
		GroupUpdatedPayload.GroupRegion = AccelByteModelsGroupInformation.GroupRegion;

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupUpdatedPayload>(GroupUpdatedPayload));
	}

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