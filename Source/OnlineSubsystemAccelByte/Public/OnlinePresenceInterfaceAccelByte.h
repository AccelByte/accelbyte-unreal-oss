// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreOnline.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePresenceInterface.h"

class FOnlineSubsystemAccelByte;


class FOnlineUserPresenceAccelByte : public FOnlineUserPresence
{
public:

	FOnlineUserPresenceAccelByte()
		: FOnlineUserPresence()
	{
	}

};

/**
 * Implementation of the IOnlinePresence interface using AccelByte services.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlinePresenceAccelByte : public IOnlinePresence, public TSharedFromThis<FOnlinePresenceAccelByte, ESPMode::ThreadSafe> 
{
public:
	
	/** Constructor that is invoked by the Subsystem instance to create a presence interface instance */
	FOnlinePresenceAccelByte (FOnlineSubsystemAccelByte* InSubsystem);

	virtual ~FOnlinePresenceAccelByte() override = default;
	
	//~ Begin IOnlinePresence Interface
	virtual void SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual void QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual EOnlineCachedResult::Type GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) override;
	virtual EOnlineCachedResult::Type GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence>& OutPresence) override;
	//~ End IOnlinePresence Interface

	//~ Begin Custom Presence
	virtual void PlatformQueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate());
	virtual EOnlineCachedResult::Type GetPlatformCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence);
	//~ End Custom Presence

PACKAGE_SCOPE:

	/** Used to update cached Presence */
	TSharedRef<FOnlineUserPresenceAccelByte> FindOrCreatePresence(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId);

protected: 

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlinePresenceAccelByte()
		: AccelByteSubsystem(nullptr) {}

	IOnlinePresencePtr GetPlatformOnlinePresenceInterface() const;

private: 

	/** All presence information we have */
	TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>> CachedPresenceByUserId;
};

typedef TSharedPtr<FOnlinePresenceAccelByte, ESPMode::ThreadSafe> FOnlinePresenceAccelbytePtr;