// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRestoreAllV2Sessions.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineError.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteRestoreAllV2Sessions"

FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::FOnlineAsyncTaskAccelByteRestoreAllV2Sessions(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnRestoreActiveSessionsComplete& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, CompletionDelegate(InCompletionDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	// Get information about the user's party, which then will give us a party to restore if we are in one
	OnGetMyPartiesSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedPartyQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyPartiesSuccess);
	OnGetMyPartiesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyPartiesError);
	API_FULL_CHECK_GUARD(Session);
	Session->GetMyParties(OnGetMyPartiesSuccessDelegate, OnGetMyPartiesErrorDelegate);

	// Get information about the user's game sessions, which then will give us a game session to restore if we are in one
	OnGetMyGameSessionsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyGameSessionsSuccess);
	OnGetMyGameSessionsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyGameSessionsError);
	Session->GetMyGameSessions(OnGetMyGameSessionsSuccessDelegate, OnGetMyGameSessionsErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::Tick()
{
	Super::Tick();

	if (HasFinishedAsyncWork())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize restoring sessions as our session interface is invalid!"));
			return;
		}

		// Restored sessions that are already in the session interface are stale by this point, so just clear and move over our new array
		SessionInterface->RestoredSessions.Empty();
		SessionInterface->RestoredSessions = TArray<FOnlineRestoredSessionAccelByte>(MoveTemp(RestoredSessions));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
	CompletionDelegate.ExecuteIfBound(UserId.ToSharedRef().Get(), ONLINE_ERROR(Result));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

bool FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::HasFinishedAsyncWork()
{
	return bHasRetrievedPartySessionInfo && bHasRetrievedGameSessionInfo;
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyGameSessionsSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GameSessionCount: %d"), Result.Data.Num());

	SetLastUpdateTimeToCurrentTime();

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to construct restored game sessions as our session interface is invalid!");

	for (const FAccelByteModelsV2GameSession& Session : Result.Data)
	{
		// First, start by checking the session interface to make sure that we're not already in this session locally. If we
		// are then we can skip restoration of that session.
		if (SessionInterface->GetNamedSessionById(Session.ID) != nullptr)
		{
			continue;
		}

		// Then, check if this is just a session that we are invited to, if so we want to ignore. We expect developers to get
		// invites separately through the session interface.
		const FAccelByteModelsV2SessionUser* FoundMember = Session.Members.FindByPredicate([UserIdStr = UserId->GetAccelByteId()](const FAccelByteModelsV2SessionUser& Member) {
			return Member.ID == UserIdStr;
		});

		if (FoundMember == nullptr || FoundMember->StatusV2 == EAccelByteV2SessionMemberStatus::INVITED)
		{
			continue;
		}

		FOnlineRestoredSessionAccelByte RestoredSession{};
		RestoredSession.LocalOwnerId = UserId;
		RestoredSession.SessionType = EAccelByteV2SessionType::GameSession;

		if (!SessionInterface->ConstructGameSessionFromBackendSessionModel(Session, RestoredSession.Session.Session))
		{
			continue;
		}

		RestoredSessions.Emplace(RestoredSession);
	}

	bHasRetrievedGameSessionInfo = true;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyGameSessionsError(int32 ErrorCode, const FString& ErrorMessage)
{
	SetLastUpdateTimeToCurrentTime();
	UE_LOG_AB(Warning, TEXT("Failed to restore game sessions for user '%s' as the call to the backend failed! Error code: %d; Error message: %s"), *UserId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyPartiesSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PartySessionCount: %d"), Result.Data.Num());

	SetLastUpdateTimeToCurrentTime();

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to construct restored party sessions as our session interface is invalid!");

	for (const FAccelByteModelsV2PartySession& Session : Result.Data)
	{
		// First, start by checking the session interface to make sure that we're not already in this session locally. If we
		// are then we can skip restoration of that session.
		if (SessionInterface->GetNamedSessionById(Session.ID) != nullptr)
		{
			continue;
		}

		// Then, check if this is just a session that we are invited to, if so we want to ignore. We expect developers to get
		// invites separately through the session interface.
		const FAccelByteModelsV2SessionUser* FoundMember = Session.Members.FindByPredicate([UserIdStr = UserId->GetAccelByteId()](const FAccelByteModelsV2SessionUser& Member) {
			return Member.ID == UserIdStr;
		});

		if (FoundMember == nullptr || FoundMember->StatusV2 == EAccelByteV2SessionMemberStatus::INVITED)
		{
			continue;
		}

		FOnlineRestoredSessionAccelByte RestoredSession{};
		RestoredSession.SessionType = EAccelByteV2SessionType::PartySession;
		RestoredSession.LocalOwnerId = UserId;

		if (!SessionInterface->ConstructPartySessionFromBackendSessionModel(Session, RestoredSession.Session.Session))
		{
			continue;
		}

		RestoredSessions.Emplace(RestoredSession);
	}

	bHasRetrievedPartySessionInfo = true;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnGetMyPartiesError(int32 ErrorCode, const FString& ErrorMessage)
{
	SetLastUpdateTimeToCurrentTime();
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to restore party session for user '%s' as the call to the backend failed! Error code: %d; Error message: %s"), *UserId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteRestoreAllV2Sessions::OnTaskTimedOut()
{
	UE_LOG_AB(Verbose, TEXT("RestoreSessions timeout: bHasRetrievedPartySessionInfo %s bHasRetrievedGameSessionInfo %s"), LOG_BOOL_FORMAT(bHasRetrievedPartySessionInfo), LOG_BOOL_FORMAT(bHasRetrievedGameSessionInfo));

	// clean up bound delegate so when response is received it doesn't trigger the delegates
	OnGetMyPartiesSuccessDelegate.Unbind();
	OnGetMyPartiesErrorDelegate.Unbind();
	OnGetMyGameSessionsSuccessDelegate.Unbind();
	OnGetMyGameSessionsErrorDelegate.Unbind();
}

#undef ONLINE_ERROR_NAMESPACE
