// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#if 1 // MMv1 Deprecation

#include "OnlineAsyncTaskAccelByteEnqueueJoinableV1Session.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameServerApi/AccelByteServerMatchmakingApi.h"

#include "Core/AccelByteError.h"

#include "Core/AccelByteReport.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnEnqueueJoinableSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	FReport::LogDeprecated(FString(__FUNCTION__),
		TEXT("Session V1 functionality is deprecated and replaced by Session V2. For more information, see https://docs.accelbyte.io/gaming-services/services/play/session/"));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
	check(SessionInterface != nullptr);

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	check(Session != nullptr);

	if (!Session->SessionInfo.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot enqueue joinable session as our session info is nullptr!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV1> ABSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
	if (!ABSessionInfo.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot enqueue joinable session as our session info is nullptr!"));
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD();

	FVoidHandler OnEnqueueJoinableSessionSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::OnEnqueueJoinableSessionSuccess);
	FErrorHandler OnEnqueueJoinableSessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::OnEnqueueJoinableSessionError);
	
	ServerApiClient->ServerMatchmaking.EnqueueJoinableSession(ABSessionInfo->GetSessionResult(), OnEnqueueJoinableSessionSuccessDelegate, OnEnqueueJoinableSessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to enqueue joinable session on backend."));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		Session->SessionSettings.bAllowJoinInProgress = true;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::OnEnqueueJoinableSessionSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Successfully enqueued joinable session for '%s' to DSM!"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnqueueJoinableV1Session::OnEnqueueJoinableSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to enqueue joinable session for '%s' to DSM! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
#endif