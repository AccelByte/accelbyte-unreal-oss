// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Models/AccelByteGameTelemetryModels.h"
#include "OnlineErrorAccelByte.h"

DECLARE_MULTICAST_DELEGATE_FourParams(FAccelByteOnSendEventCompleted, int32 /*LocalUserNum*/, const FString& /*EventName*/, bool /*bWasSuccessful*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnSendEventCompleted::FDelegate FAccelByteOnSendEventCompletedDelegate;

/**
 * Implementation of Analytics Api from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineBaseAnalyticsAccelByte : public TSharedFromThis<FOnlineBaseAnalyticsAccelByte, ESPMode::ThreadSafe>
{
protected:

	/** Constructor that is invoked by the Subsystem instance to create an analytics instance */
	FOnlineBaseAnalyticsAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	virtual ~FOnlineBaseAnalyticsAccelByte() {};

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnSendEventCompleted, const FString&, bool, const FOnlineErrorAccelByte&);

	template<typename T>
	void ConvertAndAddToCache(int32 LocalUserNum, const TSharedRef<T>& Payload, const FString& EventName, const FDateTime& ClientTimestamp = FDateTime::UtcNow())
	{
		if (LocalUserNum < 0)
		{
			LocalUserNum = GetLocalUserNumCached();
		}

		const TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Payload.Get());
		if (JsonObject.IsValid())
		{
			TSharedPtr<FAccelByteModelsTelemetryBody> Body = MakeShared<FAccelByteModelsTelemetryBody>();
			Body->Payload = JsonObject;
			Body->ClientTimestamp = ClientTimestamp;
			AddToCache(LocalUserNum, Body);
		}
		else
		{
			OnError((int32)AccelByte::ErrorCodes::InvalidRequest, TEXT("Failed to convert UStruct to Json!"), LocalUserNum, EventName);
			return;
		}
	}


	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineBaseAnalyticsAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;
	
	bool bIsHaveSettingInterval;
	int32 SettingInterval;
	TMap<int32, bool> SetEventIntervalMap;

	/** Add Event to cache, used in case of the user is not logged in yet */
	void AddToCache(int32 LocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody>& Cache);

	/** Move cached event to another user, used for non user-tied event that triggered before any user logged in*/
	void MoveTempUserCachedEvent(int32 To);
	void OnSuccess(int32 LocalUserNum, FString EventName);
	void OnError(int32 ErrorCode, const FString& ErrorMessage, int32 LocalUserNum, FString EventName);
	void OnLoginSuccess(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FOnlineErrorAccelByte& Error);
	void OnLogoutSuccess(int32 LocalUserNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error);
	void OnLocalUserNumCachedSuccess();
	const int32 GetLocalUserNumCached();
	void SetDelegatesAndInterval(int32 LocalUserNum);

	virtual bool SetEventSendInterval(int32 InLocalUserNum) = 0;
	virtual void SendCachedEvent(int32 InLocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody> & CachedEvent) = 0;

	FAccelByteInstanceWPtr GetAccelByteInstance() const;

private:
	mutable FCriticalSection CachedEventsLock;
	TMap<int32, TArray<TSharedPtr<FAccelByteModelsTelemetryBody>>> CachedEvents;
	TMap<int32, FDelegateHandle> OnLoginSuccessDelegateHandle;
	TMap<int32, FDelegateHandle> OnLogoutSuccessDelegateHandle;
	FDelegateHandle OnLocalUserNumCachedDelegateHandle;
};