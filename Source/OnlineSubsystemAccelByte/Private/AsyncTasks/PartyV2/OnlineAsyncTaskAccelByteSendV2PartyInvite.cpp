// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendV2PartyInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteSendV2PartyInvite::FOnlineAsyncTaskAccelByteSendV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InRecipientId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, RecipientId(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InRecipientId.AsShared()))
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; Recipient ID: %s"), *UserId->ToString(), *RecipientId->GetAccelByteId());

	// First, check if the player is currently in a party session of given SessionName, if we're not, then we shouldn't do this
	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	ensure(Session != nullptr);

	// Now, once we know we are in this party, we want to send a request to invite the player to the party
	const FVoidHandler OnSendPartyInviteSuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteSendV2PartyInvite::OnSendPartyInviteSuccess);
	const FErrorHandler OnSendPartyInviteErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteSendV2PartyInvite::OnSendPartyInviteError);
	ApiClient->Session.SendPartyInvite(Session->GetSessionIdStr(), RecipientId->GetAccelByteId(), OnSendPartyInviteSuccessDelegate, OnSendPartyInviteErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV2PartyInvite::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

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