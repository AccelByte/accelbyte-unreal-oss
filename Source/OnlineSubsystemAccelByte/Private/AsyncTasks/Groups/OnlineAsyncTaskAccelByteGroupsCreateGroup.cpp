// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsCreateGroup.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsCreateGroup::FOnlineAsyncTaskAccelByteGroupsCreateGroup(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InContextUserId,
	const FAccelByteGroupsInfo& InGroupInfo,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, GroupInfo(InGroupInfo)
	, Delegate(InDelegate)
	, httpStatus(0)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InContextUserId);
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	FAccelByteModelsGroupInformation CachedGroupInfo = GroupInfo.ABGroupInfo;

	FAccelByteModelsCreateGroupRequest NewGroupRequest;
	NewGroupRequest.ConfigurationCode = CachedGroupInfo.ConfigurationCode;
	NewGroupRequest.CustomAttributes = CachedGroupInfo.CustomAttributes;
	NewGroupRequest.GroupDescription = CachedGroupInfo.GroupDescription;
	NewGroupRequest.GroupIcon = CachedGroupInfo.GroupIcon;
	NewGroupRequest.GroupMaxMember = CachedGroupInfo.GroupMaxMember;
	NewGroupRequest.GroupName = CachedGroupInfo.GroupName;
	NewGroupRequest.GroupRegion = CachedGroupInfo.GroupRegion;
	NewGroupRequest.GroupRules = CachedGroupInfo.GroupRules;
	NewGroupRequest.GroupType = CachedGroupInfo.GroupType;

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupError);

	API_CLIENT_CHECK_GUARD(ErrorString);
	ApiClient->Group.CreateV2Group(NewGroupRequest, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(SubsystemPin.Get(),  GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to CreateGroup, groups interface instance is not valid!"));
		return;
	}

	const FAccelByteGroupsInfoRef AccelByteGroupsInfo = FAccelByteGroupsInfo::Create(
		GroupInfo.SenderUserId,
		GroupInfo.Namespace,
		GroupInfo.OwnerId->AsShared().Get(),
		AccelByteModelsGroupInformation,
		GroupInfo.ABAdminRoleId,
		GroupInfo.ABMemberRoleId);

	GroupsInterface->SetCachedGroupInfo(AccelByteGroupsInfo);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupCreatedPayload GroupCreatedPayload{};
		GroupCreatedPayload.ConfigurationCode = AccelByteModelsGroupInformation.ConfigurationCode;
		GroupCreatedPayload.GroupId = AccelByteModelsGroupInformation.GroupId;
		GroupCreatedPayload.GroupMaxMember = AccelByteModelsGroupInformation.GroupMaxMember;
		GroupCreatedPayload.GroupName = AccelByteModelsGroupInformation.GroupName;
		GroupCreatedPayload.GroupRegion = AccelByteModelsGroupInformation.GroupRegion;

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupCreatedPayload>(GroupCreatedPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupSuccess(const FAccelByteModelsGroupInformation& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsGroupInformation = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsCreateGroup::OnCreateGroupError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}