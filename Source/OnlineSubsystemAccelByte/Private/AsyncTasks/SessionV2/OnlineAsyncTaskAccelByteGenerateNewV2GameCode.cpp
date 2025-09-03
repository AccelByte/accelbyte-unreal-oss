// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGenerateNewV2GameCode.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::FOnlineAsyncTaskAccelByteGenerateNewV2GameCode(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InLocalUserId
	, const FName& InSessionName
	, const FOnGenerateNewPartyCodeComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s"), *UserId->ToDebugString(), *SessionName.ToString());

	TRY_PIN_SUBSYSTEM();

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_VALIDATE(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(),  SessionInterface), "Failed to get session interface for generating game code!");

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Failed to get named session for generating game code!");

	const FString SessionId = OnlineSession->GetSessionIdStr();
	AB_ASYNC_TASK_VALIDATE(!SessionId.Equals(TEXT("InvalidSession"), ESearchCase::IgnoreCase), "Named session used to generate new game code has invalid game ID!");

	OnGenerateNewCodeSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2GameSession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::OnGenerateNewCodeSuccess);
	OnGenerateNewCodeErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::OnGenerateNewCodeError);;

	if (IsDS.GetValue())
	{
		SERVER_API_CLIENT_CHECK_GUARD();
		ServerApiClient->ServerSession.GenerateNewGameSessionCode(SessionId, OnGenerateNewCodeSuccessDelegate, OnGenerateNewCodeErrorDelegate);
	}
	else
	{
		API_FULL_CHECK_GUARD(Session);
		Session->GenerateNewGameSessionCode(SessionId, OnGenerateNewCodeSuccessDelegate, OnGenerateNewCodeErrorDelegate);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
		if (!FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(),  SessionInterface))
		{
			return;
		}

		bool ConnectingP2P {false};
		SessionInterface->UpdateInternalGameSession(SessionName, UpdatedGameSession, ConnectingP2P);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, UpdatedGameSession.Code);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::OnGenerateNewCodeSuccess(const FAccelByteModelsV2GameSession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Code: %s"), *Result.Code);

	UpdatedGameSession = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateNewV2GameCode::OnGenerateNewCodeError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to generate new party code for party!", ErrorCode, ErrorMessage);
}
