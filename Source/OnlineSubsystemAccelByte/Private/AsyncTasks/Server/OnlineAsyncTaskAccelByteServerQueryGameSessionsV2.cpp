// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteServerQueryGameSessionsV2.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteServerQueryGameSessionV2"

FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2::FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2(FOnlineSubsystemAccelByte* const InABInterface, const FAccelByteModelsV2ServerQueryGameSessionsRequest& InRequest, int64 InOffset, int64 InLimit)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Request(InRequest), Offset(InOffset), Limit(InLimit)
{
}

void FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		ErrorText = FText::FromString(TEXT("server-query-game-session-v2-not-a-server"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("This query only works for Dedicated Server"));
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD()

	OnQuerySuccess = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2::OnQueryGameSessionsSuccess);
	OnQueryFailed = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2::OnQueryGameSessionsFailed);

	ServerApiClient->ServerSession.QueryGameSessions(Request, OnQuerySuccess, OnQueryFailed, Offset, Limit);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *ErrorText.ToString());

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (!ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session interface instance from online subsystem!"));
		return;
	}

	SessionInterface->TriggerOnServerQueryGameSessionsCompleteDelegates(QueryResult, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2::OnQueryGameSessionsSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result)
{
	QueryResult = Result;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2::OnQueryGameSessionsFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorText = FText::FromString(TEXT("server-query-game-session-v2-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE
