// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineGameStandardEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"

bool FOnlineGameStandardEventAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineGameStandardEventAccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetGameStandardEventInterface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineGameStandardEventAccelByte::GetFromWorld(const UWorld* World, FOnlineGameStandardEventAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineGameStandardEventAccelByte::SetEventSendInterval(int32 InLocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Set GameStandardEvent Send Interval for LocalUserNum: %d"), InLocalUserNum);

	bool bIsSuccess = false;
	if (bIsHaveSettingInterval)
	{
		if (IsRunningDedicatedServer())
		{
			const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
			if(AccelByteInstance.IsValid())
			{
				const FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient();
				if (ServerApiClient.IsValid())
				{
					ServerApiClient->ServerGameStandardEvent.SetBatchFrequency(FTimespan::FromSeconds(SettingInterval));
					bIsSuccess = true;
				}
			}
		}
		else
		{
			FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
			if (!AccelByteSubsystemPtr.IsValid())
			{
				AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
			}
			else
			{
				const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(InLocalUserNum);
				if (ApiClient.IsValid())
				{
					ApiClient->GameStandardEvent.SetBatchFrequency(FTimespan::FromSeconds(SettingInterval));
					bIsSuccess = true;
				}
			}
		}
	}
	else
	{
		bIsSuccess = true;
	}

	SetEventIntervalMap.Add(InLocalUserNum, bIsSuccess);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Set GameStandardEvent Send Interval is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

void FOnlineGameStandardEventAccelByte::SendCachedEvent(int32 InLocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody> & CachedEvent)
{
	if (CachedEvent->Payload.IsValid())
	{
		TSharedRef<FAccelByteModelsCachedGameStandardEventPayload> Payload = MakeShared<FAccelByteModelsCachedGameStandardEventPayload>();
		Payload->Payload = *CachedEvent.Get();
		CachedEvent->Payload->TryGetStringField(TEXT("GameStandardEventName"), Payload->GameStandardEventName);
		SendEvent(InLocalUserNum, Payload, CachedEvent->ClientTimestamp);
	}
}

void FOnlineGameStandardEventAccelByte::SendResourceSourcedEvent(int32 LocalUserNum, const FAccelByteModelsResourceSourcedPayload& Payload, const FDateTime& ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsResourceSourcedPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendResourceSinkedEvent(int32 LocalUserNum, const FAccelByteModelsResourceSinkedPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsResourceSinkedPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendResourceUpgradedEvent(int32 LocalUserNum, const FAccelByteModelsResourceUpgradedPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsResourceUpgradedPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendResourceActionedEvent(int32 LocalUserNum, const FAccelByteModelsResourceActionedPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsResourceActionedPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendQuestStartedEvent(int32 LocalUserNum, const FAccelByteModelsQuestStartedPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsQuestStartedPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendQuestEndedEvent(int32 LocalUserNum, const FAccelByteModelsQuestEndedPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsQuestEndedPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendPlayerLeveledEvent(int32 LocalUserNum, const FAccelByteModelsPlayerLeveledPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerLeveledPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendPlayerDeadEvent(int32 LocalUserNum, const FAccelByteModelsPlayerDeadPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPlayerDeadPayload>(Payload), ClientTimestamp);
}

void FOnlineGameStandardEventAccelByte::SendRewardCollectedEvent(int32 LocalUserNum, const FAccelByteModelsRewardCollectedPayload & Payload, const FDateTime & ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsRewardCollectedPayload>(Payload), ClientTimestamp);
}
