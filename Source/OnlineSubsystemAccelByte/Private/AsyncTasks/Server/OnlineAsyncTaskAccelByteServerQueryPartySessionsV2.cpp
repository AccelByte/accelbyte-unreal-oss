// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteServerQueryPartySessionsV2.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2"

FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2(FOnlineSubsystemAccelByte* const InABInterface
	, const FAccelByteModelsV2QueryPartiesRequest& InRequest
	, int64 InOffset
	, int64 InLimit)
	: FOnlineAsyncTaskAccelByte(InABInterface
		, INVALID_CONTROLLERID
		, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Request(InRequest), Offset(InOffset), Limit(InLimit)
{
}

void FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	TRY_PIN_SUBSYSTEM();

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet() || !IsDS.GetValue())
	{
		ErrorText = FText::FromString(TEXT("server-query-party-session-v2-not-a-server"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("This query only works for Dedicated Server"));
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD();

	OnQuerySuccess = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedPartyQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::OnQueryPartySessionsSuccess);
	OnQueryFailed = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::OnQueryPartySessionsFailed);

	ServerApiClient->ServerSession.QueryPartySessions(Request, OnQuerySuccess, OnQueryFailed, Offset, Limit);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *ErrorText.ToString());

	TRY_PIN_SUBSYSTEM();

	FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensureAlways(SessionInterface.IsValid()))
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
