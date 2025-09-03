// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindV2PartyById.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

using namespace AccelByte;

// Initialize as a server task if we are running a dedicated server, as this doubles as a server task. Otherwise, use
// no flags to indicate that it is a client task.	
FOnlineAsyncTaskAccelByteFindV2PartyById::FOnlineAsyncTaskAccelByteFindV2PartyById(FOnlineSubsystemAccelByte* const InABInterface
	, FUniqueNetId const& InSearchingPlayerId
	, FUniqueNetId const& InSessionId
	, FOnSingleSessionResultCompleteDelegate const& InDelegate
	, TSharedPtr<FAccelByteKey> InLockKey /* = nullptr */
	, bool IsDedicatedServer /* = false */)
	: FOnlineAsyncTaskAccelByte(InABInterface
		, INVALID_CONTROLLERID
		, IsDedicatedServer ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None)
		, InLockKey)
	, SessionId(FUniqueNetIdAccelByteResource::CastChecked(InSessionId))
	, Delegate(InDelegate)
{
	if (!IsDedicatedServer)
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingPlayerId);
	}
}

FOnlineAsyncTaskAccelByteFindV2PartyById::FOnlineAsyncTaskAccelByteFindV2PartyById(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, FUniqueNetId const& InSessionId
	, FOnSingleSessionResultCompleteDelegate const& InDelegate
	, TSharedPtr<FAccelByteKey> InLockKey /* = nullptr */
	, bool IsDedicatedServer /* = false */)
	: FOnlineAsyncTaskAccelByte(InABInterface
		, InLocalUserNum
		, IsDedicatedServer ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None)
		, InLockKey)
	, SessionId(FUniqueNetIdAccelByteResource::CastChecked(InSessionId))
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId->ToDebugString());

	TRY_PIN_SUBSYSTEM();

	OnGetPartySessionDetailsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsSuccess);
	OnGetPartySessionDetailsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsError);

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);

	if (IsDS.IsSet())
	{
		if (IsDS.GetValue())
		{
			SERVER_API_CLIENT_CHECK_GUARD();

			ServerApiClient->ServerSession.GetPartyDetails(SessionId->ToString()
				, OnGetPartySessionDetailsSuccessDelegate
				, OnGetPartySessionDetailsErrorDelegate);
		}
		else
		{
			API_FULL_CHECK_GUARD(Session);
			Session->GetPartyDetails(SessionId->ToString()
				, OnGetPartySessionDetailsSuccessDelegate
				, OnGetPartySessionDetailsErrorDelegate);
		}
	}
	else
	{
		TaskOnlineError = EOnlineErrorResult::NotImplemented;
		TaskErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusBadRequest);
		TaskErrorStr = TEXT("request-failed-not-implemented");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user record, access denied!"));
		return;
	}

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	TRY_PIN_SUBSYSTEM();

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize finding party by ID as our session interface is invalid!"));
			return;
		}

		SessionInterface->ConstructPartySessionFromBackendSessionModel(FoundPartySession, FoundSessionResult.Session);
	}

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, FoundSessionResult);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsSuccess(const FAccelByteModelsV2PartySession& InFoundPartySession)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FoundPartySession = InFoundPartySession;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to find party session with ID '%s'! Error code: %d; Error message: %s"), *SessionId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
