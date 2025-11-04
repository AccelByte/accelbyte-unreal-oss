// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsFindGroups.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsFindGroups::FOnlineAsyncTaskAccelByteGroupsFindGroups(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InSearchingUserId,
	const FAccelByteModelsGetGroupListRequest& InRequestedContent,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RequestedContent(InRequestedContent)
	, Delegate(InDelegate)
	, httpStatus(0)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingUserId);
}

void FOnlineAsyncTaskAccelByteGroupsFindGroups::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGetGroupListResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsFindGroups::OnFindGroupsSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsFindGroups::OnFindGroupsError);

	API_FULL_CHECK_GUARD(Group, ErrorString);
	Group->GetGroupList(RequestedContent, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroups::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroups::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bWasSuccessful == false)
		return;

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if(!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(SubsystemPin.Get(),  GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to FindGroups, groups interface instance is not valid!"));
		return;
	}

	GroupsInterface->SetCachedGroupResults(AccelByteModelsGetGroupListResponse);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupFindPayload GroupFindPayload{};
		GroupFindPayload.UserId = UserId->GetAccelByteId();
		GroupFindPayload.GroupName = RequestedContent.GroupName;
		GroupFindPayload.GroupRegion = RequestedContent.GroupRegion;

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupFindPayload>(GroupFindPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroups::OnFindGroupsSuccess(const FAccelByteModelsGetGroupListResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = static_cast<int32>(ErrorCodes::StatusOk);
	AccelByteModelsGetGroupListResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsFindGroups::OnFindGroupsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	httpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}