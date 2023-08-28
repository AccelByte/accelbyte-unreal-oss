// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendReadyToAMS.h"
#include "OnlineSubsystemAccelByte.h"
#if AB_USE_V2_SESSIONS
#include "OnlineSessionInterfaceV2AccelByte.h"
#else
#include "OnlineSessionInterfaceV1AccelByte.h"
#endif

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSendReadyToAMS::FOnlineAsyncTaskAccelByteSendReadyToAMS(FOnlineSubsystemAccelByte* const InABInterface, const FOnRegisterServerComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	
	// If already connected then directly send ready message
	if (FRegistry::ServerAMS.IsConnected())
	{
		OnAMSConnectSuccess();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	// Create delegates for successfully as well as unsuccessfully connecting to the AccelByte lobby websocket
	OnAMSConnectSuccessDelegate = TDelegateUtils<AccelByte::GameServerApi::ServerAMS::FConnectSuccess>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectSuccess);
	OnAMSConnectErrorDelegate = TDelegateUtils<AccelByte::GameServerApi::ServerAMS::FConnectError>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectError);

	OnAMSConnectionClosedDelegate = TDelegateUtils<AccelByte::GameServerApi::ServerAMS::FConnectionClosed>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectionClosed);
	FRegistry::ServerAMS.SetOnConnectionClosed(OnAMSConnectionClosedDelegate);

	// Send off a request to connect to the lobby websocket, as well as connect our delegates for doing so
	FRegistry::ServerAMS.SetOnConnectSuccess(OnAMSConnectSuccessDelegate);
	FRegistry::ServerAMS.SetOnConnectError(OnAMSConnectErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	UnbindDelegates();
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

#if !AB_USE_V2_SESSIONS
	// return failed if not using session v2
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to connect to send ready message to AMS! session v1 is not supported."));
	return;
#endif

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (ensure(SessionInterface.IsValid()))
	{
		SessionInterface->SendReadyToAMS();
	}
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectError(const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to connect to the AMS websocket! Error Message: %s"), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectionClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG_AB(Warning, TEXT("AMS disconnected. StatusCode: '%d' Reason: '%s'"), StatusCode, *Reason);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::UnbindDelegates()
{
	OnAMSConnectSuccessDelegate.Unbind();
	OnAMSConnectErrorDelegate.Unbind();
	OnAMSConnectionClosedDelegate.Unbind();
}
