// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

class FOnlineSubsystemAccelByte;


class FOnlineUserPresenceAccelByte : public FOnlineUserPresence
{
public:

	FOnlineUserPresenceAccelByte()
		: FOnlineUserPresence()
	{
	}

};

class FOnlineUserPresenceStatusAccelByte : public FOnlineUserPresenceStatus
{
public:
	void SetPresenceStatus(EAvailability InPresenceStatus)
	{
		switch (InPresenceStatus)
		{
		case EAvailability::Online: State = EOnlinePresenceState::Online; break;
		case EAvailability::Busy: State = EOnlinePresenceState::DoNotDisturb; break;
		case EAvailability::Invisible: State = EOnlinePresenceState::Chat; break;
		case EAvailability::Away: State = EOnlinePresenceState::Away; break;
		case EAvailability::Offline:
		default: State = EOnlinePresenceState::Offline; break;
		}
	}

	FOnlineUserPresenceStatusAccelByte()
		: FOnlineUserPresenceStatus()
	{}
};

#define FUserIDPresenceMap TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>>

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBulkQueryPresenceComplete, const bool /*bWasSuccessful*/, const FUserIDPresenceMap& /*Presences*/);
typedef FOnBulkQueryPresenceComplete::FDelegate FOnBulkQueryPresenceCompleteDelegate;

/**
 * Implementation of the IOnlinePresence interface using AccelByte services.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlinePresenceAccelByte : public IOnlinePresence, public TSharedFromThis<FOnlinePresenceAccelByte, ESPMode::ThreadSafe> 
{
public:
	
	/** Constructor that is invoked by the Subsystem instance to create a presence interface instance */
	FOnlinePresenceAccelByte (FOnlineSubsystemAccelByte* InSubsystem);
	virtual ~FOnlinePresenceAccelByte() override = default;
	
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnBulkQueryPresenceComplete,  const bool /*bWasSuccessful*/, const FUserIDPresenceMap& /*Presences*/);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlinePresenceAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlinePresenceAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);
	
	//~ Begin IOnlinePresence Interface
	virtual void SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual void QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual EOnlineCachedResult::Type GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) override;
	virtual EOnlineCachedResult::Type GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence>& OutPresence) override;
	//~ End IOnlinePresence Interface

	//~ Begin Custom Presence
	virtual void PlatformQueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate());
	virtual EOnlineCachedResult::Type GetPlatformCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence);
	
	/**
	 * Bulk query user presences.
	 *
	 * @param LocalUserId LocalUserId of user making the call.
	 * @param UserIds UserIds to query the presence.
	 */
	virtual void BulkQueryPresence(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& UserIds);
	//~ End Custom Presence

PACKAGE_SCOPE:

	/**
	 * Method used by the Identity interface to register delegates for presence notifications to this interface to get
	 * real-time updates from the Lobby websocket.
	 */
	virtual void RegisterRealTimeLobbyDelegates(int32 LocalUserNum);

	/** Used to update cached Presence */
	TSharedRef<FOnlineUserPresenceAccelByte> FindOrCreatePresence(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId);

	/** Getter function for cached user presence mapped by userId */
	TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>> GetCachedPresence() const;

	/** Add or Update cached presence*/
	void UpdatePresenceCache(const TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>>& UpdatedPresence);

	/** Add or Update cached presence*/
	void UpdatePresenceCache(const FString& UserID, const TSharedRef<FOnlineUserPresenceAccelByte>& Presence);

protected: 

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlinePresenceAccelByte()
		: AccelByteSubsystem(nullptr) {}

	IOnlinePresencePtr GetPlatformOnlinePresenceInterface() const;

	/** Delegate handler for when a friend change their presence status */
	void OnFriendStatusChangedNotificationReceived(const FAccelByteModelsUsersPresenceNotice& Notification, int32 LocalUserNum);
private: 

	/** All presence information we have */
	TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>> CachedPresenceByUserId;

	/** Critical section to lock presence cache map while accessing */
	mutable FCriticalSection PresenceCacheLock;
};

typedef TSharedPtr<FOnlinePresenceAccelByte, ESPMode::ThreadSafe> FOnlinePresenceAccelBytePtr;