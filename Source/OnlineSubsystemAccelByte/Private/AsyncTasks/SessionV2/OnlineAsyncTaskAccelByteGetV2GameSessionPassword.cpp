// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetV2GameSessionPassword.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

FOnlineAsyncTaskAccelByteGetV2GameSessionPassword::FOnlineAsyncTaskAccelByteGetV2GameSessionPassword(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnGetGameSessionPasswordComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGetV2GameSessionPassword::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionName: %s"), *UserId->ToDebugString(), *SessionName.ToString());

	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	AB_ASYNC_TASK_VALIDATE(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface), "Failed to get session interface for getting game session password!");

	FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Failed to get named session for getting game session password!");

	const FString SessionId = OnlineSession->GetSessionIdStr();
	AB_ASYNC_TASK_VALIDATE(!SessionId.Equals(TEXT("InvalidSession"), ESearchCase::IgnoreCase), "Named session used to get game session password has invalid session ID!");

	OnGetPasswordSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2SessionPasswordResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetV2GameSessionPassword::OnGetPasswordSuccess);
	OnGetPasswordErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetV2GameSessionPassword::OnGetPasswordError);
	API_FULL_CHECK_GUARD(Session);
	Session->GetGameSessionPassword(SessionId, OnGetPasswordSuccessDelegate, OnGetPasswordErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV2GameSessionPassword::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, PasswordResponse.Password);
	// Clear the in-memory plaintext password once it has been handed off to the delegate, to reduce the
	// window it stays resident in process memory.
	PasswordResponse.Password.Empty();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV2GameSessionPassword::OnGetPasswordSuccess(const FAccelByteModelsV2SessionPasswordResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	PasswordResponse = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV2GameSessionPassword::OnGetPasswordError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to get game session password!", ErrorCode, ErrorMessage);
}
