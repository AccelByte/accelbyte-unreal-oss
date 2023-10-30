// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteServerQueryPartySessionsV2.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2"

FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2(FOnlineSubsystemAccelByte* const InABInterface, const FAccelByteModelsV2QueryPartiesRequest& InRequest, int64 InOffset, int64 InLimit)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Request(InRequest), Offset(InOffset), Limit(InLimit)
{
}

void FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		ErrorText = FText::FromString(TEXT("server-query-party-session-v2-not-a-server"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("This query only works for Dedicated Server"));
		return;
	}

	OnQuerySuccess = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedPartyQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::OnQueryPartySessionsSuccess);
	OnQueryFailed = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::OnQueryPartySessionsFailed);

	FMultiRegistry::GetServerApiClient()->ServerSession.QueryPartySessions(Request, OnQuerySuccess, OnQueryFailed, Offset, Limit);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *ErrorText.ToString());

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (!ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session interface instance from online subsystem!"));
		return;
	}

	SessionInterface->TriggerOnServerQueryPartySessionsCompleteDelegates(QueryResult, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::OnQueryPartySessionsSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result)
{
	QueryResult = Result;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::OnQueryPartySessionsFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorText = FText::FromString(TEXT("server-query-party-session-v2-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE
