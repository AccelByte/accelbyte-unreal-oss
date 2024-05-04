// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule(
	FOnlineSubsystemAccelByte* const InABInterface,
	const int32& GroupAdmin,
	const FUniqueNetId& InAdminUserId,
	const FString& InGroupId,
	const EAccelByteAllowedAction& InAllowedAction,
	const FAccelByteModelsUpdateGroupPredefinedRuleRequest& InRequestContent,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, AdminMemberId(FUniqueNetIdAccelByteUser::CastChecked(InAdminUserId))
	, GroupId(InGroupId)
	, AllowedAction(InAllowedAction)
	, RequestedContent(InRequestContent)
	, Delegate(InDelegate)
{
	LocalUserNum = GroupAdmin;
}

void FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("AdminMemberId: %s"), *AdminMemberId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::OnUpdatePredefinedRuleSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::OnUpdatePredefinedRuleError);

	API_CLIENT_CHECK_GUARD(ErrorString);
	ApiClient->Group.UpdateV2GroupPredefinedRule(GroupId, AllowedAction, RequestedContent, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to UpdatePredefinedRule, groups interface instance is not valid!"));
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

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupPredefinedRuleUpdatedPayload GroupPredefinedRuleUpdatedPayload{};
		GroupPredefinedRuleUpdatedPayload.GroupId = GroupId;
		GroupPredefinedRuleUpdatedPayload.AdminUserId = UserId->GetAccelByteId();

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupPredefinedRuleUpdatedPayload>(GroupPredefinedRuleUpdatedPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::OnUpdatePredefinedRuleSuccess(const FAccelByteModelsGroupInformation& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsGroupInformation = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsUpdatePredefinedRule::OnUpdatePredefinedRuleError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}