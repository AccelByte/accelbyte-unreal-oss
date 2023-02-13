// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteKickV2Party.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteKickV2Party::FOnlineAsyncTaskAccelByteKickV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InPlayerIdToKick, const FOnKickPlayerComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, PlayerIdToKick(FUniqueNetIdAccelByteUser::CastChecked(InPlayerIdToKick))
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteKickV2Party::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToKick: %s"), *UserId->ToDebugString(), *SessionName.ToString(), *PlayerIdToKick->ToDebugString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to kick player from party as our session interface is invalid!");

	FNamedOnlineSession* Session = SessionInterface->GetPartySession();
	AB_ASYNC_TASK_ENSURE(Session != nullptr, "Failed to kick player from party as we do not have a party session stored locally!");

	const THandler<FAccelByteModelsV2PartySession> OnKickUserFromPartySuccessDelegate = THandler<FAccelByteModelsV2PartySession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteKickV2Party::OnKickUserFromPartySuccess);
	const FErrorHandler OnKickUserFromPartyErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteKickV2Party::OnKickUserFromPartyError);
	ApiClient->Session.KickUserFromParty(Session->GetSessionIdStr(), PlayerIdToKick->GetAccelByteId(), OnKickUserFromPartySuccessDelegate, OnKickUserFromPartyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV2Party::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize kicking a player from party session as our session interface is invalid!"));
			return;
		}

		SessionInterface->UnregisterPlayer(SessionName, PlayerIdToKick.Get());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV2Party::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for kicking a player from party session as our session interface is invalid!"));
			return;
		}

		SessionInterface->TriggerOnSessionParticipantsChangeDelegates(SessionName, PlayerIdToKick.Get(), false);
#if ENGINE_MAJOR_VERSION >= 5 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27)
		SessionInterface->TriggerOnSessionParticipantRemovedDelegates(SessionName, PlayerIdToKick.Get());
#endif
	}

	Delegate.ExecuteIfBound(bWasSuccessful, PlayerIdToKick.Get());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV2Party::OnKickUserFromPartySuccess(const FAccelByteModelsV2PartySession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UpdatedBackendSessionData = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV2Party::OnKickUserFromPartyError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to kick user from party session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
