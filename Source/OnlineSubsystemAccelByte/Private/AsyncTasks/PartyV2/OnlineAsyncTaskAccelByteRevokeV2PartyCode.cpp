﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRevokeV2PartyCode.h"

FOnlineAsyncTaskAccelByteRevokeV2PartyCode::FOnlineAsyncTaskAccelByteRevokeV2PartyCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnRevokePartyCodeComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteRevokeV2PartyCode::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s"), *UserId->ToDebugString(), *SessionName.ToString());

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_VALIDATE(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface), "Failed to get session interface for revoking party code!");

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Failed to get named session for revoking party code!");

	const FString SessionId = OnlineSession->GetSessionIdStr();
	AB_ASYNC_TASK_VALIDATE(!SessionId.Equals(TEXT("InvalidSession"), ESearchCase::IgnoreCase), "Named session used to revoke party code has invalid party ID!");

	OnRevokeCodeSuccessDelegate = AccelByte::TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRevokeV2PartyCode::OnRevokeCodeSuccess);
	OnRevokeCodeErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRevokeV2PartyCode::OnRevokeCodeError);;
	API_FULL_CHECK_GUARD(Session);
	Session->RevokePartyCode(SessionId, OnRevokeCodeSuccessDelegate, OnRevokeCodeErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2PartyCode::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
		if (!FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface))
		{
			return;
		}

		// Another sort of oddity, grab backend session data for this party session, update the code and version fields, then
		// call update internal so that we don't have to duplicate update logic
		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
		check(SessionInfo.IsValid());

		TSharedPtr<FAccelByteModelsV2PartySession> PartySessionData = SessionInfo->GetBackendSessionDataAsPartySession();
		check(PartySessionData.IsValid());

		// On revoke, party code should be empty and version should be incremented
		// #TODO: Might want to have backend just return the updated session data from the revoke to avoid this kind of hacky
		// update routine for revoking
		PartySessionData->Code = TEXT("");
		PartySessionData->Version++;

		SessionInterface->UpdateInternalPartySession(SessionName, PartySessionData.ToSharedRef().Get());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2PartyCode::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2PartyCode::OnRevokeCodeSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2PartyCode::OnRevokeCodeError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to revoke code for party session!", ErrorCode, ErrorMessage);
}
