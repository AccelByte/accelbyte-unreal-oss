// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendDSSessionReady.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSendDSReady"

FOnlineAsyncTaskAccelByteSendDSSessionReady::FOnlineAsyncTaskAccelByteSendDSSessionReady(	FOnlineSubsystemAccelByte* const InABInterface, bool bInIsServerReady)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, bIsServerReady(bInIsServerReady)
{
	TRY_PIN_SUBSYSTEM_CONSTRUCTOR()

	FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface);
}

void FOnlineAsyncTaskAccelByteSendDSSessionReady::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		ErrorText = FText::FromString(TEXT("send-ds-ready-not-a-server"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("This task only works for Dedicated Server"));
		return;
	}

	if (!SessionInterface.IsValid())
	{
		ErrorText = FText::FromString(TEXT("send-ds-ready-failed-session-interface-invalid"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);

		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get session interface instance from online subsystem!"));
		return;
	}

	const FNamedOnlineSession* GameSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (GameSession == nullptr)
	{
		ErrorText = FText::FromString(TEXT("send-ds-ready-failed-server-hasn't-claimed-a-session"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);

		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}
	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(GameSession->SessionInfo);
	const FString GameSessionId = SessionInfo.IsValid() ? SessionInfo->GetSessionId().ToString() : TEXT("");

	OnSendDSReadySuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendDSSessionReady::OnSendDSReadySuccess);
	OnSendDSReadyErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendDSSessionReady::OnSendDSReadyError);

	FMultiRegistry::GetServerApiClient()->ServerSession.SendDSSessionReady(GameSessionId, bIsServerReady, OnSendDSReadySuccessDelegate, OnSendDSReadyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendDSSessionReady::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *ErrorText.ToString());

	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnSendDSSessionReadyCompleteDelegates(OnlineError);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to trigger delegate because session interface is invalid"));
}

void FOnlineAsyncTaskAccelByteSendDSSessionReady::OnSendDSReadySuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendDSSessionReady::OnSendDSReadyError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	ErrorText = FText::FromString(TEXT("send-ds-ready-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
