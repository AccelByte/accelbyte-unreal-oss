// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendPSNEvents.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSendPSNEvents::FOnlineAsyncTaskAccelByteSendPSNEvents(FOnlineSubsystemAccelByte* const InABSubsystem
	, const FAccelByteModelsAchievementBulkCreatePSNEventRequest& InRequest
	, const FOnSendPSNEventsCompleteDelegate& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Request(InRequest)
	, Delegate(InCompletionDelegate)
{
}

void FOnlineAsyncTaskAccelByteSendPSNEvents::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (!ensure(IsRunningDedicatedServer()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot attempt to send PSN events to achievement service as a client!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD()

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteSendPSNEvents, CreatePSNEvent, THandler<FAccelByteModelsAchievementBulkCreatePSNEventResponse>);
	ServerApiClient->ServerAchievement.BulkCreatePSNEvent(Request
		, OnCreatePSNEventSuccessDelegate
		, OnCreatePSNEventErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendPSNEvents::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Delegate.ExecuteIfBound(bWasSuccessful, Response);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendPSNEvents::OnCreatePSNEventSuccess(const FAccelByteModelsAchievementBulkCreatePSNEventResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Response = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendPSNEvents::OnCreatePSNEventError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to create PSN events through achievement service!", ErrorCode, ErrorMessage)
}
