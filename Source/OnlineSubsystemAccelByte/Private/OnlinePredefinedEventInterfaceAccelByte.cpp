// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemUtils.h"

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
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Set PredefinedEvent Send Interval for LocalUserNum: %d"), InLocalUserNum);

	bool bIsSuccess = false;
	if (bIsHaveSettingInterval)
	{
		if (IsRunningDedicatedServer())
		{
			const auto ServerApiClient = AccelByte::FMultiRegistry::GetServerApiClient();
			if (ServerApiClient.IsValid())
			{
				ServerApiClient->ServerPredefinedEvent.SetBatchFrequency(FTimespan::FromSeconds(SettingInterval));
				bIsSuccess = true;
			}
		}
		else
		{
			const auto ApiClient = AccelByteSubsystem->GetApiClient(InLocalUserNum);
			if (ApiClient.IsValid())
			{
				ApiClient->PredefinedEvent.SetBatchFrequency(FTimespan::FromSeconds(SettingInterval));
				bIsSuccess = true;
			}
		}
	}
	else
	{
		bIsSuccess = true;
	}

	SetEventIntervalMap.Add(InLocalUserNum, bIsSuccess);

	AB_OSS_INTERFACE_TRACE_END(TEXT("Set PredefinedEvent Send Interval is finished with status (Success: %s)"), LOG_BOOL_FORMAT(bIsSuccess));
	return bIsSuccess;
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelBytePredefinedEvent"
void FOnlinePredefinedEventAccelByte::OnSuccess(int32 LocalUserNum, FString EventName)
{
	TriggerAccelByteOnSendEventCompletedDelegates(LocalUserNum, EventName, true, ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success));
}

void FOnlinePredefinedEventAccelByte::OnError(int32 ErrorCode, const FString & ErrorMessage, int32 LocalUserNum, FString EventName)
{
	TriggerAccelByteOnSendEventCompletedDelegates(LocalUserNum, EventName, true, ONLINE_ERROR_ACCELBYTE(ErrorCode));
}

void FOnlinePredefinedEventAccelByte::OnLocalUserNumCachedSuccess()
{
	int32 LocalUserNum = GetLocalUserNumCached();
	MoveTempUserCachedEvent(LocalUserNum);
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		const FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		const ELoginStatus::Type LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);

		if (OnLoginSuccessDelegateHandle.Find(LocalUserNum) == nullptr)
		{
			OnLoginSuccessDelegateHandle.Add(LocalUserNum, IdentityInterface->AddAccelByteOnLoginCompleteDelegate_Handle(LocalUserNum, FAccelByteOnLoginCompleteDelegate::CreateThreadSafeSP(this, &FOnlinePredefinedEventAccelByte::OnLoginSuccess)));
		}

		if (LoginStatus == ELoginStatus::LoggedIn)
		{
			OnLoginSuccess(LocalUserNum, true, *UserId.Get(), ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success));
		}

		AccelByteSubsystem->OnLocalUserNumCached().Remove(OnLocalUserNumCachedDelegateHandle);
		OnLocalUserNumCachedDelegateHandle.Reset();
	}
}
#undef ONLINE_ERROR_NAMESPACE

void FOnlinePredefinedEventAccelByte::AddToCache(int32 LocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody>& Cache)
{
	FScopeLock ScopeLock(&CachedEventsLock);
	auto CachedEvent =  CachedEvents.Find(LocalUserNum);
	if (CachedEvent == nullptr)
	{
		CachedEvents.Emplace(LocalUserNum, TArray<TSharedPtr<FAccelByteModelsTelemetryBody>>());
		CachedEvent = CachedEvents.Find(LocalUserNum);
	}
	CachedEvent->Add(Cache);
}

void FOnlinePredefinedEventAccelByte::MoveTempUserCachedEvent(int32 To)
{
	FScopeLock ScopeLock(&CachedEventsLock);
	const int32 TempLocalUserNum = -1;
	auto CachedEvent = CachedEvents.Find(TempLocalUserNum);
	if (CachedEvent != nullptr)
	{
		CachedEvents.Emplace(To, *CachedEvent);
		CachedEvents.Remove(TempLocalUserNum);
	}
}

void FOnlinePredefinedEventAccelByte::SendCachedEvent(int32 InLocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody> & CachedEvent)
{
	if (CachedEvent->Payload.IsValid())
	{
		TSharedRef<FAccelByteModelsCachedEventPayload> Payload = MakeShared<FAccelByteModelsCachedEventPayload>();
		Payload->Payload = *CachedEvent.Get();
		SendEvent(InLocalUserNum, Payload, CachedEvent->ClientTimestamp);
	}
}

void FOnlinePredefinedEventAccelByte::OnLoginSuccess(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FOnlineErrorAccelByte& Error)
{
	if (bWasSuccessful)
	{
		FScopeLock ScopeLock(&CachedEventsLock);
		auto CachedEvent = CachedEvents.Find(LocalUserNum);

		if (CachedEvent != nullptr)
		{
			for (const auto& Cache : *CachedEvent)
			{
				if (Cache.IsValid())
				{
					SendCachedEvent(LocalUserNum, Cache);
				}
			}
			CachedEvent->Empty();
		}
	}
}

void FOnlinePredefinedEventAccelByte::OnLogoutSuccess(int32 LocalUserNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error)
{
	if (bWasSuccessful)
	{
		SetEventIntervalMap.Remove(LocalUserNum);
		FScopeLock ScopeLock(&CachedEventsLock);
		CachedEvents.Remove(LocalUserNum);
	}
}

const int32 FOnlinePredefinedEventAccelByte::GetLocalUserNumCached()
{
	int32 LocalUserNum = -1;
	if (AccelByteSubsystem->IsLocalUserNumCached())
	{
		LocalUserNum = AccelByteSubsystem->GetLocalUserNumCached();
	}
	return LocalUserNum;
}
