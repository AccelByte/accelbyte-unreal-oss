// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"

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

	bool bIsSuccess = false;
	if (bIsHaveSettingInterval)
	{
		if (IsRunningDedicatedServer())
		{
			const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
			if(!AccelByteInstance.IsValid())
			{
				bIsSuccess = false;
			}
			else
			{
				const FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient();
				if (ServerApiClient.IsValid())
				{
					ServerApiClient->ServerPredefinedEvent.SetBatchFrequency(FTimespan::FromSeconds(SettingInterval));
					bIsSuccess = true;
				}
				else
				{
					bIsSuccess = false;
				}
			}
		}
		else
		{
			FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
			if (AccelByteSubsystemPtr.IsValid())
			{
				const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(InLocalUserNum);
				if (ApiClient.IsValid())
				{
					const auto PredefinedEvent = ApiClient->GetPredefinedEventApi().Pin();
					if (PredefinedEvent.IsValid())
					{
						PredefinedEvent->SetBatchFrequency(FTimespan::FromSeconds(SettingInterval));
						bIsSuccess = true;
					}
				}
			}
		}
	}
	else
	{
		bIsSuccess = true;
	}

	SetEventIntervalMap.Add(InLocalUserNum, bIsSuccess);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Set PredefinedEvent Send Interval is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
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