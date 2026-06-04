// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendReadyToAMS.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSendReadyToAMS"

FOnlineAsyncTaskAccelByteSendReadyToAMS::FOnlineAsyncTaskAccelByteSendReadyToAMS(FOnlineSubsystemAccelByte* const InABInterface, const FOnRegisterServerComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	
	SERVER_API_CLIENT_CHECK_GUARD();
	
	// If already connected then directly send ready message
	if (ServerApiClient->ServerAMS.IsConnected())
	{
		OnAMSConnectSuccess();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	// Create delegates for successfully as well as unsuccessfully connecting to the AccelByte lobby websocket
	OnAMSConnectSuccessDelegate = TDelegateUtils<AccelByte::GameServerApi::ServerAMS::FConnectSuccess>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectSuccess);
	OnAMSConnectErrorDelegate = TDelegateUtils<AccelByte::GameServerApi::ServerAMS::FConnectError>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectError);

	OnAMSConnectionClosedDelegate = TDelegateUtils<AccelByte::GameServerApi::ServerAMS::FConnectionClosed>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectionClosed);
	ServerApiClient->ServerAMS.SetOnConnectionClosed(OnAMSConnectionClosedDelegate);

	// Send off a request to connect to the lobby websocket, as well as connect our delegates for doing so
	ServerApiClient->ServerAMS.SetOnConnectSuccess(OnAMSConnectSuccessDelegate);
	ServerApiClient->ServerAMS.SetOnConnectError(OnAMSConnectErrorDelegate);

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
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	SERVER_API_CLIENT_CHECK_GUARD();

	// DSHub was connected (and session delegates restored) inside OnDSHubConnectSuccess before
	// SendReadyToAMS was called. No ConnectToDSHub call needed here.
	UnbindDelegates();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectSuccess()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (ensure(SessionInterface.IsValid()))
	{
		// Notify game code that AMS is connected. SendReadyToAMS is deferred until DSHub is connected.
		SessionInterface->TriggerAccelByteOnConnectAMSCompleteDelegates(bWasSuccessful, ONLINE_ERROR_ACCELBYTE(ErrorStr, bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));
	}

	// Connect DSHub before signalling READY to AMS. 
	// SendReadyToAMS() is called from OnDSHubConnectSuccess() once the connection is confirmed.
	SERVER_API_CLIENT_CHECK_GUARD();

	ServerSettingsPtr ServerSettings = ServerApiClient->ServerSettings;
	if (!ServerSettings.IsValid() || ServerSettings->DSId.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot connect to DSHub: DSId is empty or server settings are invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	if (ServerApiClient->ServerDSHub.IsConnected())
	{
		// DSHub already connected — restore session delegates and send READY immediately.
		OnDSHubConnectSuccess();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("DSHub already connected, proceeding to send server READY"));
		return;
	}

	OnDSHubConnectSuccessDelegate = TDelegateUtils<AccelByte::GameServerApi::FConnectSuccess>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnDSHubConnectSuccess);
	OnDSHubConnectErrorDelegate   = TDelegateUtils<AccelByte::GameServerApi::FConnectError>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendReadyToAMS::OnDSHubConnectError);
	ServerApiClient->ServerDSHub.SetOnConnectSuccess(OnDSHubConnectSuccessDelegate);
	ServerApiClient->ServerDSHub.SetOnConnectError(OnDSHubConnectErrorDelegate);
	ServerApiClient->ServerDSHub.Connect(ServerSettings->DSId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Connecting to DSHub before sending server READY to AMS"));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::OnDSHubConnectSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	TRY_PIN_SUBSYSTEM();
	SERVER_API_CLIENT_CHECK_GUARD();

	FOnlineSessionV2AccelBytePtr SessionInterface;
	if (!ensure(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send server READY after DSHub connect: session interface is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	ServerSettingsPtr ServerSettings = ServerApiClient->ServerSettings;
	if (!ServerSettings.IsValid() || ServerSettings->DSId.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send server READY after DSHub connect: DSId is empty or server settings are invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Restore all session-level delegates on DSHub: notification delegates (serverClaimed, backfill,
	// session members, etc.), OnConnectSuccess → OnDSHubConnectSuccessNotification for future reconnects,
	// and reconnect/outage.
	// The Connect() call inside ConnectToDSHub is a no-op since the WebSocket is already established.
	SessionInterface->ConnectToDSHub(ServerSettings->DSId);

	// DSHub WebSocket is fully established and all notification delegates are bound.
	// It is now safe to signal READY to AMS.
	SessionInterface->SendReadyToAMS();
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::OnDSHubConnectError(const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UE_LOG_AB(Warning, TEXT("Failed to connect to DSHub during server ready flow. Error: %s"), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
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
	OnDSHubConnectSuccessDelegate.Unbind();
	OnDSHubConnectErrorDelegate.Unbind();
}

#undef ONLINE_ERROR_NAMESPACE