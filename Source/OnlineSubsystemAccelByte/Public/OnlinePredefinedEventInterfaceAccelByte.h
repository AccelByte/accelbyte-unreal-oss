// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineBaseAnalyticsInterfaceAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Models/AccelBytePredefinedEventModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

/**
 * Implementation of Predefined Event Api from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlinePredefinedEventAccelByte : public FOnlineBaseAnalyticsAccelByte
{
PACKAGE_SCOPE:

	/** Constructor that is invoked by the Subsystem instance to create a predefined event instance */
	FOnlinePredefinedEventAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
		: FOnlineBaseAnalyticsAccelByte(InSubsystem)
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

	template<typename T>
	void SendEvent(int32 LocalUserNum, const TSharedRef<T>& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow())
	{
		if (!TIsDerivedFrom<T, FAccelByteModelsPredefinedEvent>::IsDerived)
		{
			OnError((int32)AccelByte::ErrorCodes::InvalidRequest, TEXT("Payload must derive from FAccelByteModelsPredefinedEvent!"), LocalUserNum, TEXT(""));
			return;
		}

		if (LocalUserNum < 0)
		{
			LocalUserNum = GetLocalUserNumCached();
		}

		SetDelegatesAndInterval(LocalUserNum);
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
		if (IdentityInterface.IsValid())
		{
			if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
			{
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
				Payload->PreDefinedEventName = Payload->GetPreDefinedEventName();
				ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetPreDefinedEventName(), ClientTimestamp);
			}
		}
	}

protected:

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlinePredefinedEventAccelByte()
		: FOnlineBaseAnalyticsAccelByte(nullptr)
	{}

private:
	virtual bool SetEventSendInterval(int32 InLocalUserNum) override;
	virtual void SendCachedEvent(int32 InLocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody> & CachedEvent) override;
};