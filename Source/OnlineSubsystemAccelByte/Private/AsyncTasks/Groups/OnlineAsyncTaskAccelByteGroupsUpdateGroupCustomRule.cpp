// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& InAdminUserId,
	const FString& InGroupId,
	const FAccelByteModelsUpdateCustomRulesRequest& InRequestContent,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, AdminMemberId(FUniqueNetIdAccelByteUser::CastChecked(InAdminUserId))
	, GroupId(InGroupId)
	, RequestedContent(InRequestContent)
	, Delegate(InDelegate)
	, httpStatus(0)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("AdminMemberId: %s"), *AdminMemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::OnUpdateGroupCustomRuleSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::OnUpdateGroupCustomRuleError);

	API_FULL_CHECK_GUARD(Group, ErrorString);
	Group->UpdateV2GroupCustomRule(GroupId, RequestedContent, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(SubsystemPin.Get(),  GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to UpdateGroupCustomRule, groups interface instance is not valid!"));
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
		FAccelByteModelsGroupCustomRuleUpdatedPayload GroupCustomRuleUpdatedPayload{};
		GroupCustomRuleUpdatedPayload.GroupId = GroupId;
		GroupCustomRuleUpdatedPayload.AdminUserId = AdminMemberId->GetAccelByteId();
		GroupCustomRuleUpdatedPayload.GroupRules = AccelByteModelsGroupInformation.GroupRules.GroupCustomRule;

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupCustomRuleUpdatedPayload>(GroupCustomRuleUpdatedPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::OnUpdateGroupCustomRuleSuccess(const FAccelByteModelsGroupInformation& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsGroupInformation = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdateGroupCustomRule::OnUpdateGroupCustomRuleError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}