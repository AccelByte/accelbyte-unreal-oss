// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsGetGroup.h"

FOnlineAsyncTaskAccelByteGroupsGetGroup::FOnlineAsyncTaskAccelByteGroupsGetGroup(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InContextUserId,
	const FUniqueNetId& InGroupId,
	const FOnFindGroupsCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, GroupId(FUniqueNetIdAccelByteResource::CastChecked(InGroupId))
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InContextUserId);
}

void FOnlineAsyncTaskAccelByteGroupsGetGroup::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGroupInformation>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsGetGroup::OnGetGroupSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsGetGroup::OnGetGroupError);

	ApiClient->Group.GetGroup(GroupId->ToString(), OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsGetGroup::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FUniqueNetIdPtr GenericUserId = StaticCastSharedPtr<const FUniqueNetId>(UserId);
	if (!ensure(GenericUserId.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get group, user ID is not valid!"));
		return;
	}

	//FOnFindGroupsCompleted result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	//Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsGetGroup::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get group, groups interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful == false)
		return;

	// Cannot get group if user is not in a group
	if (GroupsInterface->IsGroupValid() == false)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get group, not in a group!"));
		return;
	}

	//GroupsInterface->SetLastFetchedGroupInfo(AccelByteModelsGroupInformation);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsGetGroup::OnGetGroupSuccess(const FAccelByteModelsGroupInformation& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GroupId: %s"), *Result.GroupId);

	AccelByteModelsGroupInformation = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	UniqueNetIdAccelByteResource = FUniqueNetIdAccelByteResource::Create(Result.GroupId);

	// Success = 201
	httpStatus = 201;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsGetGroup::OnGetGroupError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	httpStatus = ErrorCode;
}
