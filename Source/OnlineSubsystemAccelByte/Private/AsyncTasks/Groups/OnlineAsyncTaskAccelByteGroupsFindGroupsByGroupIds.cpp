// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InSearchingUserId,
	const TArray<FString> InGroupIds,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, GroupIds(InGroupIds)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingUserId);
}

void FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGetGroupListResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::OnFindGroupsByGroupIdsSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::OnFindGroupsByGroupIdsError);

	API_CLIENT_CHECK_GUARD(ErrorString);
	ApiClient->Group.GetGroupsByGroupIds(GroupIds, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(Subsystem, GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to FindGroupsByGroupIds, groups interface instance is not valid!"));
		return;
	}

	GroupsInterface->SetCachedGroupListByGroupIdsResults(AccelByteModelsGetGroupListResponse);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupFindByIdsPayload GroupFindByIdsPayload{};
		GroupFindByIdsPayload.UserId = UserId->GetAccelByteId();
		GroupFindByIdsPayload.GroupIds = GroupIds;

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupFindByIdsPayload>(GroupFindByIdsPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::OnFindGroupsByGroupIdsSuccess(const FAccelByteModelsGetGroupListResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsGetGroupListResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds::OnFindGroupsByGroupIdsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}