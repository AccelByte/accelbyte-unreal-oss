// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAuthUser.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteAuthUser::FOnlineAsyncTaskAccelByteAuthUser(FOnlineSubsystemAccelByte* const InABInterface, const FString& InUserId, const FOnAuthUserCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, UserId(InUserId)
	, bRequestResult(false)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteAuthUser::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s"), *UserId);

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteAuthUser, Auth, THandler<FGetUserBansResponse>);
	SERVER_API_CLIENT_CHECK_GUARD();
	ServerApiClient->ServerUser.GetUserBanInfo(UserId, OnAuthSuccessDelegate, OnAuthErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAuthUser::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Delegate.ExecuteIfBound(bRequestResult, UserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAuthUser::OnAuthSuccess(const FGetUserBansResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	if (Result.Data.Num() == 0)
	{
		bRequestResult = true;
	}
	else
	{
		bRequestResult = false;
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to query AccelByte Authentication for user '%s' (lists: %d)!"), *UserId, Result.Data.Num());
}

void FOnlineAsyncTaskAccelByteAuthUser::OnAuthError(int32 ErrorCode, const FString& ErrorMessage)
{
	bRequestResult = false;

	UE_LOG_AB(Warning, TEXT("Failed to Authenticate for user '%s'; Error Code: %d; Error Message: %s"), *UserId, ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
