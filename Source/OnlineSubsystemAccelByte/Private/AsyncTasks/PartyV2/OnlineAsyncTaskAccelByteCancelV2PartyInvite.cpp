// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCancelV2PartyInvite.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteCancelV2PartyInvite"

FOnlineAsyncTaskAccelByteCancelV2PartyInvite::FOnlineAsyncTaskAccelByteCancelV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InInvitee)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, InviteeId(FUniqueNetIdAccelByteUser::CastChecked(InInvitee))
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteCancelV2PartyInvite::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Canceling %s session invite, local user id %s, invitee id %s"), *SessionName.ToString(), *UserId->ToDebugString(), *InviteeId->ToDebugString())

	Super::Initialize();

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!SessionInterface.IsValid())
	{
		ErrorText = FText::FromString(TEXT("cancel-party-invite-session-interface-invalid"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initialize async task as our SessionInterface is invalid"))
		return;
	}

	const FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	if (OnlineSession == nullptr)
	{
		ErrorText = FText::FromString(TEXT("cancel-party-invite-user-not-in-party"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Current user is not in %s session"), *SessionName.ToString())
		return;
	}

	const FString PartyID = OnlineSession->GetSessionIdStr();

	API_FULL_CHECK_GUARD(Session)
	const FVoidHandler OnCancelPartyInviteSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCancelV2PartyInvite::OnCancelPartyInviteSuccess);
	const FErrorHandler OnCancelPartyInviteErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCancelV2PartyInvite::OnCancelPartyInviteError);
	Session->CancelPartyInvitation(PartyID, InviteeId->GetAccelByteId(), OnCancelPartyInviteSuccessDelegate, OnCancelPartyInviteErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteCancelV2PartyInvite::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""))

	Super::TriggerDelegates();

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!SessionInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates as our SessionInterface is invalid"))
		return;
	}

	SessionInterface->TriggerOnCancelSessionInviteCompleteDelegates(*UserId, SessionName, *InviteeId, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteCancelV2PartyInvite::OnCancelPartyInviteSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""))
	OnlineError = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, FString(), EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteCancelV2PartyInvite::OnCancelPartyInviteError(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed canceling party invite, error code %d, %s"), ErrorCode, *ErrorMessage)

	ErrorText = FText::FromString(TEXT("cancel-party-invite-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorMessage, ErrorText);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT(""))
}

#undef ONLINE_ERROR_NAMESPACE
