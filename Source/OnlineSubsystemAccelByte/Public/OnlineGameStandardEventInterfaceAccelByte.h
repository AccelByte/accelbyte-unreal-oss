// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineBaseAnalyticsInterfaceAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "Models/AccelByteGameStandardEventModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

/**
 * Implementation of Predefined Event Api from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineGameStandardEventAccelByte : public FOnlineBaseAnalyticsAccelByte
{
PACKAGE_SCOPE:

	/** Constructor that is invoked by the Subsystem instance to create a predefined event instance */
	FOnlineGameStandardEventAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
		: FOnlineBaseAnalyticsAccelByte(InSubsystem)
	{
		bIsHaveSettingInterval = GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("SendGameStandardEventInterval"), SettingInterval, GEngineIni);
	}

public:
	virtual ~FOnlineGameStandardEventAccelByte() {};
	
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineGameStandardEventAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineGameStandardEventAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	void SendResourceSourcedEvent(int32 LocalUserNum, const FAccelByteModelsResourceSourcedPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendResourceSinkedEvent(int32 LocalUserNum, const FAccelByteModelsResourceSinkedPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendResourceUpgradedEvent(int32 LocalUserNum, const FAccelByteModelsResourceUpgradedPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendResourceActionedEvent(int32 LocalUserNum, const FAccelByteModelsResourceActionedPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendQuestStartedEvent(int32 LocalUserNum, const FAccelByteModelsQuestStartedPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendQuestEndedEvent(int32 LocalUserNum, const FAccelByteModelsQuestEndedPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendPlayerLeveledEvent(int32 LocalUserNum, const FAccelByteModelsPlayerLeveledPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendPlayerDeadEvent(int32 LocalUserNum, const FAccelByteModelsPlayerDeadPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());
	void SendRewardCollectedEvent(int32 LocalUserNum, const FAccelByteModelsRewardCollectedPayload& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow());

protected:
	template<typename T>
	void SendEvent(int32 LocalUserNum, const TSharedRef<T>& Payload, const FDateTime& ClientTimestamp = FDateTime::UtcNow())
	{
		if (!TIsDerivedFrom<T, FAccelByteModelsGameStandardEvent>::IsDerived)
		{
			OnError((int32)AccelByte::ErrorCodes::InvalidRequest, TEXT("Payload must derive from FAccelByteModelsGameStandardEvent!"), LocalUserNum, TEXT(""));
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
						ApiClient->GameStandardEvent.SendGameStandardEventData(Payload, AccelByte::FVoidHandler::CreateThreadSafeSP(this, &FOnlineGameStandardEventAccelByte::OnSuccess, LocalUserNum, Payload->GetGameStandardEventName()), FErrorHandler::CreateThreadSafeSP(this, &FOnlineGameStandardEventAccelByte::OnError, LocalUserNum, Payload->GetGameStandardEventName()), ClientTimestamp);
					}
				}
				else
				{
					const auto ApiClient = AccelByte::FMultiRegistry::GetServerApiClient();
					if (ApiClient.IsValid())
					{
						ApiClient->ServerGameStandardEvent.SendGameStandardEventData(Payload, AccelByte::FVoidHandler::CreateThreadSafeSP(this, &FOnlineGameStandardEventAccelByte::OnSuccess, LocalUserNum, Payload->GetGameStandardEventName()), FErrorHandler::CreateThreadSafeSP(this, &FOnlineGameStandardEventAccelByte::OnError, LocalUserNum, Payload->GetGameStandardEventName()), ClientTimestamp);
					}
				}
			}
			else
			{
				Payload->GameStandardEventName = Payload->GetGameStandardEventName();
				ConvertAndAddToCache(LocalUserNum, Payload, Payload->GetGameStandardEventName(), ClientTimestamp);
			}
		}
	}

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineGameStandardEventAccelByte()
		: FOnlineBaseAnalyticsAccelByte(nullptr)
	{}

private:
	virtual bool SetEventSendInterval(int32 InLocalUserNum) override;
	virtual void SendCachedEvent(int32 InLocalUserNum, const TSharedPtr<FAccelByteModelsTelemetryBody> & CachedEvent) override;
};