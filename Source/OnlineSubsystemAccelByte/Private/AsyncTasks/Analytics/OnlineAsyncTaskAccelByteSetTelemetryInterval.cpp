// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSetTelemetryInterval.h"

FOnlineAsyncTaskAccelByteSetTelemetryInterval::FOnlineAsyncTaskAccelByteSetTelemetryInterval(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, int32 IntervalInSeconds)
	: FOnlineAsyncTaskAccelByte(InABInterface), IntervalInSeconds(IntervalInSeconds)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteSetTelemetryInterval::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Setting telemetry send interval, LocalUserNum: %d"), LocalUserNum);

	if (IsRunningDedicatedServer())
	{
		const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		if (ServerApiClient.IsValid())
		{
			ServerApiClient->ServerGameTelemetry.SetBatchFrequency(FTimespan::FromSeconds(IntervalInSeconds));
		}
	}
	else
	{
		ApiClient = GetApiClient(LocalUserNum);
		if (ApiClient.IsValid())
		{
			ApiClient->GameTelemetry.SetBatchFrequency(FTimespan::FromSeconds(IntervalInSeconds));
		}
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}
