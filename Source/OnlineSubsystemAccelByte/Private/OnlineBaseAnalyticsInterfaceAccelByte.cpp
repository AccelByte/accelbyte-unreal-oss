// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineBaseAnalyticsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemUtils.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteAnalytics"
void FOnlineBaseAnalyticsAccelByte::OnSuccess(int32 LocalUserNum, FString EventName)
{
	TriggerAccelByteOnSendEventCompletedDelegates(LocalUserNum, EventName, true, ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success));
}

void FOnlineBaseAnalyticsAccelByte::OnError(int32 ErrorCode, const FString & ErrorMessage, int32 LocalUserNum, FString EventName)
{
	TriggerAccelByteOnSendEventCompletedDelegates(LocalUserNum, EventName, true, ONLINE_ERROR_ACCELBYTE(ErrorCode));
}

void FOnlineBaseAnalyticsAccelByte::OnLocalUserNumCachedSuccess()
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
			OnLoginSuccessDelegateHandle.Add(LocalUserNum, IdentityInterface->AddAccelByteOnLoginCompleteDelegate_Handle(LocalUserNum, FAccelByteOnLoginCompleteDelegate::CreateThreadSafeSP(this, &FOnlineBaseAnalyticsAccelByte::OnLoginSuccess)));
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

void FOnlineBaseAnalyticsAccelByte::AddToCache(int32 LocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody>& Cache)
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

void FOnlineBaseAnalyticsAccelByte::MoveTempUserCachedEvent(int32 To)
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

void FOnlineBaseAnalyticsAccelByte::OnLoginSuccess(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FOnlineErrorAccelByte& Error)
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

void FOnlineBaseAnalyticsAccelByte::OnLogoutSuccess(int32 LocalUserNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error)
{
	if (bWasSuccessful)
	{
		SetEventIntervalMap.Remove(LocalUserNum);
		FScopeLock ScopeLock(&CachedEventsLock);
		CachedEvents.Remove(LocalUserNum);
	}
}

const int32 FOnlineBaseAnalyticsAccelByte::GetLocalUserNumCached()
{
	int32 LocalUserNum = -1;
	if (AccelByteSubsystem->IsLocalUserNumCached())
	{
		LocalUserNum = AccelByteSubsystem->GetLocalUserNumCached();
	}
	return LocalUserNum;
}

void FOnlineBaseAnalyticsAccelByte::SetDelegatesAndInterval(int32 LocalUserNum)
{
	if (LocalUserNum < 0)
	{
		LocalUserNum = GetLocalUserNumCached();
	}
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		if (!SetEventIntervalMap.Find(LocalUserNum) && LocalUserNum != -1)
		{
			SetEventSendInterval(LocalUserNum);
		}

		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			if (OnLogoutSuccessDelegateHandle.Find(LocalUserNum) == nullptr)
			{
				if (LocalUserNum != -1)
				{
					OnLogoutSuccessDelegateHandle.Add(LocalUserNum, IdentityInterface->AddAccelByteOnLogoutCompleteDelegate_Handle(LocalUserNum, FAccelByteOnLogoutCompleteDelegate::CreateThreadSafeSP(this, &FOnlineBaseAnalyticsAccelByte::OnLogoutSuccess)));
				}
			}
		}
		else
		{
			if (LocalUserNum != -1)
			{
				if (OnLoginSuccessDelegateHandle.Find(LocalUserNum) == nullptr)
				{
					OnLoginSuccessDelegateHandle.Add(LocalUserNum, IdentityInterface->AddAccelByteOnLoginCompleteDelegate_Handle(LocalUserNum, FAccelByteOnLoginCompleteDelegate::CreateThreadSafeSP(this, &FOnlineBaseAnalyticsAccelByte::OnLoginSuccess)));
				}
			}
			else
			{
				if (!OnLocalUserNumCachedDelegateHandle.IsValid())
				{
					OnLocalUserNumCachedDelegateHandle = OnLoginSuccessDelegateHandle.Add(LocalUserNum, AccelByteSubsystem->OnLocalUserNumCached().AddRaw(this, &FOnlineBaseAnalyticsAccelByte::OnLocalUserNumCachedSuccess));
				}
			}
		}
	}
}
