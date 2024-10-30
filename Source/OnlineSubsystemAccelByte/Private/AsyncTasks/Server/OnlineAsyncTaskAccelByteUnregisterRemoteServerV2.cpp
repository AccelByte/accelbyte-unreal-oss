// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUnregisterRemoteServerV2.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnUnregisterServerComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionName(InSessionName)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to unregister remote server as our session interface is invalid!");

	// Attempt to get session ID to send along to DSMC when unregistering server. In case this shutdown call is being
	// made before the server is assigned a session, allow for a blank value.
	FString SessionId = TEXT("");
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session != nullptr && !Session->GetSessionIdStr().Equals("InvalidSession"))
	{
		SessionId = Session->GetSessionIdStr();
	}

	OnUnregisterServerSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::OnUnregisterServerSuccess);
	OnUnregisterServerErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::OnUnregisterServerError);;
	FRegistry::ServerDSM.SendShutdownToDSM(false, SessionId, OnUnregisterServerSuccessDelegate, OnUnregisterServerErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface;
		if (!ensure(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize unregistering remote server as our session interface is invalid!"));
			return;
		}

		SessionInterface->DisconnectFromDSHub();

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsDSUnregisteredPayload DSUnregisteredPayload{};
			DSUnregisteredPayload.PodName = FRegistry::ServerDSM.GetServerName();
			PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSUnregisteredPayload>(DSUnregisteredPayload));
		}
	}

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
