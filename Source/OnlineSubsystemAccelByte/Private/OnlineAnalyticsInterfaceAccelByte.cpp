// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAnalyticsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"

using namespace AccelByte;

bool FOnlineAnalyticsAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem
	, TSharedPtr<FOnlineAnalyticsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
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

bool FOnlineAnalyticsAccelByte::GetFromWorld(const UWorld* World
	, TSharedPtr<FOnlineAnalyticsAccelByte
	, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineAnalyticsAccelByte::SetTelemetrySendInterval(int32 InLocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Set Telemetry Send Interval for LocalUserNum: %d"), InLocalUserNum);

	bool bIsSuccess = false;
	int32 SendTelemetryEventIntervalInSeconds;
	if (GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("SendTelemetryEventIntervalInSeconds"), SendTelemetryEventIntervalInSeconds, GEngineIni))
	{
		if (IsRunningDedicatedServer())
		{
			const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
			if (ServerApiClient.IsValid())
			{
				ServerApiClient->ServerGameTelemetry.SetBatchFrequency(FTimespan::FromSeconds(SendTelemetryEventIntervalInSeconds));
				bIsSuccess = true;
			}
		}
		else
		{
			const auto ApiClient = AccelByteSubsystem->GetApiClient(InLocalUserNum);
			if (ApiClient.IsValid())
			{
				ApiClient->GameTelemetry.SetBatchFrequency(FTimespan::FromSeconds(SendTelemetryEventIntervalInSeconds));
				bIsSuccess = true;
			}
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Set Telemetry Send Interval is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

bool FOnlineAnalyticsAccelByte::SetTelemetryImmediateEventList(int32 InLocalUserNum, TArray<FString> const& EventNames)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Set Telemetry Immediate Event List for LocalUserNum: %d"), InLocalUserNum);
	
	if (!IsUserLoggedIn(InLocalUserNum) || EventNames.Num() <= 0)
	{
		return false;
	}

	bool bIsSuccess = false;
	if (IsRunningDedicatedServer())
	{
		const auto ServerApiClient = FMultiRegistry::GetServerApiClient();
		if (ServerApiClient.IsValid())
		{
			ServerApiClient->ServerGameTelemetry.SetImmediateEventList(EventNames);
			bIsSuccess = true;
		}
	}
	else
	{
		const auto ApiClient = AccelByteSubsystem->GetApiClient(InLocalUserNum);
		if (ApiClient.IsValid())
		{
			ApiClient->GameTelemetry.SetImmediateEventList(EventNames);
			bIsSuccess = true;
		}
	}
	
	AB_OSS_INTERFACE_TRACE_END(TEXT("Set Telemetry Immediate Event List is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

bool FOnlineAnalyticsAccelByte::SetTelemetryCriticalEventList(int32 InLocalUserNum, TArray<FString> const& EventNames)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Set Telemetry Critical Event List for LocalUserNum: %d"), InLocalUserNum);

	if (!IsUserLoggedIn(InLocalUserNum) || EventNames.Num() <= 0)
	{
		return false;
	}

	bool bIsSuccess = false;
	if (!IsRunningDedicatedServer())
	{
		const auto ApiClient = AccelByteSubsystem->GetApiClient(InLocalUserNum);
		if (ApiClient.IsValid())
		{
			ApiClient->GameTelemetry.SetCriticalEventList(EventNames);
			bIsSuccess = true;
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Set Telemetry Critical Event List is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

bool FOnlineAnalyticsAccelByte::SendTelemetryEvent(int32 InLocalUserNum
	, FAccelByteModelsTelemetryBody const& TelemetryBody
	, FVoidHandler const& OnSuccess
	, FErrorHandler const& OnError)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Send Telemetry Event for LocalUserNum: %d"), InLocalUserNum);
	
	bool bIsSuccess = false;
	if (IsUserLoggedIn(InLocalUserNum) && IsValidTelemetry(TelemetryBody))
	{
		if (IsRunningDedicatedServer())
		{
			const auto ServerApiClient = FMultiRegistry::GetServerApiClient();
			if (ServerApiClient.IsValid())
			{
				ServerApiClient->ServerGameTelemetry.Send(TelemetryBody, OnSuccess, OnError);
				bIsSuccess = true;
			}
		}
		else
		{
			const auto ApiClient = AccelByteSubsystem->GetApiClient(InLocalUserNum);
			if (ApiClient.IsValid())
			{
				ApiClient->GameTelemetry.Send(TelemetryBody, OnSuccess, OnError);
				bIsSuccess = true;
			}
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Send Telemetry Event is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
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
