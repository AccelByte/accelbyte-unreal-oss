// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRefreshV2PartySession.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Api/AccelByteSessionApi.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

FOnlineAsyncTaskAccelByteRefreshV2PartySession::FOnlineAsyncTaskAccelByteRefreshV2PartySession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnRefreshSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	TRY_PIN_SUBSYSTEM_CONSTRUCTOR()

	IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
	if (ensure(SessionInterface.IsValid()))
	{
		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		if (ensure(Session != nullptr))
		{
			UserId = FUniqueNetIdAccelByteUser::CastChecked(Session->LocalOwnerId.ToSharedRef());
		}
	}
}

void FOnlineAsyncTaskAccelByteRefreshV2PartySession::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Could not refresh party session named '%s' as our session interface is invalid!", *SessionName.ToString());

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Could not refresh party session named '%s' as the session does not exist locally!", *SessionName.ToString());

	const FString SessionId = OnlineSession->GetSessionIdStr();
	AB_ASYNC_TASK_VALIDATE(!SessionId.Equals(TEXT("InvalidSession")), "Could not refresh party session named '%s' as there is not a valid session ID associated!", *SessionName.ToString());

	OnRefreshPartySessionSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshV2PartySession::OnRefreshPartySessionSuccess);
	OnRefreshPartySessionErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshV2PartySession::OnRefreshPartySessionError);;
	API_FULL_CHECK_GUARD(Session);
	Session->GetPartyDetails(SessionId, OnRefreshPartySessionSuccessDelegate, OnRefreshPartySessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2PartySession::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		if (bWasSessionRemoved)
		{
			SessionInterface->RemoveNamedSession(SessionName);
		}
		else
		{
			SessionInterface->UpdateInternalPartySession(SessionName, RefreshedPartySession);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2PartySession::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
		if (ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
		{
			if (bWasSessionRemoved)
			{
				SessionInterface->TriggerOnSessionRemovedDelegates(SessionName);
			}
			else
			{
				SessionInterface->TriggerOnSessionUpdateReceivedDelegates(SessionName);
			}
		}
	}

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2PartySession::OnRefreshPartySessionSuccess(const FAccelByteModelsV2PartySession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	RefreshedPartySession = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2PartySession::OnRefreshPartySessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	if (ErrorCode == static_cast<int32>(AccelByte::ErrorCodes::SessionPartyNotFound))
	{
		// Party session was not found when we attempted to refresh, treat this as the session being removed
		UE_LOG_AB(Verbose, TEXT("Party session '%s' was not found when attempting to refresh, treating it as removed!"), *SessionName.ToString());
		bWasSessionRemoved = true;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	AB_ASYNC_TASK_REQUEST_FAILED("Request to refresh party session failed on backend!", ErrorCode, ErrorMessage);
}
