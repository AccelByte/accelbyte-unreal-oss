// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "Models/AccelByteGameTelemetryModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

/**
 * Implementation of Analytics service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineAnalyticsAccelByte
	: public TSharedFromThis<FOnlineAnalyticsAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	FOnlineAnalyticsAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
	{}

public:
	virtual ~FOnlineAnalyticsAccelByte() {};

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World
		, FOnlineAnalyticsAccelBytePtr& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem
		, FOnlineAnalyticsAccelBytePtr& OutInterfaceInstance);

	/**
	 * Set GameTelemetry or ServerGameTelemetry send event interval from DefaultEngine.ini
	 *
	 * @param InLocalUserNum user identifier
	 * @returns boolean that is true if task for setting send interval successfully dispatched
	 */
	bool SetTelemetrySendInterval(int32 InLocalUserNum);

	/**
	 * Set GameTelemetry or ServerGameTelemetry immediate event list
	 *
	 * @param InLocalUserNum user identifier
	 * @param EventNames list of immediate event name 
	 * @returns boolean that is true if task for setting immediate event lists successfully dispatched
	 */
	bool SetTelemetryImmediateEventList(int32 InLocalUserNum
		, TArray<FString> const& EventNames);

	/**
	* Set GameTelemetry or ServerGameTelemetry critical event list
	*
	* @param InLocalUserNum user identifier
	* @param EventNames list of critical event name 
	* @returns boolean that is true if task for setting critical event lists successfully dispatched
	*/
	bool SetTelemetryCriticalEventList(int32 InLocalUserNum
		, TArray<FString> const& EventNames);

	/**
	 * Send GameTelemetry or ServerGameTelemetry event with delegates OnSuccess and OnError
	 *
	 * @param InLocalUserNum user identifier
	 * @param TelemetryBody content of telemetry event
	 * @param OnSuccess delegate will be called after event send succeed
	 * @param OnError delegate will be called after event send error
	 * @returns boolean that is true if task for sending telemetry event successfully dispatched
	 */
	bool SendTelemetryEvent(int32 InLocalUserNum
		, FAccelByteModelsTelemetryBody const& TelemetryBody
		, AccelByte::FVoidHandler const& OnSuccess
		, AccelByte::FErrorHandler const& OnError);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineAnalyticsAccelByte()
		: AccelByteSubsystem(nullptr)
	{}
	
	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

	/**
	 * Helper function to get AccelByteInstance from subsystem
	 *
	 * @return weak ptr instance of the AccelByteInstance, nullptr if it's invalid
	 */
	FAccelByteInstanceWPtr GetAccelByteInstance() const;

private:
	/**
	 * Helper function to check if user logged in
	 *
	 * @param InLocalUserNum user identifier
	 * @return boolean that is true if the user logged in
	 */
	bool IsUserLoggedIn(const int32 InLocalUserNum) const;

	/**
	 * Helper function to check if Telemetry body is valid
	 *
	 * @param TelemetryBody telemetry object
	 * @return boolean that is true if the telemetry object is valid
	 */
	static bool IsValidTelemetry(FAccelByteModelsTelemetryBody const& TelemetryBody);

	/**
	 * Set telemetry send interval for game clients.
	 */
	bool ClientSetTelemetrySendInterval(int32 LocalUserNum, int64 IntervalSeconds);

	/**
	 * Set telemetry send interval for dedicated servers.
	 */
	bool ServerSetTelemetrySendInterval(int32 LocalUserNum, int64 IntervalSeconds);
};
