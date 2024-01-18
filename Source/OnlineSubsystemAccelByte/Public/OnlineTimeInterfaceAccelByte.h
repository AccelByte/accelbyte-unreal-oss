// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlineTimeInterface.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Misc/DateTime.h"
#include "OnlineSubsystemAccelBytePackage.h"

class ONLINESUBSYSTEMACCELBYTE_API FOnlineTimeAccelByte : public IOnlineTime
{
PACKAGE_SCOPE:
	/** Constructor that is invoked by the Subsystem instance to create a Time interface instance */
	FOnlineTimeAccelByte(FOnlineSubsystemAccelByte* InSubsystem);
	
public:
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineTimeAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineTimeAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	virtual bool QueryServerUtcTime() override;
	virtual FString GetLastServerUtcTime() override;

	/**
	 * Get server current timestamp back calculated from cached last server time query
	 * 
	 * @returns string representation of time (yyyy.MM.dd-HH.mm.ss)
	 */
	virtual FString GetCurrentServerUtcTime();

protected:
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;
};
