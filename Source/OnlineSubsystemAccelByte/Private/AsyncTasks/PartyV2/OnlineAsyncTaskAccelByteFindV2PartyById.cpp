// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindV2PartyById.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteFindV2PartyById::FOnlineAsyncTaskAccelByteFindV2PartyById(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const FUniqueNetId& InSessionId, const FOnSingleSessionResultCompleteDelegate& InDelegate)
	// Initialize as a server task if we are running a dedicated server, as this doubles as a server task. Otherwise, use
	// no flags to indicate that it is a client task.	
	: FOnlineAsyncTaskAccelByte(InABInterface
		, INVALID_CONTROLLERID
		, IsRunningDedicatedServer() ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
	, SessionId(FUniqueNetIdAccelByteResource::CastChecked(InSessionId))
	, Delegate(InDelegate)
{
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingPlayerId);
	}
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionId: %s"), *SessionId->ToDebugString());

	OnGetPartySessionDetailsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsSuccess);
	OnGetPartySessionDetailsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteFindV2PartyById::OnGetPartySessionDetailsError);

	if (IsRunningDedicatedServer())
	{
		FServerApiClientPtr ServerApiClient = AccelByte::FMultiRegistry::GetServerApiClient();
		ensure(ServerApiClient.IsValid());

		ServerApiClient->ServerSession.GetPartyDetails(SessionId->ToString()
			, OnGetPartySessionDetailsSuccessDelegate
			, OnGetPartySessionDetailsErrorDelegate);
	}
	else
	{
		ApiClient->Session.GetPartyDetails(SessionId->ToString()
			, OnGetPartySessionDetailsSuccessDelegate
			, OnGetPartySessionDetailsErrorDelegate);
	}

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV2PartyById::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize finding party by ID as our session interface is invalid!"));
			return;
		}

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
