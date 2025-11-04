// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserIdsMapping.h"

#include "Core/AccelByteError.h"
#include "Api/AccelByteUserApi.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryUserIdsMapping::FOnlineAsyncTaskAccelByteQueryUserIdsMapping
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId
	, const FString& InDisplayNameOrEmail
	, const FOnQueryUserIdsMappingComplete& InDelegate
	, int32 InOffset
	, int32 InLimit)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, DisplayNameOrEmail(InDisplayNameOrEmail)
	, Delegate(InDelegate)
	, Offset(InOffset)
	, Limit(InLimit)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryUserIdsMapping::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// First, check if the display name or email we wish to query an ID for is blank, if so we want to bail out
	if (DisplayNameOrEmail.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query user ID mapping as the display name or email passed in is blank!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	THandler<FPagedPublicUsersInfo> OnSuccess = TDelegateUtils<THandler<FPagedPublicUsersInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUserIdsMapping::OnSearchUserSuccessResponse);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUserIdsMapping::OnSearchUserErrorResponse);

	API_FULL_CHECK_GUARD(User, ErrorString);
	User->SearchUsers(DisplayNameOrEmail, EAccelByteSearchType::ALL, OnSuccess, OnError, Offset, Limit);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserIdsMapping::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; Is Delegate Bound: %s; bWasSuccessful: %s"), *UserId->ToString(), LOG_BOOL_FORMAT(Delegate.IsBound()), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, UserId.ToSharedRef().Get(), DisplayNameOrEmail, SearchedUsers, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserIdsMapping::OnSearchUserSuccessResponse(const FPagedPublicUsersInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result Amount: %d; UserId: %s"), Result.Data.Num(), *UserId->ToString());

	if (Result.Data.Num() < 1)
	{
		ErrorString = FString::Printf(TEXT("Query for '%s' had no results."), *DisplayNameOrEmail);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("%s"), *ErrorString);
		return;
	}

	for (const FPublicUserInfo& User : Result.Data)
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = User.UserId;
		SearchedUsers.Add(FUniqueNetIdAccelByteUser::Create(CompositeId));
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Found users for query '%s'! Total found users %d"), *DisplayNameOrEmail, SearchedUsers.Num());
}

void FOnlineAsyncTaskAccelByteQueryUserIdsMapping::OnSearchUserErrorResponse(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *Message);

	ErrorString = Message;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
