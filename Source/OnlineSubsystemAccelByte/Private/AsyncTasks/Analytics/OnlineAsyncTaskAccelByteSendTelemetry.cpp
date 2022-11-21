// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendTelemetry.h"

FOnlineAsyncTaskAccelByteSendTelemetry::FOnlineAsyncTaskAccelByteSendTelemetry(
	FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum,
	FAccelByteModelsTelemetryBody const& TelemetryBody, FVoidHandler const& OnSuccess, FErrorHandler const& OnError)
		:	FOnlineAsyncTaskAccelByte(InABInterface),
			TelemetryBody(TelemetryBody), OnSuccessHandler(OnSuccess), OnErrorHandler(OnError)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteSendTelemetry::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Sending telemetry event, UserId: %s"), *UserId->ToDebugString());

	if (IsRunningDedicatedServer())
	{
		const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		if (ServerApiClient.IsValid())
		{
			ServerApiClient->ServerGameTelemetry.Send(TelemetryBody, OnSuccessHandler, OnErrorHandler);
		}
	}
	else
	{
		ApiClient = GetApiClient(LocalUserNum);
		if (ApiClient.IsValid())
		{
			ApiClient->GameTelemetry.Send(TelemetryBody, OnSuccessHandler, OnErrorHandler);
		}
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
