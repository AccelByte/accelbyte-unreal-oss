// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAnalyticsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Analytics/OnlineAsyncTaskAccelByteSendTelemetry.h"
#include "AsyncTasks/Analytics/OnlineAsyncTaskAccelByteSetImmediateEventList.h"
#include "AsyncTasks/Analytics/OnlineAsyncTaskAccelByteSetTelemetryInterval.h"

bool FOnlineAnalyticsInterfaceAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineAnalyticsInterfaceAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
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

bool FOnlineAnalyticsInterfaceAccelByte::GetFromWorld(const UWorld* World, TSharedPtr<FOnlineAnalyticsInterfaceAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineAnalyticsInterfaceAccelByte::SetTelemetrySendInterval(int32 InLocalUserNum)
{
	int32 SendTelemetryEventIntervalInSeconds;
	if(GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("SendTelemetryEventIntervalInSeconds"), SendTelemetryEventIntervalInSeconds, GEngineIni))
	{
		if(IsUserLoggedIn(InLocalUserNum))
		{
			AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSetTelemetryInterval>(
				AccelByteSubsystem, InLocalUserNum, SendTelemetryEventIntervalInSeconds);
			return true;
		}
	}

	return false;
}

bool FOnlineAnalyticsInterfaceAccelByte::SetTelemetryImmediateEventList(int32 InLocalUserNum, TArray<FString> const& EventNames)
{
	if (IsUserLoggedIn(InLocalUserNum))
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSetImmediateEventList>(
			AccelByteSubsystem, InLocalUserNum, EventNames);
		return true;
	}

	return false;
}

bool FOnlineAnalyticsInterfaceAccelByte::SendTelemetryEvent(
	int32 InLocalUserNum, FAccelByteModelsTelemetryBody const& TelemetryBody,
	FVoidHandler const& OnSuccess, FErrorHandler const& OnError)
{
	if (IsUserLoggedIn(InLocalUserNum) && IsValidTelemetry(TelemetryBody))
	{
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendTelemetry>(
			AccelByteSubsystem, InLocalUserNum, TelemetryBody, OnSuccess, OnError);
		
		return true;
	}

	return false;
}

bool FOnlineAnalyticsInterfaceAccelByte::IsUserLoggedIn(const int32 InLocalUserNum) const
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

bool FOnlineAnalyticsInterfaceAccelByte::IsValidTelemetry(FAccelByteModelsTelemetryBody const& TelemetryBody)
{
	return TelemetryBody.Payload.IsValid() && !TelemetryBody.EventName.IsEmpty() && !TelemetryBody.EventNamespace.IsEmpty();
}
