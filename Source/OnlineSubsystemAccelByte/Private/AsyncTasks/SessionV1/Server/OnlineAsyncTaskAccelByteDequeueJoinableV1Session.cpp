// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#if 1 // MMv1 Deprecation

#include "OnlineAsyncTaskAccelByteDequeueJoinableV1Session.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "GameServerApi/AccelByteServerMatchmakingApi.h"

#include "Core/AccelByteError.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Core/AccelByteReport.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::FOnlineAsyncTaskAccelByteDequeueJoinableV1Session(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnDequeueJoinableSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
	FReport::LogDeprecated(FString(__FUNCTION__),
		TEXT("Session V1 functionality is deprecated and replaced by Session V2. For more information, see https://docs.accelbyte.io/gaming-services/services/play/session/"));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::Initialize()
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
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot dequeue joinable session as our session info is nullptr!"));
		return;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV1> ABSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
	if (!ABSessionInfo.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot dequeue joinable session as our session info is nullptr!"));
		return;
	}

	SERVER_API_CLIENT_CHECK_GUARD();

	FVoidHandler OnDequeueJoinableSessionSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::OnDequeueJoinableSessionSuccess);
	FErrorHandler OnDequeueJoinableSessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::OnDequeueJoinableSessionError);
	ServerApiClient->ServerMatchmaking.DequeueJoinableSession(ABSessionInfo->GetSessionId().ToString(), OnDequeueJoinableSessionSuccessDelegate, OnDequeueJoinableSessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to dequeue joinable session on backend."));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
		check(SessionInterface != nullptr);

		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		check(Session != nullptr);

		Session->SessionSettings.bAllowJoinInProgress = false;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::OnDequeueJoinableSessionSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Successfully dequeued joinable session for '%s' on DSM!"), *SessionName.ToString());

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDequeueJoinableV1Session::OnDequeueJoinableSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to dequeue joinable session for '%s' on DSM! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
#endif