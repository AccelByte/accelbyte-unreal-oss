// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindV1Sessions.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/OnlineSessionNames.h"
#endif // ENGINE_MAJOR_VERSION >= 5
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Misc/DefaultValueHelper.h"
#include "OnlineSubsystemAccelByteDefines.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteFindV1Sessions::FOnlineAsyncTaskAccelByteFindV1Sessions(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& InSearchSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, SearchSettings(InSearchSettings)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingPlayerId);
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	if (SearchSettings->SearchState != EOnlineAsyncTaskState::NotStarted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Search is in a state other than 'NotStarted', cannot continue with finding sessions! Current search state: %s"), EOnlineAsyncTaskState::ToString(SearchSettings->SearchState));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
	SearchSettings->SearchResults.Empty();

	// We need a way to signal to the SessionBrowser APIs whether we want to search for P2P relay sessions, or for dedicated matches
	// Only way we can do this is by 
	FString SearchType;
	SearchSettings->QuerySettings.Get(SETTING_SEARCH_TYPE, SearchType);

	// Default to searching for dedicated sessions, P2P session search will be opt-in
	if (SearchType.IsEmpty())
	{
		SearchType = SETTING_SEARCH_TYPE_DEDICATED;
	}

	THandler<FAccelByteModelsSessionBrowserGetResult> OnSessionBrowserFindSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsSessionBrowserGetResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteFindV1Sessions::OnSessionBrowserFindSuccess);
	FErrorHandler OnSessionBrowserFindErrorDelegate = FErrorHandler::CreateLambda([this](int32 ErrorCode, const FString& ErrorMessage) {
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		UE_LOG_AB(Error, TEXT("Failed to find sessions! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	});

	FString RegionName = TEXT("");
	SearchSettings->QuerySettings.Get(SETTING_REGION, RegionName);
	API_CLIENT_CHECK_GUARD();
	ApiClient->SessionBrowser.GetGameSessions(SearchType, FString(""), OnSessionBrowserFindSuccessDelegate, OnSessionBrowserFindErrorDelegate, 0, SearchSettings->MaxSearchResults);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off task to find %s game sessions!"), *SearchType);
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::Done;
		SearchSettings->SearchResults = SearchResults;
	}
	else
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	check(Subsystem != nullptr);
	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	SessionInterface->TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::OnSessionBrowserFindSuccess(const FAccelByteModelsSessionBrowserGetResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Session Count: %d"), Result.Sessions.Num());

	if (SearchSettings->SearchState != EOnlineAsyncTaskState::InProgress)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Cannot set up session find results if the search is not InProgress! Current state: %s"), EOnlineAsyncTaskState::ToString(SearchSettings->SearchState));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	const TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize finding a game session as our session interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Update the timeout just in case processing takes a bit of time
	SetLastUpdateTimeToCurrentTime();

	for (const FAccelByteModelsSessionBrowserData& FoundSession : Result.Sessions)
	{
		FOnlineSessionSearchResult SearchResult;
		SearchResult.PingInMs = -1;
		SessionInterface->ConstructGameSessionFromBackendSessionModel(FoundSession, SearchResult.Session);
		SearchResults.Add(SearchResult);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}
