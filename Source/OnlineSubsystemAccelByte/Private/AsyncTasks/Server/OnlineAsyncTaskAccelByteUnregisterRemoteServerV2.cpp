// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUnregisterRemoteServerV2.h"

FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnUnregisterServerComplete& InDelegate)
    : FOnlineAsyncTaskAccelByte(InABInterface, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
    , SessionName(InSessionName)
    , Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    ensure(SessionInterface.IsValid());

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to unregister remote server from Armada DSM as a session with name '%s' does not exist locally!"), *SessionName.ToString());
        CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
        return;
    }

    AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2, UnregisterServer, FVoidHandler);
	FRegistry::ServerDSM.SendShutdownToDSM(false, Session->GetSessionIdStr(), OnUnregisterServerSuccessDelegate, OnUnregisterServerErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    Delegate.ExecuteIfBound(bWasSuccessful);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::OnUnregisterServerSuccess()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::OnUnregisterServerError(int32 ErrorCode, const FString& ErrorMessage)
{
    UE_LOG_AB(Warning, TEXT("Failed to unregister cloud server from Armada! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
