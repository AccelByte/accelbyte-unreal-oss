// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdatePartyV2.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

FOnlineAsyncTaskAccelByteUpdatePartyV2::FOnlineAsyncTaskAccelByteUpdatePartyV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
{
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	ensure(SessionInterface.IsValid());

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	ensure(Session != nullptr);

	UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(Session->OwningUserId);
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	ensure(Session != nullptr);

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	ensure(SessionInfo.IsValid());

	TSharedPtr<FAccelByteModelsV2PartySession> PartySessionBackendData = StaticCastSharedPtr<FAccelByteModelsV2PartySession>(SessionInfo->GetBackendSessionData());
	ensure(PartySessionBackendData.IsValid());

	FAccelByteModelsV2PartyUpdateRequest UpdateRequest;

	// Set version for update to be the current version on backend
	UpdateRequest.Version = PartySessionBackendData->Version;

	// Currently we just want to update our attributes based on the new settings object passed in
	UpdateRequest.Attributes.JsonObject = SessionInterface->ConvertSessionSettingsToJsonObject(NewSessionSettings);
	
	// Check if joinability has changed and if so send it along to the backend
	FString JoinTypeString;
	NewSessionSettings.Get(SETTING_SESSION_JOIN_TYPE, JoinTypeString);
	
	EAccelByteV2SessionJoinability JoinType = SessionInterface->GetJoinabilityFromString(JoinTypeString);
	if (JoinType != PartySessionBackendData->JoinType)
	{
		UpdateRequest.JoinType = JoinType;
	}

	const THandler<FAccelByteModelsV2PartySession> OnUpdatePartySessionSuccessDelegate = THandler<FAccelByteModelsV2PartySession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionSuccess);
	const FErrorHandler OnUpdatePartySessionErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionError);
	ApiClient->Session.UpdateParty(Session->GetSessionIdStr(), UpdateRequest, OnUpdatePartySessionSuccessDelegate, OnUpdatePartySessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		ensure(SessionInterface.IsValid());

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		ensure(Session != nullptr);

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
		ensure(SessionInfo.IsValid());

		SessionInfo->SetBackendSessionData(MakeShared<FAccelByteModelsV2PartySession>(NewSessionData));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionSuccess(const FAccelByteModelsV2PartySession& BackendSessionData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UpdatedSessionVersion: %d"), BackendSessionData.Version);

	NewSessionData = BackendSessionData;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartyV2::OnUpdatePartySessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to update party session on backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
