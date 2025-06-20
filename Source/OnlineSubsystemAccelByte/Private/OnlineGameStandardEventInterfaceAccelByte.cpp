// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineGameStandardEventInterfaceAccelByte.h"
#include "OnlineBaseAnalyticsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteConfig.h"
#include "Core/AccelByteServerApiClient.h"

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

	int64 IntervalSeconds { 1 * 60 };
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (AccelByteSubsystemPtr.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
		if (Config.IsValid())
		{
			IntervalSeconds = Config->GetGameStandardEventSendIntervalSeconds().GetValue();
		}
	}

	const int64* CachedIntervalSeconds = SetEventIntervalMap.Find(InLocalUserNum);
	if (CachedIntervalSeconds != nullptr && *CachedIntervalSeconds == IntervalSeconds)
	{
		// No need to update as our cached value matches our currently configured value, return success
		return true;
	}

	bool bSuccessful{};
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

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Set GameStandardEvent Send Interval is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bSuccessful));
	return bSuccessful;
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

void FOnlineGameStandardEventAccelByte::SendRewardCollectedEvent(int32 LocalUserNum, const FAccelByteModelsRewardCollectedPayload& Payload, const FDateTime& ClientTimestamp)
{
	SendEvent(LocalUserNum, MakeShared<FAccelByteModelsRewardCollectedPayload>(Payload), ClientTimestamp);
}

