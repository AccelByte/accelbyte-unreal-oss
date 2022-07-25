// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLeaveV2Party.h"

FOnlineAsyncTaskAccelByteLeaveV2Party::FOnlineAsyncTaskAccelByteLeaveV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InSessionId, const FOnLeaveSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionId(InSessionId)
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	const FVoidHandler OnLeavePartySuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteLeaveV2Party::OnLeavePartySuccess);
	const FErrorHandler OnLeavePartyErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteLeaveV2Party::OnLeavePartyError);
	ApiClient->Session.LeaveParty(SessionId, OnLeavePartySuccessDelegate, OnLeavePartyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Regardless of whether we successfully left or not, we still want to destroy the session or remove the restored
	// instance. Session worker on backend should remove us after a timeout even if the call fails.
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	// Try and find a named session instance from the session interface to remove it
	FNamedOnlineSession* FoundSession = SessionInterface->GetNamedSessionById(SessionId);
	if (FoundSession != nullptr)
	{
		RemovedSessionName = FoundSession->SessionName;
		SessionInterface->RemoveNamedSession(FoundSession->SessionName);
		bRemovedNamedSession = true;
	}
	else
	{
		bRemovedRestoreSession = SessionInterface->RemoveRestoreSessionById(SessionId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	if (bRemovedNamedSession)
	{
		SessionInterface->TriggerOnDestroySessionCompleteDelegates(RemovedSessionName, bWasSuccessful);
	}
	Delegate.ExecuteIfBound(true, SessionId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::OnLeavePartySuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::OnLeavePartyError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to leave party on the backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
