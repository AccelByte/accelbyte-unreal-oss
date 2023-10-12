// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLeaveV2Party.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteLeaveV2Party::FOnlineAsyncTaskAccelByteLeaveV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InSessionId, const FOnLeaveSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionId(InSessionId)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	const FVoidHandler OnLeavePartySuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLeaveV2Party::OnLeavePartySuccess);
	const FErrorHandler OnLeavePartyErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLeaveV2Party::OnLeavePartyError);
	ApiClient->Session.LeaveParty(SessionId, OnLeavePartySuccessDelegate, OnLeavePartyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Regardless of whether we successfully left or not, we still want to destroy the session or remove the restored
	// instance. Session worker on backend should remove us after a timeout even if the call fails.
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to leave party session as our session interface is invalid!");

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
	
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2PartySessionLeftPayload PartySessionLeftPayload{};
		PartySessionLeftPayload.UserId = UserId->GetAccelByteId();
		PartySessionLeftPayload.PartySessionId = SessionId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionLeftPayload>(PartySessionLeftPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV2Party::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for leaving a party session as our session interface is invalid!"));
		return;
	}

	if (bRemovedNamedSession)
	{
		SessionInterface->TriggerOnDestroySessionCompleteDelegates(RemovedSessionName, bWasSuccessful);
	}
	Delegate.ExecuteIfBound(bWasSuccessful, SessionId);

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
