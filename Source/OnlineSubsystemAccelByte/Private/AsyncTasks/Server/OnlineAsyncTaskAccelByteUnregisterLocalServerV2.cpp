// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUnregisterLocalServerV2.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::FOnlineAsyncTaskAccelByteUnregisterLocalServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FOnUnregisterServerComplete& InDelegate)
    : FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
    , Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::Initialize()
{
	TRY_PIN_SUBSYSTEM();

    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to unregister local server as our session interface is invalid!");

	ServerName = TEXT("");
	AB_ASYNC_TASK_VALIDATE(SessionInterface->GetLocalServerName(ServerName), "Failed to unregister local server as we failed to get the name of the server!");

	SERVER_API_CLIENT_CHECK_GUARD();

	OnUnregisterServerSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::OnUnregisterServerSuccess);
	OnUnregisterServerErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::OnUnregisterServerError);;
	ServerApiClient->ServerDSM.DeregisterLocalServerFromDSM(ServerName, OnUnregisterServerSuccessDelegate, OnUnregisterServerErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface;
		if (!ensure(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface)))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize unregistering local server as our session interface is invalid!"));
			return;
		}

		SessionInterface->DisconnectFromDSHub();

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsDSUnregisteredPayload DSUnregisteredPayload{};
			DSUnregisteredPayload.PodName = ServerName;
			PredefinedEventInterface->SendEvent(-1, MakeShared<FAccelByteModelsDSUnregisteredPayload>(DSUnregisteredPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::OnUnregisterServerSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnregisterLocalServerV2::OnUnregisterServerError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to unregister local server from Armada! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
