// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindGameSessionsV2.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

FOnlineAsyncTaskAccelByteFindGameSessionsV2::FOnlineAsyncTaskAccelByteFindGameSessionsV2(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& InSearchSettings)
    : FOnlineAsyncTaskAccelByte(InABInterface, true)
    , SearchSettings(InSearchSettings)
    , ResultsRemaining(SearchSettings->MaxSearchResults)
{
    // #TODO #SESSIONv2 Make this support the custom timeout value from the search settings handle eventually...
    UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InSearchingPlayerId.AsShared());
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

    QueryStruct = FAccelByteModelsV2GameSessionQuery();
    for (const TPair<FName, FOnlineSessionSearchParam>& SearchParam : SearchSettings->QuerySettings.SearchParams)
    {
        // Do not include the session type in the query!
        if (SearchParam.Key == SETTING_SESSION_TYPE)
        {
            continue;
        }

        // Always ToUpper the search param key to prevent weirdness with Unreal FName casing.
        SearchParam.Value.Data.AddToJsonObject(QueryStruct.JsonWrapper.JsonObject.ToSharedRef(), SearchParam.Key.ToString().ToUpper(), true);
    }

    // Query first page of results to start
    QueryResultsPage(0);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::Tick()
{
    Super::Tick();
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	SearchSettings->SearchState = (bWasSuccessful) ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

    SessionInterface->TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::QueryResultsPage(int32 Offset)
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Offset: %d"), Offset);

    SetLastUpdateTimeToCurrentTime();

    // Mark that we are actively searching if we have not already
    if (SearchSettings->SearchState != EOnlineAsyncTaskState::InProgress)
    {
		SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
    }

    // Make call to query game sessions from offset with user defined limit
	const THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult> OnQueryGameSessionsSuccessDelegate = THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsSuccess, Offset);
	const FErrorHandler OnQueryGameSessionsErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsError);

    const int32 Limit = FMath::Min(ResultsRemaining, ResultsPerPage);
    ApiClient->Session.QueryGameSessions(QueryStruct, OnQueryGameSessionsSuccessDelegate, OnQueryGameSessionsErrorDelegate, Offset, Limit);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result, int32 LastOffset)
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionsFound: %d"), Result.Data.Num());

	SetLastUpdateTimeToCurrentTime();

    const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
    ensure(SessionInterface.IsValid());

    ResultsRemaining -= Result.Data.Num();
    for (const FAccelByteModelsV2GameSession& Session : Result.Data)
    {
        FOnlineSessionSearchResult SearchResult;
        if (!SessionInterface->ConstructGameSessionFromBackendSessionModel(Session, SearchResult.Session))
        {
            UE_LOG_AB(Warning, TEXT("Failed to convert session with ID '%s' to a session search result during FindSessions! Skipping!"), *Session.ID);
            continue;
        }

        SearchSettings->SearchResults.Emplace(SearchResult);
    }

    if (ResultsRemaining > 0 && !Result.Paging.Next.IsEmpty())
    {
		// If we still have not hit our maximum search results, but we also have results to grab from backend, then make a request
        // to get more results past the ones that we have already queried!
        const int32 NewOffset = LastOffset + Result.Data.Num();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to get more search results at offset %d"), NewOffset);
        QueryResultsPage(NewOffset);
    }
    else
    {
        // Otherwise, just complete the task so that we can return these results to the game
        AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Found all possible results for this search, completing task!"));
        CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
    }
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsError(int32 ErrorCode, const FString& ErrorMessage)
{
	SetLastUpdateTimeToCurrentTime();

    UE_LOG_AB(Warning, TEXT("Failed to query game sessions from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
    CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
