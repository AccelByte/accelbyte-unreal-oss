// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& ContextUserId,
	const FAccelByteModelsLimitOffsetRequest& InRequestContent,
	const FString& InGroupId,
	const FOnGroupsRequestCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, RequestContent(InRequestContent)
	, GroupId(InGroupId)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(ContextUserId);
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGetMemberRequestsListResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::OnQueryGroupJoinRequestsSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::OnQueryGroupJoinRequestsError);

	API_CLIENT_CHECK_GUARD(ErrorString);
	ApiClient->Group.GetGroupJoinRequests(GroupId, RequestContent, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FGroupsResult result(httpStatus, ErrorString, UniqueNetIdAccelByteResource);
	Delegate.ExecuteIfBound(result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineGroupsAccelBytePtr GroupsInterface;
	if (!ensure(FOnlineGroupsAccelByte::GetFromSubsystem(SubsystemPin.Get(),  GroupsInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to QueryGroupJoinRequests, groups interface instance is not valid!"));
		return;
	}

	if (bWasSuccessful == false)
		return;

	GroupsInterface->SetCachedGroupRequests(AccelByteModelsGetMemberRequestsListResponse);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsGroupJoinRequestListPayload GroupJoinRequestListPayload{};
		GroupJoinRequestListPayload.GroupId = GroupId;
		GroupJoinRequestListPayload.UserId = UserId->GetAccelByteId();

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsGroupJoinRequestListPayload>(GroupJoinRequestListPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::OnQueryGroupJoinRequestsSuccess(const FAccelByteModelsGetMemberRequestsListResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	// Success = 200
	httpStatus = 200;
	AccelByteModelsGetMemberRequestsListResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGroupsQueryGroupJoinRequests::OnQueryGroupJoinRequestsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	ErrorString = ErrorMessage;
	httpStatus = ErrorCode;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
