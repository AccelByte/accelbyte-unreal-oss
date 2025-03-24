// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAnalyticsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineIdentityInterfaceAccelByte.h"


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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Set Telemetry Send Interval for LocalUserNum: %d"), InLocalUserNum);

	bool bIsSuccess = false;
	int32 SendTelemetryEventIntervalInSeconds;
	if (GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("SendTelemetryEventIntervalInSeconds"), SendTelemetryEventIntervalInSeconds, GEngineIni))
	{
		if (IsRunningDedicatedServer())
		{
			const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
			if(AccelByteInstance.IsValid())
			{
				const FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient();
				if (ServerApiClient.IsValid())
				{
					ServerApiClient->ServerGameTelemetry.SetBatchFrequency(FTimespan::FromSeconds(SendTelemetryEventIntervalInSeconds));
					bIsSuccess = true;
				}
			}
		}
		else
		{
			const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
			if(!AccelByteSubsystemPtr.IsValid())
			{
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to set telemetry send interval, AccelByteSubsystem ptr is invalid"));
				return false;
			}
			
			const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(InLocalUserNum);
			if (ApiClient.IsValid())
			{
				const auto GameTelemetry = ApiClient->GetGameTelemetryApi().Pin();
				if (GameTelemetry.IsValid())
				{
					GameTelemetry->SetBatchFrequency(FTimespan::FromSeconds(SendTelemetryEventIntervalInSeconds));
					bIsSuccess = true;
				}
			}
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Set Telemetry Send Interval is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

bool FOnlineAnalyticsAccelByte::SetTelemetryImmediateEventList(int32 InLocalUserNum, TArray<FString> const& EventNames)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Set Telemetry Immediate Event List for LocalUserNum: %d"), InLocalUserNum);
	
	if (!IsUserLoggedIn(InLocalUserNum) || EventNames.Num() <= 0)
	{
		return false;
	}

	bool bIsSuccess = false;
	if (IsRunningDedicatedServer())
	{
		const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
		if(AccelByteInstance.IsValid())
		{
			const FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient();
			if (ServerApiClient.IsValid())
			{
				ServerApiClient->ServerGameTelemetry.SetImmediateEventList(EventNames);
				bIsSuccess = true;
			}
		}
	}
	else
	{
		const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
		if(!AccelByteSubsystemPtr.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to set telemetry immediate event list, AccelByteSubsystem ptr is invalid"));
			return false;
		}
		
		const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(InLocalUserNum);
		if (ApiClient.IsValid())
		{
			const auto GameTelemetry = ApiClient->GetGameTelemetryApi().Pin();
			if (GameTelemetry.IsValid())
			{
				GameTelemetry->SetImmediateEventList(EventNames);
				bIsSuccess = true;
			}
		}
	}
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Set Telemetry Immediate Event List is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

bool FOnlineAnalyticsAccelByte::SetTelemetryCriticalEventList(int32 InLocalUserNum, TArray<FString> const& EventNames)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Set Telemetry Critical Event List for LocalUserNum: %d"), InLocalUserNum);

	if (!IsUserLoggedIn(InLocalUserNum) || EventNames.Num() <= 0)
	{
		return false;
	}

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to set telemetry critical event list, AccelByteSubsystem ptr is invalid"));
		return false;
	}

	bool bIsSuccess = false;
	if (!IsRunningDedicatedServer())
	{
		const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(InLocalUserNum);
		if (ApiClient.IsValid())
		{
			const auto GameTelemetry = ApiClient->GetGameTelemetryApi().Pin();
			if (GameTelemetry.IsValid())
			{
				GameTelemetry->SetCriticalEventList(EventNames);
				bIsSuccess = true;
			}
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Set Telemetry Critical Event List is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

bool FOnlineAnalyticsAccelByte::SendTelemetryEvent(int32 InLocalUserNum
	, FAccelByteModelsTelemetryBody const& TelemetryBody
	, FVoidHandler const& OnSuccess
	, FErrorHandler const& OnError)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send Telemetry Event for LocalUserNum: %d"), InLocalUserNum);
	
	bool bIsSuccess = false;
	if (IsUserLoggedIn(InLocalUserNum) && IsValidTelemetry(TelemetryBody))
	{
		if (IsRunningDedicatedServer())
		{
			const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
			if(AccelByteInstance.IsValid())
			{
				const FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient();
				if (ServerApiClient.IsValid())
				{
					ServerApiClient->ServerGameTelemetry.Send(TelemetryBody, OnSuccess, OnError);
					bIsSuccess = true;
				}
			}
		}
		else
		{
			const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
			if(!AccelByteSubsystemPtr.IsValid())
			{
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to send telemetry event, AccelByteSubsystem ptr is invalid"));
				return false;
			}
			
			const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(InLocalUserNum);
			if (ApiClient.IsValid())
			{
				const auto GameTelemetry = ApiClient->GetGameTelemetryApi().Pin();
				if (GameTelemetry.IsValid())
				{
					GameTelemetry->Send(TelemetryBody, OnSuccess, OnError);
					bIsSuccess = true;
				}
			}
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send Telemetry Event is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

bool FOnlineAnalyticsAccelByte::IsUserLoggedIn(const int32 InLocalUserNum) const
{
	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to check is user logged in, AccelByteSubsystem ptr is invalid"));
		return false;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	
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

FAccelByteInstanceWPtr FOnlineAnalyticsAccelByte::GetAccelByteInstance() const
{
	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to get accelbyte instance, AccelByteSubsystem ptr is invalid"));
		return nullptr;
	}

	return AccelByteSubsystemPtr->GetAccelByteInstance();
}

bool FOnlineAnalyticsAccelByte::IsValidTelemetry(FAccelByteModelsTelemetryBody const& TelemetryBody)
{
	return TelemetryBody.Payload.IsValid() && !TelemetryBody.EventName.IsEmpty();
}
