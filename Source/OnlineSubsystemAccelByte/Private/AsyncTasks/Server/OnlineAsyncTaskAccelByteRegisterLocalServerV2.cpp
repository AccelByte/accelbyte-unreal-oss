﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRegisterLocalServerV2.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRegisterLocalServerV2::FOnlineAsyncTaskAccelByteRegisterLocalServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FOnRegisterServerComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteRegisterLocalServerV2::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to register local server as our session interface is invalid!");

	FString LocalServerIp = TEXT("");
	AB_ASYNC_TASK_VALIDATE(SessionInterface->GetServerLocalIp(LocalServerIp), "Failed to register local server to Armada as we failed to get the server's local address!");

	int32 RegisterPort = 0;
	AB_ASYNC_TASK_VALIDATE(SessionInterface->GetServerPort(RegisterPort, true), "Failed to register local server to Armada as we failed to get the server's port!");

	ServerName = TEXT("");
	AB_ASYNC_TASK_VALIDATE(SessionInterface->GetLocalServerName(ServerName), "Failed to register local server to Armada as we failed to get the server's name!");

	SERVER_API_CLIENT_CHECK_GUARD();

	OnRegisterServerSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRegisterLocalServerV2::OnRegisterServerSuccess);
	OnRegisterServerErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRegisterLocalServerV2::OnRegisterServerError);;
	ServerApiClient->ServerDSM.RegisterLocalServerToDSM(LocalServerIp, RegisterPort, ServerName, OnRegisterServerSuccessDelegate, OnRegisterServerErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterLocalServerV2::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface;
		if (!ensure(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize registering local server as our session interface is invalid!"));
			return;
		}

		SessionInterface->ConnectToDSHub(ServerName);

		// #NOTE Deliberately not checking session ID here as there's no way to have a buffer local server. Local servers
		// should always be claimed later through the DS hub.

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsDSRegisteredPayload DSRegisteredPayload{};
			DSRegisteredPayload.PodName = ServerName;
			PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSRegisteredPayload>(DSRegisteredPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterLocalServerV2::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterLocalServerV2::OnRegisterServerSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterLocalServerV2::OnRegisterServerError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to register local server with Armada! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
