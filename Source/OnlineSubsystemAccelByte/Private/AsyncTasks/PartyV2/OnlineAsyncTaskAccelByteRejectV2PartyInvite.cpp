// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRejectV2PartyInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteRejectV2PartyInvite::FOnlineAsyncTaskAccelByteRejectV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlineSessionSearchResult& InInvitedSession)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, InvitedSession(InInvitedSession)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s"), *UserId->GetAccelByteId(), *InvitedSession.GetSessionIdStr());

	const FVoidHandler OnRejectPartyInviteSuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteRejectV2PartyInvite::OnRejectPartyInviteSuccess);
	const FErrorHandler OnRejectPartyInviteErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteRejectV2PartyInvite::OnRejectPartyInviteError);
	ApiClient->Session.RejectPartyInvite(InvitedSession.GetSessionIdStr(), OnRejectPartyInviteSuccessDelegate, OnRejectPartyInviteErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		ensure(SessionInterface.IsValid());

		SessionInterface->RemoveInviteById(InvitedSession.GetSessionIdStr());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::OnRejectPartyInviteSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::OnRejectPartyInviteError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to reject party session invite as the request failed on the backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
