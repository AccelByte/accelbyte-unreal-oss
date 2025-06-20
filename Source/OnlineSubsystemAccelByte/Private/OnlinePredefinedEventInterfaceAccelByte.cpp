// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteConfig.h"

using namespace AccelByte;

bool FOnlinePredefinedEventAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlinePredefinedEventAccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetPredefinedEventInterface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlinePredefinedEventAccelByte::GetFromWorld(const UWorld* World, FOnlinePredefinedEventAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlinePredefinedEventAccelByte::SetEventSendInterval(int32 InLocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Set PredefinedEvent Send Interval for LocalUserNum: %d"), InLocalUserNum);

	int64 IntervalSeconds { 1 * 60 };
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			IntervalSeconds = Config->GetPredefinedEventSendIntervalSeconds().GetValue();
		}
	}

	const int64* CachedIntervalSeconds = SetEventIntervalMap.Find(InLocalUserNum);
	if (CachedIntervalSeconds != nullptr && *CachedIntervalSeconds == IntervalSeconds)
	{
		// No need to update as our cached value matches our currently configured value, return success
		return true;
	}

	bool bSuccessful { false };
	if (IsRunningDedicatedServer())
	{
		bSuccessful = ServerSetEventSendInterval(IntervalSeconds);
	}
	else
	{
		bSuccessful = ClientSetEventSendInterval(InLocalUserNum, IntervalSeconds);
	}

	if (bSuccessful)
	{
		// Only store new cached value if the call was successful
		int64& NewCachedIntervalSeconds = SetEventIntervalMap.FindOrAdd(InLocalUserNum);
		NewCachedIntervalSeconds = IntervalSeconds;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Set PredefinedEvent Send Interval is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bSuccessful));
	return bSuccessful;
}

void FOnlinePredefinedEventAccelByte::SendCachedEvent(int32 InLocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody> & CachedEvent)
{
	if (CachedEvent->Payload.IsValid())
	{
		TSharedRef<FAccelByteModelsCachedPredefinedEventPayload> Payload = MakeShared<FAccelByteModelsCachedPredefinedEventPayload>();
		Payload->Payload = *CachedEvent.Get();
		CachedEvent->Payload->TryGetStringField(TEXT("PreDefinedEventName"), Payload->PreDefinedEventName);
		SendEvent(InLocalUserNum, Payload, CachedEvent->ClientTimestamp);
	}
}

bool FOnlinePredefinedEventAccelByte::ClientSetEventSendInterval(int32 LocalUserNum, int64 IntervalSeconds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: AccelByte subsystem instance is invalid"));
		return false;
	}

	const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: API client is invalid"));
		return false;
	}

	const auto PredefinedEvent = ApiClient->GetPredefinedEventApi().Pin();
	if (!PredefinedEvent.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: predefined event API is invalid"));
		return false;
	}

	PredefinedEvent->SetBatchFrequency(FTimespan::FromSeconds(IntervalSeconds));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlinePredefinedEventAccelByte::ServerSetEventSendInterval(int64 IntervalSeconds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
	if(!AccelByteInstance.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: AccelByte instance is invalid"));
		return false;
	}
	
	const FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient();
	if (!ServerApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: API client instance is invalid"));
		return false;
	}

	ServerApiClient->ServerPredefinedEvent.SetBatchFrequency(FTimespan::FromSeconds(IntervalSeconds));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}
