// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindV2PartyById.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

FOnlineAsyncTaskAccelByteFindV2PartyById::FOnlineAsyncTaskAccelByteFindV2PartyById(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const FUniqueNetId& InSessionId, const FOnSingleSessionResultCompleteDelegate& InDelegate)
    : FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionId(StaticCastSharedRef<const FUniqueNetIdAccelByteResource>(InSessionId.AsShared()))
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InSearchingPlayerId.AsShared());
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId->ToDebugString());

	const THandler<FAccelByteModelsV2PartySession> OnGetPartySessionDetailsSuccessDelegate = THandler<FAccelByteModelsV2PartySession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsSuccess);
	const FErrorHandler OnGetPartySessionDetailsErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsError);
	ApiClient->Session.GetPartyDetails(SessionId->ToString(), OnGetPartySessionDetailsSuccessDelegate, OnGetPartySessionDetailsErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		ensure(SessionInterface.IsValid());

		SessionInterface->ConstructPartySessionFromBackendSessionModel(FoundPartySession, FoundSessionResult.Session);
	}

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, FoundSessionResult);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsSuccess(const FAccelByteModelsV2PartySession& InFoundPartySession)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FoundPartySession = InFoundPartySession;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to find party session with ID '%s'! Error code: %d; Error message: %s"), *SessionId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
