// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRegisterRemoteServerV2.h"
#include "Engine/Engine.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::FOnlineAsyncTaskAccelByteRegisterRemoteServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnRegisterServerComplete& InDelegate)
    : FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
    , SessionName(InSessionName)
    , Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
    AB_ASYNC_TASK_ENSURE(SessionInterface.IsValid(), "Failed to register remote server as our session interface is invalid!");

    int32 RegisterPort = 0;
    AB_ASYNC_TASK_ENSURE(SessionInterface->GetServerPort(RegisterPort), "Failed to register server to Armada as we failed to get the server's port!");

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteRegisterRemoteServerV2, RegisterServer, FVoidHandler);
	FRegistry::ServerDSM.RegisterServerToDSM(RegisterPort, OnRegisterServerSuccessDelegate, OnRegisterServerErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    if (bWasSuccessful)
    {
		FOnlineSessionV2AccelBytePtr SessionInterface;
        if (!ensure(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface)))
        {
            AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize registering remote server as our session interface is invalid!"));
            return;
        }

		const FString PodName = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME"));
        SessionInterface->ConnectToDSHub(PodName);

        // Check if we have a session ID as an environment variable, if so then we want to start the process of creating
        // a local session for the server based on the backend session data
		const FString SessionId = FPlatformMisc::GetEnvironmentVariable(TEXT("NOMAD_META_session_id"));
        if (!SessionId.IsEmpty())
        {
            SessionInterface->GetServerClaimedSession(SessionName, SessionId);
        }

        const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
        if (PredefinedEventInterface.IsValid())
        {
            FAccelByteModelsDSRegisteredPayload DSRegisteredPayload{};
            DSRegisteredPayload.PodName = FRegistry::ServerDSM.GetServerName();
            PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSRegisteredPayload>(DSRegisteredPayload));
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
