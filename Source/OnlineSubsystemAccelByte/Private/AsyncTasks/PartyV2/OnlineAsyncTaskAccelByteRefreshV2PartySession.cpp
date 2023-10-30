// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRefreshV2PartySession.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Api/AccelByteSessionApi.h"

FOnlineAsyncTaskAccelByteRefreshV2PartySession::FOnlineAsyncTaskAccelByteRefreshV2PartySession(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnRefreshSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
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
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Could not refresh party session named '%s' as our session interface is invalid!", *SessionName.ToString());

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_ENSURE(Session != nullptr, "Could not refresh party session named '%s' as the session does not exist locally!", *SessionName.ToString());

	const FString SessionId = Session->GetSessionIdStr();
	AB_ASYNC_TASK_ENSURE(!SessionId.Equals(TEXT("InvalidSession")), "Could not refresh party session named '%s' as there is not a valid session ID associated!", *SessionName.ToString());

	OnRefreshPartySessionSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshV2PartySession::OnRefreshPartySessionSuccess);
	OnRefreshPartySessionErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRefreshV2PartySession::OnRefreshPartySessionError);;
	ApiClient->Session.GetPartyDetails(SessionId, OnRefreshPartySessionSuccessDelegate, OnRefreshPartySessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2PartySession::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		SessionInterface->UpdateInternalPartySession(SessionName, RefreshedPartySession);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2PartySession::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful && bWasSessionRemoved)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
		if (ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface)))
		{
			SessionInterface->TriggerOnSessionRemovedDelegates(SessionName);
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
