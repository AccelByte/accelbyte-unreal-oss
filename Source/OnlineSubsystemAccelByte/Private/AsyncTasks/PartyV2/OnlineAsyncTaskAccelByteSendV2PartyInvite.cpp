// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendV2PartyInvite.h"
#include "OnlineSubsystemAccelByte.h"

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteSendV2PartyInvite::FOnlineAsyncTaskAccelByteSendV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InRecipientId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, RecipientId(FUniqueNetIdAccelByteUser::CastChecked(InRecipientId))
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; Recipient ID: %s"), *UserId->ToString(), *RecipientId->GetAccelByteId());

	// First, check if the player is currently in a party session of given SessionName, if we're not, then we shouldn't do this
	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to send invite to party session as our session interface is invalid!");
	check(SessionInterface.IsValid());

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Failed to send invite to party session as our local session instance is invalid!");

	// Now, once we know we are in this party, we want to send a request to invite the player to the party
	OnSendPartyInviteSuccessDelegate = AccelByte::TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendV2PartyInvite::OnSendPartyInviteSuccess);
	OnSendPartyInviteErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendV2PartyInvite::OnSendPartyInviteError);;
	API_FULL_CHECK_GUARD(Session);
	Session->SendPartyInvite(OnlineSession->GetSessionIdStr(), RecipientId->GetAccelByteId(), OnSendPartyInviteSuccessDelegate, OnSendPartyInviteErrorDelegate);

	SessionId = OnlineSession->GetSessionIdStr();
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2PartySessionInvitedPayload PartySessionInvitedPayload{};
		PartySessionInvitedPayload.UserId = UserId->GetAccelByteId();
		PartySessionInvitedPayload.PartySessionId = SessionId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionInvitedPayload>(PartySessionInvitedPayload));
	}
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	if (!ensureAlways(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session interface instance from online subsystem!"));
		return;
	}

	SessionInterface->TriggerOnSendSessionInviteCompleteDelegates(UserId.ToSharedRef().Get(), SessionName, bWasSuccessful, RecipientId.Get());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::OnSendPartyInviteSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::OnSendPartyInviteError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to invite user '%s' to party as the request failed on the backend! Error code: %d; Error message: %s"), *RecipientId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}