bool FOnlineGameStandardEventAccelByte::SendMissionStartedEvent(int32 LocalUserNum
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, FMissionId const& MissionId
	, FMissionInstanceId const& MissionInstanceId
	, FAccelByteModelsMissionStartedOptPayload const& Optional)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send MissionStarted Event for LocalUserNum: %d"), LocalUserNum);
	if (!UserId.IsValid()
		|| MissionId.IsEmpty()
		|| !MissionInstanceId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStarted Event is finished"));
					return GameStandardEvent->SendMissionStartedEventData(FAccelByteUserId(UserId->GetAccelByteId())
						, MissionId
						, MissionInstanceId
						, Optional);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStarted Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendMissionStartedEventData(FAccelByteUserId(UserId->GetAccelByteId())
					, MissionId
					, MissionInstanceId
					, Optional);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsMissionStartedPayload>(Optional);
			Payload->UserID = UserId->GetAccelByteId();
			Payload->MissionID = MissionId.ToString();
			Payload->MissionInstanceID = MissionInstanceId.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStarted Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStarted Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendMissionStepEndedEvent(int32 LocalUserNum
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, FMissionId const& MissionId
	, FMissionInstanceId const& MissionInstanceId
	, FMissionStep const& MissionStep
	, FMissionStepName const& MissionStepName)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send MissionStepEnded Event for LocalUserNum: %d"), LocalUserNum);
	if (!UserId.IsValid()
		|| MissionId.IsEmpty()
		|| !MissionInstanceId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStepEnded Event is finished"));
					return GameStandardEvent->SendMissionStepEndedEventData(FAccelByteUserId(UserId->GetAccelByteId())
						, MissionId
						, MissionInstanceId
						, MissionStep
						, MissionStepName);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStepEnded Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendMissionStepEndedEventData(FAccelByteUserId(UserId->GetAccelByteId())
					, MissionId
					, MissionInstanceId
					, MissionStep
					, MissionStepName);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsMissionStepEndedPayload>();
			Payload->UserID = UserId->GetAccelByteId();
			Payload->MissionID = MissionId.ToString();
			Payload->MissionInstanceID = MissionInstanceId.ToString();
			Payload->MissionStep = MissionStep.GetValue();
			Payload->MissionStepName = MissionStepName.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStepEnded Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionStepEnded Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendMissionEndedEvent(int32 LocalUserNum
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, FMissionId const& MissionId
	, FMissionInstanceId const& MissionInstanceId
	, FMissionSuccess const& MissionSuccess
	, FMissionOutcome const& MissionOutcome)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send MissionEnded Event for LocalUserNum: %d"), LocalUserNum);
	if (!UserId.IsValid()
		|| MissionId.IsEmpty()
		|| !MissionInstanceId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionEnded Event is finished"));
					return GameStandardEvent->SendMissionEndedEventData(FAccelByteUserId(UserId->GetAccelByteId())
						, MissionId
						, MissionInstanceId
						, MissionSuccess
						, MissionOutcome);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionEnded Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendMissionEndedEventData(FAccelByteUserId(UserId->GetAccelByteId())
					, MissionId
					, MissionInstanceId
					, MissionSuccess
					, MissionOutcome);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsMissionEndedPayload>();
			Payload->UserID = UserId->GetAccelByteId();
			Payload->MissionID = MissionId.ToString();
			Payload->MissionInstanceID = MissionInstanceId.ToString();
			Payload->MissionSuccess = MissionSuccess.GetValue();
			Payload->MissionOutcome = MissionOutcome.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionEnded Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MissionEnded Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendMatchInfoEvent(int32 LocalUserNum
	, FMatchInfoId const& MatchInfoId
	, FAccelByteModelsMatchInfoOptPayload const& Optional)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send MatchInfo Event for LocalUserNum: %d"), LocalUserNum);
	if (MatchInfoId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfo Event is finished"));
					return GameStandardEvent->SendMatchInfoEventData(MatchInfoId
						, Optional);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfo Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendMatchInfoEventData(MatchInfoId
					, Optional);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsMatchInfoPayload>(Optional);
			Payload->MatchinfoID = MatchInfoId.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfo Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfo Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendMatchInfoPlayerEvent(int32 LocalUserNum
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, FMatchInfoId const& MatchInfoId
	, FAccelByteModelsMatchInfoPlayerOptPayload const& Optional)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send MatchInfoPlayer Event for LocalUserNum: %d"), LocalUserNum);
	if (!UserId.IsValid()
		|| MatchInfoId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoPlayer Event is finished"));
					return GameStandardEvent->SendMatchInfoPlayerEventData(FAccelByteUserId(UserId->GetAccelByteId())
						, MatchInfoId
						, Optional);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoPlayer Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendMatchInfoPlayerEventData(FAccelByteUserId(UserId->GetAccelByteId())
					, MatchInfoId
					, Optional);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsMatchInfoPlayerPayload>(Optional);
			Payload->UserID = UserId->GetAccelByteId();
			Payload->MatchinfoID = MatchInfoId.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoPlayer Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoPlayer Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendMatchInfoEndedEvent(int32 LocalUserNum
	, FMatchInfoId const& MatchInfoId
	, FMatchEndReason const& EndReason
	, FAccelByteModelsMatchInfoEndedOptPayload const& Optional)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send MatchInfoEnded Event for LocalUserNum: %d"), LocalUserNum);
	if (MatchInfoId.IsEmpty()
		|| EndReason.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoEnded Event is finished"));
					return GameStandardEvent->SendMatchInfoEndedEventData(MatchInfoId
						, EndReason
						, Optional);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoEnded Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendMatchInfoEndedEventData(MatchInfoId
					, EndReason
					, Optional);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsMatchInfoEndedPayload>(Optional);
			Payload->MatchinfoID = MatchInfoId.ToString();
			Payload->EndReason = EndReason.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoEnded Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send MatchInfoEnded Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendPopupAppearEvent(int32 LocalUserNum
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, FPopupEventId const& PopupId
	, FAccelByteModelsPopupAppearOptPayload const& Optional)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send PopupAppear Event for LocalUserNum: %d"), LocalUserNum);
	if (!UserId.IsValid()
		|| PopupId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send PopupAppear Event is finished"));
					return GameStandardEvent->SendPopupAppearEventData(FAccelByteUserId(UserId->GetAccelByteId())
						, PopupId
						, Optional);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send PopupAppear Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendPopupAppearEventData(FAccelByteUserId(UserId->GetAccelByteId())
					, PopupId
					, Optional);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsPopupAppearPayload>(Optional);
			Payload->UserID = UserId->GetAccelByteId();
			Payload->PopupID = PopupId.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send PopupAppear Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send PopupAppear Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendEntityLeveledEvent(int32 LocalUserNum
	, FEntityType const& EntityType
	, FEntityId const& EntityId
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, FAccelByteModelsEntityLeveledOptPayload const& Optional)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send EntityLeveled Event for LocalUserNum: %d"), LocalUserNum);
	if (EntityType.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		const FString UserIdStr = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityLeveled Event is finished"));
					return GameStandardEvent->SendEntityLeveledEventData(EntityType
						, EntityId
						, FAccelByteUserId(UserIdStr)
						, Optional);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityLeveled Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendEntityLeveledEventData(EntityType
					, EntityId
					, FAccelByteUserId(UserIdStr)
					, Optional);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsEntityLeveledPayload>(Optional);
			Payload->UserID = UserIdStr;
			Payload->EntityID = EntityId.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityLeveled Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityLeveled Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendEntityDeadEvent(int32 LocalUserNum
	, FEntityType const& EntityType
	, FEntityId const& EntityId
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, FAccelByteModelsEntityDeadOptPayload const& Optional)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send EntityDead Event for LocalUserNum: %d"), LocalUserNum);
	if (EntityType.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		const FString UserIdStr = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityDead Event is finished"));
					return GameStandardEvent->SendEntityDeadEventData(EntityType
						, EntityId
						, FAccelByteUserId(UserIdStr)
						, Optional);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityDead Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendEntityDeadEventData(EntityType
					, EntityId
					, FAccelByteUserId(UserIdStr)
					, Optional);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsEntityDeadPayload>(Optional);
			Payload->UserID = UserIdStr;
			Payload->EntityID = EntityId.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityDead Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send EntityDead Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendResourceFlowEvent(int32 LocalUserNum
	, FUniqueNetIdAccelByteUserPtr const& UserId
	, EAccelByteFlowType const& FlowType
	, FAccelByteTransactionId const& TransactionId
	, FTransactionType const& TransactionType
	, FResourceName const& ResourceName
	, FResourceAmount const& Amount
	, FResourceEndBalance const& EndBalance)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Send ResourceFlow Event for LocalUserNum: %d"), LocalUserNum);
	if (!UserId.IsValid()
		|| !TransactionId.IsValid()
		|| TransactionType.IsEmpty()
		|| ResourceName.IsEmpty()
		|| !Amount.IsValid()
		|| !EndBalance.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, Request is invalid"));
		return false;
	}
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	SetDelegatesAndInterval(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (!IsRunningDedicatedServer())
			{
				const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
				const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
				if (GameStandardEvent.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send ResourceFlow Event is finished"));
					return GameStandardEvent->SendResourceFlowEventData(FAccelByteUserId(UserId->GetAccelByteId())
						, FlowType
						, TransactionId
						, TransactionType
						, ResourceName
						, Amount
						, EndBalance);
				}
			}
			else
			{
				const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
				if(!AccelByteInstance.IsValid())
				{
					AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("failed, AccelByteInstance is invalid"));
					return false;
				}
				
				const auto ApiClient = AccelByteInstance->GetServerApiClient();
				AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send ResourceFlow Event is finished"));
				return ApiClient->ServerGameStandardEvent.SendResourceFlowEventData(FAccelByteUserId(UserId->GetAccelByteId())
				, FlowType
				, TransactionId
				, TransactionType
				, ResourceName
				, Amount
				, EndBalance);
			}
		}
		else
		{
			const auto Payload = MakeShared<FAccelByteModelsResourceFlowPayload>();
			Payload->UserID = UserId->GetAccelByteId();
			Payload->FlowType = FAccelByteUtilities::GetUEnumValueAsString(FlowType);
			Payload->TransactionID = TransactionId.ToString();
			Payload->TransactionType = TransactionType.ToString();
			Payload->ResourceName = ResourceName.ToString();
			Payload->Amount = Amount.ToString();
			Payload->EndBalance = EndBalance.ToString();
			Payload->GameStandardEventName = Payload->GetGameStandardEventName();
			ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), FDateTime::UtcNow());
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send ResourceFlow Event is finished"));
			return true;
		}
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Send ResourceFlow Event is finished"));
	return false;
}

bool FOnlineGameStandardEventAccelByte::SendMatchInfoEvent(int32 LocalUserNum, FMatchInfoId const& MatchInfoId, TSharedPtr<FNamedOnlineSession> const& Session, FMatchGameMode const& GameMode, FMatchDifficulty const& MatchDifficulty)
{
	FAccelByteModelsMatchInfoOptPayload Optional;
	if (Session.IsValid() && (!Session->GetSessionIdStr().Equals(TEXT("InvalidSession"))))
	{
		Optional.MatchID = Session->GetSessionIdStr();
	}
	Optional.GameMode = GameMode.ToString();
	Optional.MatchDifficulty = MatchDifficulty.ToString();
	return SendMatchInfoEvent(LocalUserNum, MatchInfoId, Optional);
}

bool FOnlineGameStandardEventAccelByte::SendMatchInfoPlayerEvent(int32 LocalUserNum, FUniqueNetIdAccelByteUserPtr const& UserId, FMatchInfoId const& MatchInfoId, TSharedPtr<FNamedOnlineSession> const& Session, FMatchTeam const& Team, FMatchClass const& Class, FMatchRank const& Rank)
{
	FAccelByteModelsMatchInfoPlayerOptPayload Optional;
	if (Session.IsValid() && (!Session->GetSessionIdStr().Equals(TEXT("InvalidSession"))))
	{
		Optional.MatchID = Session->GetSessionIdStr();
	}
	Optional.Team = Team.ToString();
	Optional.Class = Class.ToString();
	Optional.Rank = Rank.ToString();
	return SendMatchInfoPlayerEvent(LocalUserNum, UserId, MatchInfoId, Optional);
}

bool FOnlineGameStandardEventAccelByte::SendMatchInfoEndedEvent(int32 LocalUserNum, FMatchInfoId const& MatchInfoId, FMatchEndReason const& EndReason, TSharedPtr<FNamedOnlineSession> const& Session, FMatchWinner const& Winner)
{
	FAccelByteModelsMatchInfoEndedOptPayload Optional;
	if (Session.IsValid() && (!Session->GetSessionIdStr().Equals(TEXT("InvalidSession"))))
	{
		Optional.MatchID = Session->GetSessionIdStr();
	}
	Optional.Winner = Winner.ToString();
	return SendMatchInfoEndedEvent(LocalUserNum, MatchInfoId, EndReason, Optional);
}

bool FOnlineGameStandardEventAccelByte::ClientSetEventSendInterval(int32 LocalUserNum, int64 IntervalSeconds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: AccelByte subsystem is invalid"));
		return false;
	}

	const auto ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if (!ApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: API client is invalid"));
		return false;
	}

	const auto GameStandardEvent = ApiClient->GetGameStandardEventApi().Pin();
	if (!GameStandardEvent.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: GameStandardEvent API is invalid"));
		return false;
	}

	GameStandardEvent->SetBatchFrequency(FTimespan::FromSeconds(IntervalSeconds));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}

bool FOnlineGameStandardEventAccelByte::ServerSetEventSendInterval(int64 IntervalSeconds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	const FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
	if (!AccelByteInstance.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: AccelByte instance is invalid"));
		return false;
	}

	const AccelByte::FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient();
	if (!ServerApiClient.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to set event send interval: server API client is invalid"));
		return false;
	}

	ServerApiClient->ServerGameStandardEvent.SetBatchFrequency(FTimespan::FromSeconds(IntervalSeconds));

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}
