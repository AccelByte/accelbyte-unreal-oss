// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateDSInformation.h"
#include "Models/AccelByteSessionModels.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteUpdateDSInformation"

FOnlineAsyncTaskAccelByteUpdateDSInformation::FOnlineAsyncTaskAccelByteUpdateDSInformation(FOnlineSubsystemAccelByte* const InABInterface
	, FName InSessionName
	, FAccelByteModelsGameSessionUpdateDSInformationRequest const& InNewDSInformation
	, FOnUpdateDSInformationComplete const& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface
		, INVALID_CONTROLLERID
		, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionName(InSessionName)
	, NewDSInformation(InNewDSInformation)
	, CompletionDelegate(InCompletionDelegate)
{
}

void FOnlineAsyncTaskAccelByteUpdateDSInformation::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	TRY_PIN_SUBSYSTEM();

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet() || !IsDS.GetValue())
	{
		const FText ErrorText = FText::FromString(TEXT("update-ds-information-not-a-server"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Updating DS information failed: caller must be a dedicated server"));
		return;
	}

	FOnlineSessionV2AccelBytePtr SessionInterface{};
	if (!FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface))
	{
		const FText ErrorText = FText::FromString(TEXT("update-ds-information-session-interface-invalid"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Updating DS information failed: session interface was invalid"));
		return;
	}

	const FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		const FText ErrorText = FText::FromString(TEXT("update-ds-information-no-session-exists"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Updating DS information failed: session '%s' does not exist locally"), *SessionName.ToString());
		return;
	}

	const FString SessionId = Session->GetSessionIdStr();
	if (SessionId.Equals(TEXT("InvalidSession")))
	{
		const FText ErrorText = FText::FromString(TEXT("update-ds-information-session-invalid-id"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Updating DS information failed: session '%s' has no valid ID"), *SessionName.ToString());
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD();

	const FVoidHandler OnUpdateDSInformationSuccessDelegate =
		TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateDSInformation::OnUpdateDSInformationComplete);
	const FErrorHandler OnUpdateDSInformationErrorDelegate =
		TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateDSInformation::OnUpdateDSInformationError);

	ServerApiClient->ServerSession.UpdateDSInformation(SessionId
		, NewDSInformation
		, OnUpdateDSInformationSuccessDelegate
		, OnUpdateDSInformationErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateDSInformation::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	CompletionDelegate.ExecuteIfBound(SessionName, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateDSInformation::OnUpdateDSInformationComplete()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateDSInformation::OnUpdateDSInformationError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FText ErrorText = FText::FromString(TEXT("update-ds-information-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
