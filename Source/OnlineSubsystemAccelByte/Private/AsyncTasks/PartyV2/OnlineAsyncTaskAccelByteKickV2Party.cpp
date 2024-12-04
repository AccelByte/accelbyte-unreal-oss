// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteKickV2Party.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

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
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToKick: %s"), *UserId->ToDebugString(), *SessionName.ToString(), *PlayerIdToKick->ToDebugString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to kick player from party as our session interface is invalid!");

	FNamedOnlineSession* Session = SessionInterface->GetPartySession();
	AB_ASYNC_TASK_VALIDATE(Session != nullptr, "Failed to kick player from party as we do not have a party session stored locally!");

	OnKickUserFromPartySuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteKickV2Party::OnKickUserFromPartySuccess);
	OnKickUserFromPartyErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteKickV2Party::OnKickUserFromPartyError);
	API_CLIENT_CHECK_GUARD();
	ApiClient->Session.KickUserFromParty(Session->GetSessionIdStr(), PlayerIdToKick->GetAccelByteId(), OnKickUserFromPartySuccessDelegate, OnKickUserFromPartyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV2Party::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize kicking a player from party session as our session interface is invalid!"));
			return;
		}

		SessionInterface->UnregisterPlayer(SessionName, PlayerIdToKick.Get());

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsMPV2PartySessionKickedPayload PartySessionKickedPayload{};
			PartySessionKickedPayload.UserId = UserId->GetAccelByteId();
			PartySessionKickedPayload.PartySessionId = SessionInterface->GetPartySession()->GetSessionIdStr();
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionKickedPayload>(PartySessionKickedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV2Party::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for kicking a player from party session as our session interface is invalid!"));
			return;
		}
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
