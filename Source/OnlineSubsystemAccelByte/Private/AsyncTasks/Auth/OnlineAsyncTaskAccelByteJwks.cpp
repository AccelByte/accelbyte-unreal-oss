// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteJwks.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteJwks::FOnlineAsyncTaskAccelByteJwks(FOnlineSubsystemAccelByte* const InABInterface, const FOnJwksCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteJwks::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteJwks, Jwks, THandler<FJwkSet>);

	SERVER_API_CLIENT_CHECK_GUARD();
	ServerApiClient->ServerOauth2.GetJwks(OnJwksSuccessDelegate, OnJwksErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJwks::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Delegate.ExecuteIfBound(JwkSet);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJwks::OnJwksSuccess(const FJwkSet& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	JwkSet = Response;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to query AccelByte's Jwks (lists: %d)!"), JwkSet.keys.Num());
}

void FOnlineAsyncTaskAccelByteJwks::OnJwksError(int32 InErrorCode, const FString& InErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to query AccelByte's Jwks; Error Code: %d; Error Message: %s"), InErrorCode, *InErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
