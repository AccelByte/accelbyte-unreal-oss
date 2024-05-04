// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRejectV2PartyInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteRejectV2PartyInvite::FOnlineAsyncTaskAccelByteRejectV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlineSessionSearchResult& InInvitedSession, const FOnRejectSessionInviteComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, InvitedSession(InInvitedSession)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s"), *UserId->GetAccelByteId(), *InvitedSession.GetSessionIdStr());

	const FString SessionId = InvitedSession.GetSessionIdStr();
	AB_ASYNC_TASK_ENSURE(!SessionId.Equals(TEXT("InvalidSession")), "Failed to reject party session invite as the session ID provided is not valid!");

	OnRejectPartyInviteSuccessDelegate = AccelByte::TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRejectV2PartyInvite::OnRejectPartyInviteSuccess);
	OnRejectPartyInviteErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRejectV2PartyInvite::OnRejectPartyInviteError);;
	API_CLIENT_CHECK_GUARD();
	ApiClient->Session.RejectPartyInvite(SessionId, OnRejectPartyInviteSuccessDelegate, OnRejectPartyInviteErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize rejecting a party invite as our session interface is invalid!"));
			return;
		}

		SessionInterface->RemoveInviteById(InvitedSession.GetSessionIdStr());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectV2PartyInvite::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

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
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to reject party session invite as the request failed on the backend!", ErrorCode, ErrorMessage);
}
