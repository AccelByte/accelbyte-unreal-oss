// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Models/AccelByteGameTelemetryModels.h"
#include "Models/AccelBytePredefinedEventModels.h"
#include "OnlineErrorAccelByte.h"

DECLARE_MULTICAST_DELEGATE_FourParams(FAccelByteOnSendEventCompleted, int32 /*LocalUserNum*/, const FString& /*EventName*/, bool /*bWasSuccessful*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnSendEventCompleted::FDelegate FAccelByteOnSendEventCompletedDelegate;

/**
 * Implementation of Predefined Event Api from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlinePredefinedEventAccelByte : public TSharedFromThis<FOnlinePredefinedEventAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:

	/** Constructor that is invoked by the Subsystem instance to create a predefined event instance */
	FOnlinePredefinedEventAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
		: AccelByteSubsystem(InSubsystem)
	{
		bIsHaveSettingInterval = GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("SendPredefinedEventInterval"), SettingInterval, GEngineIni);
	}

public:
	virtual ~FOnlinePredefinedEventAccelByte() {};
	
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlinePredefinedEventAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlinePredefinedEventAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnSendEventCompleted, const FString&, bool, const FOnlineErrorAccelByte&);

	template<typename T>
	void SendEvent(int32 LocalUserNum, const TSharedRef<T>& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow())
	{
		if (LocalUserNum < 0)
		{
			LocalUserNum = GetLocalUserNumCached();
		}
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
		if (IdentityInterface.IsValid())
		{
			if (!TIsDerivedFrom<T, FAccelByteModelsPredefinedEvent>::IsDerived)
			{
				OnError((int32)AccelByte::ErrorCodes::InvalidRequest, TEXT("Payload must derive from FAccelByteModelsPredefinedEvent!"), LocalUserNum, Payload->GetPreDefinedEventName());
				return;
			}

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
						OnLogoutSuccessDelegateHandle.Add(LocalUserNum, IdentityInterface->AddAccelByteOnLogoutCompleteDelegate_Handle(LocalUserNum, FAccelByteOnLogoutCompleteDelegate::CreateThreadSafeSP(this, &FOnlinePredefinedEventAccelByte::OnLogoutSuccess)));
					}
				}

				if (!IsRunningDedicatedServer())
				{
					const auto ApiClient = IdentityInterface->GetApiClient(LocalUserNum);
					if (ApiClient.IsValid())
					{
						ApiClient->PredefinedEvent.SendPredefinedEventData(Payload, AccelByte::FVoidHandler::CreateThreadSafeSP(this, &FOnlinePredefinedEventAccelByte::OnSuccess, LocalUserNum, Payload->GetPreDefinedEventName()), FErrorHandler::CreateThreadSafeSP(this, &FOnlinePredefinedEventAccelByte::OnError, LocalUserNum, Payload->GetPreDefinedEventName()), ClientTimestamp);
					}
				}
				else
				{
					const auto ApiClient = AccelByte::FMultiRegistry::GetServerApiClient();
					if (ApiClient.IsValid())
					{
						ApiClient->ServerPredefinedEvent.SendPredefinedEventData(Payload, AccelByte::FVoidHandler::CreateThreadSafeSP(this, &FOnlinePredefinedEventAccelByte::OnSuccess, LocalUserNum, Payload->GetPreDefinedEventName()), FErrorHandler::CreateThreadSafeSP(this, &FOnlinePredefinedEventAccelByte::OnError, LocalUserNum, Payload->GetPreDefinedEventName()), ClientTimestamp);
					}
				}
			}
			else
			{
				if (LocalUserNum != -1)
				{
					if (OnLoginSuccessDelegateHandle.Find(LocalUserNum) == nullptr)
					{
						OnLoginSuccessDelegateHandle.Add(LocalUserNum, IdentityInterface->AddAccelByteOnLoginCompleteDelegate_Handle(LocalUserNum, FAccelByteOnLoginCompleteDelegate::CreateThreadSafeSP(this, &FOnlinePredefinedEventAccelByte::OnLoginSuccess)));
					}
				}
				else
				{
					if (!OnLocalUserNumCachedDelegateHandle.IsValid())
					{
						OnLocalUserNumCachedDelegateHandle = OnLoginSuccessDelegateHandle.Add(LocalUserNum, AccelByteSubsystem->OnLocalUserNumCached().AddRaw(this, &FOnlinePredefinedEventAccelByte::OnLocalUserNumCachedSuccess));
					}
				}

				Payload->PreDefinedEventName = Payload->GetPreDefinedEventName();
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
					OnError((int32)AccelByte::ErrorCodes::InvalidRequest, TEXT("Failed to convert UStruct to Json!"), LocalUserNum, Payload->GetPreDefinedEventName());
					return;
				}
			}
		}
	}

protected:

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlinePredefinedEventAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:

	/** Add Event to cache, used in case of the user is not logged in yet */
	void AddToCache(int32 LocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody>& Cache);

	/** Move cached event to another user, used for non user-tied event that triggered before any user logged in*/
	void MoveTempUserCachedEvent(int32 To);
	bool SetEventSendInterval(int32 InLocalUserNum);
	void SendCachedEvent(int32 InLocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody> & CachedEvent);
	void OnSuccess(int32 LocalUserNum, FString EventName);
	void OnError(int32 ErrorCode, const FString& ErrorMessage, int32 LocalUserNum, FString EventName);
	void OnLoginSuccess(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FOnlineErrorAccelByte& Error);
	void OnLogoutSuccess(int32 LocalUserNum, bool bWasSuccessful, const FOnlineErrorAccelByte& Error);
	void OnLocalUserNumCachedSuccess();
	const int32 GetLocalUserNumCached();

	bool bIsHaveSettingInterval;
	int32 SettingInterval;
	mutable FCriticalSection CachedEventsLock;
	TMap<int32, bool> SetEventIntervalMap;
	TMap<int32, TArray<TSharedPtr<FAccelByteModelsTelemetryBody>>> CachedEvents;
	TMap<int32, FDelegateHandle> OnLoginSuccessDelegateHandle;
	TMap<int32, FDelegateHandle> OnLogoutSuccessDelegateHandle;
	FDelegateHandle OnLocalUserNumCachedDelegateHandle;
};