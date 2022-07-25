// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRegisterRemoteServerV2.h"
#include "Engine/Engine.h"

FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::FOnlineAsyncTaskAccelByteRegisterRemoteServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnRegisterServerComplete& InDelegate)
    : FOnlineAsyncTaskAccelByte(InABInterface)
    , SessionName(InSessionName)
    , Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
    ensure(SessionInterface.IsValid());

    int32 RegisterPort = 0;
    if (!SessionInterface->GetServerPort(RegisterPort))
    {
        AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register server to Armada as we failed to get the server's port!"));
        CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
        return;
    }

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteRegisterRemoteServerV2, RegisterServer, FVoidHandler);
	FRegistry::ServerDSM.RegisterServerToDSM(RegisterPort, OnRegisterServerSuccessDelegate, OnRegisterServerErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    if (bWasSuccessful)
    {
		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
		ensure(SessionInterface.IsValid());

        // Check if we have a session ID as an environment variable, if so then we want to start the process of creating
        // a local session for the server based on the backend session data
		const FString SessionId = FPlatformMisc::GetEnvironmentVariable(TEXT("NOMAD_META_session_id"));
        if (!SessionId.IsEmpty())
        {
            SessionInterface->GetServerAssociatedSession(SessionName);
        }
    }

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    Delegate.ExecuteIfBound(bWasSuccessful);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::OnRegisterServerSuccess()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::OnRegisterServerError(int32 ErrorCode, const FString& ErrorMessage)
{
    UE_LOG_AB(Warning, TEXT("Failed to register server with Armada! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
