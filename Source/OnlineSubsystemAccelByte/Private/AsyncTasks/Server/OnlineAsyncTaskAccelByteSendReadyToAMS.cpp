// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendReadyToAMS.h"
#include "OnlineSubsystemAccelByte.h"
#if !AB_USE_V2_SESSIONS
#include "OnlineSessionInterfaceV1AccelByte.h"
#else
#include "OnlineSessionInterfaceV2AccelByte.h"
#endif

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
	
	UnbindDelegates();
	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface;
		if (!ensure(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to ConnectToDSHub after Sending Ready to AMS as our session interface is invalid!"));
			return;
		}

		ServerSettingsPtr ServerSettings = ServerApiClient->ServerSettings;
		if(!ServerSettings.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to ConnectToDSHub after Sending Ready to AMS as our server settings is invalid!"));
			return;
		}

		SessionInterface->ConnectToDSHub(ServerSettings->DSId);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendReadyToAMS::OnAMSConnectSuccess()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

#if !AB_USE_V2_SESSIONS
	// return failed if not using session v2
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to connect to send ready message to AMS! session v1 is not supported."));
	return;
#endif

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (ensure(SessionInterface.IsValid()))
	{
		SessionInterface->SendReadyToAMS();
		SessionInterface->TriggerAccelByteOnConnectAMSCompleteDelegates(bWasSuccessful, ONLINE_ERROR_ACCELBYTE(ErrorStr, bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));
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

#undef ONLINE_ERROR_NAMESPACE