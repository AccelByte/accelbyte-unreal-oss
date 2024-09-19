// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryGameSessionHistories.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteQueryGameSessionHistories"

FOnlineAsyncTaskAccelByteQueryGameSessionHistories::FOnlineAsyncTaskAccelByteQueryGameSessionHistories(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, EAccelByteGeneralSortBy const& InSortBy
	, FPagedQuery const& InPage)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SortBy(InSortBy)
	, PagedQuery(InPage)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteQueryGameSessionHistories::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to join game session as our session interface instance is invalid!");

	/* Validate if the the user wants all (-1) or any specific number that greater than or equal to 100 */
	if (PagedQuery.Count == -1 || PagedQuery.Count >= MaximumQueryLimit)
	{
		QueryGameSessionHistory(PagedQuery.Start, MaximumQueryLimit);
	}
	else
	{
		QueryGameSessionHistory(PagedQuery.Start, PagedQuery.Count);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryGameSessionHistories::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		ErrorString = TEXT("Failed to trigger delegates to query game session history as AccelByte session interface is invalid!");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorString);
		return;
	}

	if (!bWasSuccessful)
	{
		SessionInterface->TriggerOnQueryGameSessionHistoryCompleteDelegates(GameSessionHistoriesResult
			, ONLINE_ERROR(EOnlineErrorResult::RequestFailure
				, FString::Printf(TEXT("%d"), HttpStatus)
				, FText::FromString(ErrorString)));

		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorString);
		return;
	}

	SessionInterface->TriggerOnQueryGameSessionHistoryCompleteDelegates(GameSessionHistoriesResult
		, ONLINE_ERROR(EOnlineErrorResult::Success));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryGameSessionHistories::QueryGameSessionHistory(const int32& Offset, const int32& Limit)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting query game session history, Offset: %d, Limit: %d"), Offset, Limit);

	OnQueryGameSessionHistorySuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsGameSessionHistoriesResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryGameSessionHistories::OnQueryGameSessionHistorySuccess);
	OnQueryGameSessionHistoryErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryGameSessionHistories::OnQueryGameSessionHistoryError);;
	API_CLIENT_CHECK_GUARD();
	ApiClient->SessionHistory.QueryGameSessionHistory(OnQueryGameSessionHistorySuccessDelegate, OnQueryGameSessionHistoryErrorDelegate, SortBy, Offset, Limit);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryGameSessionHistories::OnQueryGameSessionHistorySuccess(const FAccelByteModelsGameSessionHistoriesResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	HttpStatus = static_cast<int32>(ErrorCodes::StatusOk);

	// Decrease the paged query count for the next iteration
	for (FAccelByteModelsGameSessionHistoriesData const& SessionHistory : Result.Data)
	{
		if (PagedQuery.Count > 0)
		{
			PagedQuery.Count -= 1;

			if (PagedQuery.Count == 0)
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			}
		}

		GameSessionHistoriesResult.Add(SessionHistory);
	}

	// If the next page has value iterate until the validation above stop it.
	// Note: This should global function, can be accessed anywhere
	if (!Result.Paging.Next.IsEmpty())
	{
		FString UrlOut;
		FString Params;
		Result.Paging.Next.Split(TEXT("?"), &UrlOut, &Params);
		if (!Params.IsEmpty())
		{
			TArray<FString> ParamsArray;
			Params.ParseIntoArray(ParamsArray, TEXT("&"));
			int32 Offset = -1;
			int32 Limit = -1;
			for (const FString& Param : ParamsArray)
			{
				FString Key;
				FString Value;
				Param.Split(TEXT("="), &Key, &Value);
				if (Key.Equals(TEXT("offset")) && Value.IsNumeric())
				{
					Offset = FCString::Atoi(*Value);
				}
				else if (Key.Equals(TEXT("limit")) && Value.IsNumeric())
				{
					Limit = FCString::Atoi(*Value);
				}

				if (Offset != -1 && Limit != -1)
				{
					SetLastUpdateTimeToCurrentTime(); //Increase this AsyncTask lifespan to do the initial QueryGameSessionHistory operation

					QueryGameSessionHistory(Offset, Limit);

					AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));

					return;
				}
			}
		}
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryGameSessionHistories::OnQueryGameSessionHistoryError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	HttpStatus = ErrorCode;
	ErrorString = ErrorMessage;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE