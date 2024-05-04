// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineTaskAccelByteRevokeV2GameCode.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRevokeV2GameCode::FOnlineAsyncTaskAccelByteRevokeV2GameCode(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnRevokeGameCodeComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteRevokeV2GameCode::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s"), *UserId->ToDebugString(), *SessionName.ToString());

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_ENSURE(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface), "Failed to get session interface for revoking game code!");

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_ENSURE(Session != nullptr, "Failed to get named session for revoking game code!");

	const FString SessionId = Session->GetSessionIdStr();
	AB_ASYNC_TASK_ENSURE(!SessionId.Equals(TEXT("InvalidSession"), ESearchCase::IgnoreCase), "Named session used to revoke game code has invalid session ID!");

	OnRevokeCodeSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRevokeV2GameCode::OnRevokeCodeSuccess);
	OnRevokeCodeErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRevokeV2GameCode::OnRevokeCodeError);;

	if(IsRunningDedicatedServer())
	{
		FMultiRegistry::GetServerApiClient()->ServerSession.RevokeGameSessionCode(SessionId, OnRevokeCodeSuccessDelegate, OnRevokeCodeErrorDelegate);
	}
	else
	{
		API_CLIENT_CHECK_GUARD();
		ApiClient->Session.RevokeGameSessionCode(SessionId, OnRevokeCodeSuccessDelegate, OnRevokeCodeErrorDelegate);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2GameCode::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
		if (!FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface))
		{
			return;
		}

		// Another sort of oddity, grab backend session data for this party session, update the code and version fields, then
		// call update internal so that we don't have to duplicate update logic
		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
		check(SessionInfo.IsValid());

		TSharedPtr<FAccelByteModelsV2GameSession> GameSessionData = SessionInfo->GetBackendSessionDataAsGameSession();
		check(GameSessionData.IsValid());

		// On revoke, party code should be empty and version should be incremented
		// #TODO: Might want to have backend just return the updated session data from the revoke to avoid this kind of hacky
		// update routine for revoking
		GameSessionData->Code = TEXT("");
		GameSessionData->Version++;

		bool bIsConnectingP2P {false};
		SessionInterface->UpdateInternalGameSession(SessionName, GameSessionData.ToSharedRef().Get(), bIsConnectingP2P);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2GameCode::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2GameCode::OnRevokeCodeSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRevokeV2GameCode::OnRevokeCodeError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to revoke code for party session!", ErrorCode, ErrorMessage);
}
