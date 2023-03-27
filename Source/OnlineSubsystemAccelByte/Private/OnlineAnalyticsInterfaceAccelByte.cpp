// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAnalyticsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Analytics/OnlineAsyncTaskAccelByteSendTelemetry.h"
#include "AsyncTasks/Analytics/OnlineAsyncTaskAccelByteSetImmediateEventList.h"
#include "AsyncTasks/Analytics/OnlineAsyncTaskAccelByteSetTelemetryInterval.h"

bool FOnlineAnalyticsAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineAnalyticsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetAnalyticsInterface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineAnalyticsAccelByte::GetFromWorld(const UWorld* World, TSharedPtr<FOnlineAnalyticsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineAnalyticsAccelByte::SetTelemetrySendInterval(int32 InLocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Set telemetry send interval for LocalUserNum: %d"), InLocalUserNum);

	int32 SendTelemetryEventIntervalInSeconds;
	if(GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("SendTelemetryEventIntervalInSeconds"), SendTelemetryEventIntervalInSeconds, GEngineIni))
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSetTelemetryInterval>(
			AccelByteSubsystem, InLocalUserNum, SendTelemetryEventIntervalInSeconds);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to set telemetry send interval"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("SendTelemetryEventIntervalInSeconds is not found in config, using default interval"));
	return false;
}

bool FOnlineAnalyticsAccelByte::SetTelemetryImmediateEventList(int32 InLocalUserNum, TArray<FString> const& EventNames)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Set telemetry immediate event list for LocalUserNum: %d"), InLocalUserNum);
	if (EventNames.Num() <= 0)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Event list is empty"));
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSetImmediateEventList>(
		AccelByteSubsystem, InLocalUserNum, EventNames);
	AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to set immediate event list"));
	return true;
}

bool FOnlineAnalyticsAccelByte::SendTelemetryEvent(
	int32 InLocalUserNum, FAccelByteModelsTelemetryBody const& TelemetryBody,
	FVoidHandler const& OnSuccess, FErrorHandler const& OnError)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Send telemetry event for LocalUserNum: %d"), InLocalUserNum);
	if (IsUserLoggedIn(InLocalUserNum) && IsValidTelemetry(TelemetryBody))
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendTelemetry>(
			AccelByteSubsystem, InLocalUserNum, TelemetryBody, OnSuccess, OnError);
		AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to send telemetry event"));
		return true;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to send telemetry event, user not logged in or invalid telemetry"));
	return false;
}

bool FOnlineAnalyticsAccelByte::IsUserLoggedIn(const int32 InLocalUserNum) const
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	
	if (IdentityInterface.IsValid())
	{
		if (IsRunningDedicatedServer())
		{
			#if AB_USE_V2_SESSIONS
				return IdentityInterface->GetLoginStatus(InLocalUserNum) == ELoginStatus::LoggedIn;
			#else
				return IdentityInterface->IsServerAuthenticated();
			#endif
		}

		return IdentityInterface->GetLoginStatus(InLocalUserNum) == ELoginStatus::LoggedIn;
	}
	
	return false;
}

bool FOnlineAnalyticsAccelByte::IsValidTelemetry(FAccelByteModelsTelemetryBody const& TelemetryBody)
{
	return TelemetryBody.Payload.IsValid() && !TelemetryBody.EventName.IsEmpty() && !TelemetryBody.EventNamespace.IsEmpty();
}
