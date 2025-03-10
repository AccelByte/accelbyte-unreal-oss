﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
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
    TRY_PIN_SUBSYSTEM();

    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    SERVER_API_CLIENT_CHECK_GUARD();

    const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
    AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to register remote server as our session interface is invalid!");

    int32 RegisterPort = 0;
    AB_ASYNC_TASK_VALIDATE(SessionInterface->GetServerPort(RegisterPort), "Failed to register server to Armada as we failed to get the server's port!");

	OnRegisterServerSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsServerInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::OnRegisterServerSuccess);
    OnRegisterServerErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::OnRegisterServerError);;
	ServerApiClient->ServerDSM.RegisterServerToDSM(RegisterPort, OnRegisterServerSuccessDelegate, OnRegisterServerErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::Finalize()
{
    TRY_PIN_SUBSYSTEM();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    if (bWasSuccessful)
    {
		FOnlineSessionV2AccelBytePtr SessionInterface;
        if (!ensure(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
        {
            AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize registering remote server as our session interface is invalid!"));
            return;
        }

		const FString PodName = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME"));
        SessionInterface->ConnectToDSHub(PodName);

        // Check if we have a session ID as an environment variable, if so then we want to start the process of creating
        // a local session for the server based on the backend session data
		const FString SessionId = RegisterResult.Session_id;
        if (!SessionId.IsEmpty())
        {
            SessionInterface->GetServerClaimedSession(SessionName, SessionId);
        }

        const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
        if (PredefinedEventInterface.IsValid())
        {
            SERVER_API_CLIENT_CHECK_GUARD();
            FAccelByteModelsDSRegisteredPayload DSRegisteredPayload{};
            DSRegisteredPayload.PodName = ServerApiClient->ServerDSM.GetServerName();
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

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::OnRegisterServerSuccess(const FAccelByteModelsServerInfo& Result)
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    RegisterResult = Result;
    CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterRemoteServerV2::OnRegisterServerError(int32 ErrorCode, const FString& ErrorMessage)
{
    UE_LOG_AB(Warning, TEXT("Failed to register server with Armada! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
