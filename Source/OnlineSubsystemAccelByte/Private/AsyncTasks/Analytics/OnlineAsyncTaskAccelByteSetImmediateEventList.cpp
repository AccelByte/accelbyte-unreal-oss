// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSetImmediateEventList.h"

FOnlineAsyncTaskAccelByteSetImmediateEventList::FOnlineAsyncTaskAccelByteSetImmediateEventList(
	FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, TArray<FString> const& EventNames)
	: FOnlineAsyncTaskAccelByte(InABInterface), ImmediateEventList(EventNames)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteSetImmediateEventList::Initialize()
{
	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Setting immediate event list, LocalUserNum: %d"), LocalUserNum);

	if (IsRunningDedicatedServer())
	{
		const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		if (ServerApiClient.IsValid())
		{
			ServerApiClient->ServerGameTelemetry.SetImmediateEventList(ImmediateEventList);
		}
	}
	else
	{
		ApiClient = GetApiClient(LocalUserNum);
		if (ApiClient.IsValid())
		{
			ApiClient->GameTelemetry.SetImmediateEventList(ImmediateEventList);
		}
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

