// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Models/AccelByteUserModels.h"
#include "Misc/ConfigCacheIni.h"
#include "Core/AccelByteUtilities.h"
#include <atomic>
#include "OnlineSubsystemAccelBytePackage.h"

/**
 * @brief Templated structure used to convert a string value to a typed value. Used when loading from config, or setting
 * values directly using a string.
 */
template <typename T>
struct TOnlineSubsystemAccelByteConfigValueConverter
{
	/**
	 * @brief Convert a given string to a templated value.
	 */
	bool ConfigValueFromString(TWeakPtr<class FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> Config
		, const FString& InString
		, T& OutValue);
};

/**
 * @brief Templated structure used to convert an array of string values to an array of typed values. This should not need
 * to be specialized. Prefer specializing TOnlineSubsystemAccelByteConfigValueConverter<T>, which will automatically be
 * used per element.
 */
template <typename T>
struct TOnlineSubsystemAccelByteConfigArrayValueConverter
{
	/**
	 * @brief Convert a given string array to an array of templated values. Creates a
	 * TOnlineSubsystemAccelByteConfigValueConverter instance using the array element
	 * type, and converts individual strings to that type.
	 */
	// #HACK: Since Unreal 4.27 uses C++14, we cannot reliably use 'if constexpr'. But we only want to compile this
	// method if the type of the class is an array. With that in mind, declare a fake template parameter that is used
	// to enable the correct method overload.
	template<typename FakeT>
	typename TEnableIf<TIsTArray<FakeT>::Value, bool>::Type ConfigArrayValueFromStringArray(TWeakPtr<class FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> Config
		, const TArray<FString>& InStringArray
		, T& OutArray)
	{
		using ElementType = typename T::ElementType;

		TOnlineSubsystemAccelByteConfigValueConverter<ElementType> Converter{};
		for (const FString& Element : InStringArray)
		{
			ElementType ConvertedElement{};
			if (!Converter.ConfigValueFromString(Config, Element, ConvertedElement))
			{
				return false;
			}

			OutArray.Emplace(MoveTemp(ConvertedElement));
		}

		return true;
	}

	/**
	 * Stub method used if the type passed in is not an array. This should be replaced with an 'if constexpr' when we
	 * are able to.
	 */
	template<typename FakeT>
	typename TEnableIf<!TIsTArray<FakeT>::Value, bool>::Type ConfigArrayValueFromStringArray(TWeakPtr<class FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> Config
		, const TArray<FString>& InStringArray
		, T& OutArray)
	{
		return false;
	}
};

/**
 * @brief Represents a single configuration field for the AccelByte OSS.
 */
template <typename T>
class TOnlineSubsystemAccelByteConfigEntry
{
// Types
public:
	using FValidatorFunction = TFunction<bool(const T&)>;

public:
	/**
	 * @brief Load the given configuration entry from the configuration file. Will overwrite previously set value.
	 * @return bool that is true if the load was successful, false otherwise
	 */
	bool Load()
	{
		if (Section.IsEmpty())
		{
			return false;
		}

		if (Key.IsEmpty())
		{
			return false;
		}

		if (TIsTArray<T>::Value)
		{
			TArray<FString> StringArray{};
			
			// #NOTE: LoadABConfigFallback does not support arrays, and we do not have a format for arrays on the command
			// line in place. For now, just directly grab the array from the config. In the future, we may want to convert
			// this to allow for command line overriding.
			const int32 ElemNum = GConfig->GetArray(*Section
				, *Key
				, StringArray
				, GEngineIni);

			if (ElemNum <= 0)
			{
				return false;
			}

			return SetValueFromStringArray(StringArray);
		}
		else
		{
			FString ConfigValueStr{};
			const bool bExists = FAccelByteUtilities::LoadABConfigFallback(Section
				, Key
				, ConfigValueStr);

			if (!bExists)
			{
				return false;
			}

			return SetValueFromString(ConfigValueStr);
		}
	}

	/**
	 * @brief Set value of this configuration entry to a copy of the given value.
	 * @param InValue Value to set the configuration to
	 */
	void SetValue(const T& InValue)
	{
		if (Validator && !Validator(InValue))
		{
			return;
		}

		Value = InValue;
	}

	/**
	 * @brief Set the value of this configuration by moving the given value.
	 * @param InValue Value to set the configuration to
	 */
	void SetValue(T&& InValue)
	{
		if (Validator && !Validator(InValue))
		{
			return;
		}

		Value = InValue;
	}

	/**
	 * @brief Set the value of this configuration entry from the given string.
	 */
	bool SetValueFromString(const FString& InStringValue)
	{
		TOnlineSubsystemAccelByteConfigValueConverter<T> Converter{};

		T TempValue{};
		if (!Converter.ConfigValueFromString(Config, InStringValue, TempValue))
		{
			return false;
		}

		SetValue(MoveTemp(TempValue));
		return true;
	}

	/**
	 * @brief Set the value of this configuration entry from the given string array.
	 */
	bool SetValueFromStringArray(const TArray<FString>& InStringArray)
	{
		TOnlineSubsystemAccelByteConfigArrayValueConverter<T> Converter{};

		T TempValue{};
		if (!Converter.template ConfigArrayValueFromStringArray<T>(Config, InStringArray, TempValue))
		{
			return false;
		}

		SetValue(MoveTemp(TempValue));
		return true;
	}

	/**
	 * @brief Get the value stored in this configuration entry.
	 */
	const T& GetValue() const
	{
		return Value;
	}
	
	/**
	 * @brief Get the default value stored in this configuration entry.
	 */
	const T& GetDefaultValue() const
	{
		return DefaultValue;
	}

	/**
	 * @brief Check whether this configuration entry is set to the default.
	 */
	const bool IsDefault() const
	{
		return Value == DefaultValue;
	}

	/**
	 * @brief Reset the configuration entry to its default value.
	 */
	void Reset()
	{
		Value = DefaultValue;
	}

PACKAGE_SCOPE:
	/**
	 * @brief Internal default constructor. Config entry is not ready to use with just this constructor. Entry should be
	 * initialized by calling the Init method in FOnlineSubsystemAccelByteConfig::Init.
	 */
	TOnlineSubsystemAccelByteConfigEntry() = default;

	/**
	 * @brief Initialize this configuration entry with all necessary values. Only should be called from the
	 * FOnlineSubsystemAccelByteConfig::Init method.
	 */
	void Init(TSharedRef<class FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> InConfig
		, const FString& InSection
		, const FString& InKey
		, const T& InDefaultValue
		, const FValidatorFunction& InValidator={})
	{
		Config = InConfig;
		Section = InSection;
		Key = InKey;
		Value = InDefaultValue;
		DefaultValue = InDefaultValue;
		Validator = InValidator;
	}

private:
	/**
	 * @brief Weak reference to the owning config instance.
	 */
	TWeakPtr<class FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> Config{};

	/**
	 * @brief Section that this configuration entry should be loaded from.
	 */
	FString Section{};

	/**
	 * @brief Key that this configuration entry should be loaded from.
	 */
	FString Key{};

	/**
	 * @brief Current value of the configuration entry.
	 */
	T Value{};

	/**
	 * @brief Default value of the configuration entry.
	 */
	T DefaultValue{};

	/**
	 * @brief Optional function used for further validation of config values.
	 */
	FValidatorFunction Validator{};

// Disable move and copy, as the entry should only be retrieved from the config class as a shared instance
public:
	TOnlineSubsystemAccelByteConfigEntry(const TOnlineSubsystemAccelByteConfigEntry&) = delete;
	TOnlineSubsystemAccelByteConfigEntry(TOnlineSubsystemAccelByteConfigEntry&&) = delete;

	TOnlineSubsystemAccelByteConfigEntry& operator=(const TOnlineSubsystemAccelByteConfigEntry&) = delete;
	TOnlineSubsystemAccelByteConfigEntry& operator=(TOnlineSubsystemAccelByteConfigEntry&&) = delete;

};

/**
 * @brief Handles getting and setting any configuration variables for the OnlineSubsystem. All variables minus arrays
 * allow for overriding on the command line. Any portion of OSS code that requires accessing configuration is recommended
 * to directly grab the config instance from the subsystem, and directly get the value from one of the getters. This will
 * allow for on the fly changes of config values to take effect, such as for testing purposes.
 *
 * When adding a new config entry, these steps must be taken:
 * 1. Add the entry with private access
 * 2. Make sure that the Init method of the entry is called in the config Init method
 * 3. Make sure the Load method of the entry is called in the config Reload method
 * 4. Make sure that the Reset method of the entry is called in the config Reset method
 * 5. Make sure to add both const and non-const getters for the entry
 * 6. Make sure to add a log line to the Dump method for the config entry
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineSubsystemAccelByteConfig
	: public TSharedFromThis<FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe>
{
public:
	FOnlineSubsystemAccelByteConfig() = default;
	~FOnlineSubsystemAccelByteConfig() = default;

public:
	/**
	 * @brief Loads all configuration entries from their respective sources.
	 */
	void Reload();
	
	/**
	 * @brief Reset all configuration entries to their default values.
	 */
	void Reset();

	/**
	 * @brief Dumps out state for each configuration entry to the log.
	 */
	void Dump();

	/**
	 * @brief Gets whether this config instance has already been initialized.
	 */
	bool IsInitialized() const;

PACKAGE_SCOPE:
	/**
	 * @brief Set up and load all configuration values from their respective sources.
	 */
	void Init();

// Non-const Getters
public:
	/**
	 * @brief Get configuration entry for lobby websocket auto connect on log in success.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetAutoConnectLobbyAfterLoginSuccess();

	/**
	 * @brief Get configuration entry for chat websocket auto connect on log in success.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetAutoConnectChatAfterLoginSuccess();

	/**
	 * @brief Get configuration entry for requiring manual refresh of native platform tokens.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableManualNativePlatformTokenRefresh();

	/**
	 * @brief Get configuration entry for source of user's display name.
	 */
	TOnlineSubsystemAccelByteConfigEntry<EAccelBytePlatformType>& GetDisplayNameSource();

	/**
	 * @brief Get configuration entry for name of native platform.
	 */
	TOnlineSubsystemAccelByteConfigEntry<FName>& GetNativePlatformName();

	/**
	 * @brief Get configuration entry for name of secondary platform.
	 */
	TOnlineSubsystemAccelByteConfigEntry<FName>& GetSecondaryPlatformName();

	/**
	 * @brief Get configuration entry for EOS refresh token interval in milliseconds.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetEOSRefreshIntervalMilliseconds();

	/**
	 * @brief Get configuration entry controlling whether encryption is enabled in the auth handler component.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableAuthHandlerEncryption();

	/**
	 * @brief Get configuration entry controlling whether maximum chat message limit should be automatically queried
	 * before connection.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetAutoCheckMaximumChatMessageLimit();

	/**
	 * @brief Get configuration entry controlling whether polling for match ticket information after a initial delay at
	 * an interval should be enabled.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableMatchTicketPolling();

	/**
	 * @brief Get configuration entry controlling the initial delay time in seconds before match ticket polling starts.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetMatchTicketPollStartDelaySeconds();

	/**
	 * @brief Get configuration entry controlling the interval time in seconds for match ticket polling.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetMatchTicketPollIntervalSeconds();

	/**
	 * @brief Get configuration entry controlling whether polling for server information after a initial delay at an
	 * interval should be enabled.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableServerInfoPolling();

	/**
	 * @brief Get configuration entry controlling the initial delay time in seconds before server information polling starts.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetServerInfoPollStartDelaySeconds();

	/**
	 * @brief Get configuration entry controlling the interval time in seconds for server information polling.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetServerInfoPollIntervalSeconds();

	/**
	 * @brief Get configuration entry controlling whether polling for matchmaking session invites after a initial delay
	 * at an interval should be enabled.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableSessionInvitePolling();

	/**
	 * @brief Get configuration entry controlling the initial delay time in seconds before matchmaking session invite
	 * polling starts.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetSessionInvitePollStartDelaySeconds();

	/**
	 * @brief Get configuration entry controlling the interval time in seconds for matchmaking session invite polling.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetSessionInvitePollIntervalSeconds();

	/**
	 * @brief Get configuration entry controlling whether server registration should be manually initiated by game code.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableManualServerRegistration();

	/**
	 * @brief Get configuration entry controlling whether staleness checks on cached user data is enabled.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableStalenessChecking();

	/**
	 * @brief Get configuration entry controlling how long cached user data can be held before it is considered stale.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetTimeUntilStaleSeconds();

	/**
	 * @brief Get configuration entry controlling whether lobby connection should be gated by an entitlement check.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableEntitlementGateCheck();

	/**
	 * @brief Get configuration entry controlling entitlement SKUs that are checked for entitlement gating.
	 */
	TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& GetEntitlementGateCheckSkus();

	/**
	 * @brief Get configuration entry controlling entitlement item IDs that are checked for entitlement gating.
	 */
	TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& GetEntitlementGateCheckItemIds();

	/**
	 * @brief Get configuration entry controlling entitlement app IDs that are checked for entitlement gating.
	 */
	TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& GetEntitlementGateCheckAppIds();

	/**
	 * @brief Get configuration entry controlling the amount of time in seconds between sending telemetry event batches.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetSendTelemetryEventIntervalSeconds();

	/**
	 * @brief Get configuration entry controlling whether voice chat is enabled.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetVoiceEnabled();

	/**
	 * @brief Get configuration entry controlling threshold in seconds until log in is presented as queued.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetLoginQueuePresentationThresholdSeconds();

	/**
	 * @brief Get configuration entry controlling whether log in queue ticket will require manual claim through game code.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableManualLoginQueueClaim();

	/**
	 * @brief Get configuration entry controlling interval at which events from the game standard event API get sent.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetGameStandardEventSendIntervalSeconds();

	/**
	 * @brief Get configuration entry controlling interval at which events from the predefined event API get sent.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64>& GetPredefinedEventSendIntervalSeconds();

// Const Getters
public:
	/**
	 * @brief Get configuration entry for lobby websocket auto connect on log in success.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetAutoConnectLobbyAfterLoginSuccess() const;

	/**
	 * @brief Get configuration entry for chat websocket auto connect on log in success.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetAutoConnectChatAfterLoginSuccess() const;

	/**
	 * @brief Get configuration entry for requiring manual refresh of native platform tokens.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableManualNativePlatformTokenRefresh() const;

	/**
	 * @brief Get configuration entry for source of user's display name.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<EAccelBytePlatformType>& GetDisplayNameSource() const;

	/**
	 * @brief Get configuration entry for name of native platform.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<FName>& GetNativePlatformName() const;

	/**
	 * @brief Get configuration entry for name of secondary platform.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<FName>& GetSecondaryPlatformName() const;

	/**
	 * @brief Get configuration entry for EOS refresh token interval in milliseconds.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetEOSRefreshIntervalMilliseconds() const;

	/**
	 * @brief Get configuration entry controlling whether encryption is enabled in the auth handler component.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableAuthHandlerEncryption() const;

	/**
	 * @brief Get configuration entry controlling whether maximum chat message limit should be automatically queried
	 * before connection.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetAutoCheckMaximumChatMessageLimit() const;

	/**
	 * @brief Get configuration entry controlling whether polling for match ticket information after a initial delay at
	 * an interval should be enabled.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableMatchTicketPolling() const;

	/**
	 * @brief Get configuration entry controlling the initial delay time in seconds before match ticket polling starts.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetMatchTicketPollStartDelaySeconds() const;

	/**
	 * @brief Get configuration entry controlling the interval time in seconds for match ticket polling.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetMatchTicketPollIntervalSeconds() const;

	/**
	 * @brief Get configuration entry controlling whether polling for server information after a initial delay at an
	 * interval should be enabled.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableServerInfoPolling() const;

	/**
	 * @brief Get configuration entry controlling the initial delay time in seconds before server information polling starts.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetServerInfoPollStartDelaySeconds() const;

	/**
	 * @brief Get configuration entry controlling the interval time in seconds for server information polling.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetServerInfoPollIntervalSeconds() const;

	/**
	 * @brief Get configuration entry controlling whether polling for matchmaking session invites after a initial delay
	 * at an interval should be enabled.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableSessionInvitePolling() const;

	/**
	 * @brief Get configuration entry controlling the initial delay time in seconds before matchmaking session invite
	 * polling starts.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetSessionInvitePollStartDelaySeconds() const;

	/**
	 * @brief Get configuration entry controlling the interval time in seconds for matchmaking session invite polling.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetSessionInvitePollIntervalSeconds() const;

	/**
	 * @brief Get configuration entry controlling whether server registration should be manually initiated by game code.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableManualServerRegistration() const;

	/**
	 * @brief Get configuration entry controlling whether staleness checks on cached user data is enabled.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableStalenessChecking() const;

	/**
	 * @brief Get configuration entry controlling how long cached user data can be held before it is considered stale.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetTimeUntilStaleSeconds() const;

	/**
	 * @brief Get configuration entry controlling whether lobby connection should be gated by an entitlement check.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableEntitlementGateCheck() const;

	/**
	 * @brief Get configuration entry controlling entitlement SKUs that are checked for entitlement gating.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& GetEntitlementGateCheckSkus() const;

	/**
	 * @brief Get configuration entry controlling entitlement item IDs that are checked for entitlement gating.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& GetEntitlementGateCheckItemIds() const;

	/**
	 * @brief Get configuration entry controlling entitlement app IDs that are checked for entitlement gating.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& GetEntitlementGateCheckAppIds() const;

	/**
	 * @brief Get configuration entry controlling the amount of time in seconds between sending telemetry event batches.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetSendTelemetryEventIntervalSeconds() const;

	/**
	 * @brief Get configuration entry controlling whether voice chat is enabled.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetVoiceEnabled() const;

	/**
	 * @brief Get configuration entry controlling threshold in seconds until log in is presented as queued.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetLoginQueuePresentationThresholdSeconds() const;

	/**
	 * @brief Get configuration entry controlling whether log in queue ticket will require manual claim through game code.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<bool>& GetEnableManualLoginQueueClaim() const;

	/**
	 * @brief Get configuration entry controlling interval at which events from the game standard event API get sent.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetGameStandardEventSendIntervalSeconds() const;

	/**
	 * @brief Get configuration entry controlling interval at which events from the predefined event API get sent.
	 */
	const TOnlineSubsystemAccelByteConfigEntry<int64>& GetPredefinedEventSendIntervalSeconds() const;

// Configuration values
private:
	/**
	 * Flag indicating whether this config instance has been initialized already.
	 */
	std::atomic<bool> bInitialized { false };

	/**
	 * @brief Controls whether the lobby websocket should automatically connect after a user logs in. Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> AutoConnectLobbyAfterLoginSuccess {};

	/**
	 * @brief Controls whether the chat websocket should automatically connect after a user logs in. Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> AutoConnectChatAfterLoginSuccess {};

	/**
	 * @brief Controls whether tokens from a native platform (such as Steam) should be refreshed manually by the game
	 * code. Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableManualNativePlatformTokenRefresh {};

	/**
	 * @brief Name of the OSS that we want to treat as the native platform, such as Steam. This is loaded from the
	 * NativePlatformService set in the OnlineSubsystem section of the engine config. Defaults to None.
	 */
	TOnlineSubsystemAccelByteConfigEntry<FName> NativePlatformName {};

	/**
	 * @brief Name of the OSS that we want to treat as the secondary platform, such as Steam. Defaults to None.
	 */
	TOnlineSubsystemAccelByteConfigEntry<FName> SecondaryPlatformName {};

	/**
	 * @brief Controls what platform (such as Steam) should be the source of user display names. Defaults to None,
	 * indicating the display name should come from the AccelByte user data.
	 */
	TOnlineSubsystemAccelByteConfigEntry<EAccelBytePlatformType> DisplayNameSource {};

	/**
	 * @brief Controls whether encryption is enabled for the AccelByte auth handler component. Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableAuthHandlerEncryption {};

	/**
	 * @brief Controls the amount of time in milliseconds that an attempt is made to refresh an EOS token. Defaults to
	 * one minute.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> EOSRefreshIntervalMilliseconds {};

	/**
	 * @brief Controls whether the maximum limit for a chat message should automatically be checked before connection.
	 * Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> AutoCheckMaximumChatMessageLimit {};

	/**
	 * @brief Controls whether match tickets should auto poll after a set delay at an interval. Defaults to true.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableMatchTicketPolling {};

	/**
	 * @brief Controls the delay in seconds at which match ticket polling will start if enabled. Defaults to 30 seconds.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> MatchTicketPollStartDelaySeconds {};

	/**
	* @brief Controls the interval in seconds at which match ticket polling will occur. Defaults to 15 seconds.
	*/
	TOnlineSubsystemAccelByteConfigEntry<int64> MatchTicketPollIntervalSeconds {};

	/**
	 * @brief Controls whether server connection information should auto poll after a set delay at an interval. Defaults
	 * to true.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableServerInfoPolling {};

	/**
	 * @brief Controls the delay in seconds at which server connection information polling will start if enabled.
	 * Defaults to 30 seconds.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> ServerInfoPollStartDelaySeconds {};

	/**
	 * @brief Controls the interval in seconds at which server connection polling will occur. Defaults to 15 seconds.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> ServerInfoPollIntervalSeconds {};

	/**
	 * @brief Controls whether matchmaking related session invites should auto poll after a set delay at an interval.
	 * Defaults to true.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableSessionInvitePolling {};

	/**
	 * @brief Controls the delay in seconds at which matchmaking related session invite polling will start if enabled.
	 * Defaults to 30 seconds.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> SessionInvitePollStartDelaySeconds {};

	/**
	 * @brief Controls the interval in seconds at which matchmaking related session invite polling will occur. Defaults to 15 seconds.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> SessionInvitePollIntervalSeconds {};

	/**
	 * @brief Controls whether server registration should be done manually by the game code. Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableManualServerRegistration {};

	/**
	 * @brief Controls whether staleness checking should be enabled for the user cache. Defaults to true.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableStalenessChecking {};

	/**
	 * @brief Controls whether number of seconds in which a cached user's data will be considered stale. Defaults to ten minutes.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> TimeUntilStaleSeconds {};

	/**
	 * @brief Controls whether gating lobby connection by an entitlement check should be enabled. Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableEntitlementGateCheck {};

	/**
	 * @brief Controls what entitlement SKUs are checked for the entitlement gating feature. Defaults to empty.
	 */
	TOnlineSubsystemAccelByteConfigEntry<TArray<FString>> EntitlementGateCheckSkus {};

	/**
	 * @brief Controls what entitlement item IDs are checked for the entitlement gating feature. Defaults to empty.
	 */
	TOnlineSubsystemAccelByteConfigEntry<TArray<FString>> EntitlementGateCheckItemIds {};

	/**
	 * @brief Controls what entitlement app IDs are checked for the entitlement gating feature. Defaults to empty.
	 */
	TOnlineSubsystemAccelByteConfigEntry<TArray<FString>> EntitlementGateCheckAppIds {};

	/**
	 * @brief Controls the frequency in which batches of telemetry events are sent to the backend. Defaults to one minute.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> SendTelemetryEventIntervalSeconds {};

	/**
	 * @brief Controls whether the voice chat functionality of the AccelByte OSS is enabled. This is loaded from the
	 * bHasVoiceEnabled field in the OnlineSubsystem config section. Default is false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> VoiceEnabled {};

	/**
	 * @brief Controls the amount of time in seconds that a log in request is notified as queued. If estimated wait time
	 * is below this threshold, then no queue notification is presented. Otherwise, the game code will be notified that
	 * log in was queued. Default is zero seconds.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> LoginQueuePresentationThresholdSeconds {};

	/**
	 * @brief Controls whether the log in queue ticket will require manual claim by the game code. Defaults to false.
	 */
	TOnlineSubsystemAccelByteConfigEntry<bool> EnableManualLoginQueueClaim {};

	/**
	 * @brief Controls interval in seconds in which events from the game standard event API are sent. Defaults to one minute.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> GameStandardEventSendIntervalSeconds {};

	/**
	 * @brief Controls interval in seconds in which events from the predefined event API are sent. Defaults to one minute.
	 */
	TOnlineSubsystemAccelByteConfigEntry<int64> PredefinedEventSendIntervalSeconds {};

// Disable copy and move constructors for this class. Only access via shared instances.
public:
	FOnlineSubsystemAccelByteConfig(const FOnlineSubsystemAccelByteConfig&) = delete;
	FOnlineSubsystemAccelByteConfig(FOnlineSubsystemAccelByteConfig&&) = delete;

	FOnlineSubsystemAccelByteConfig& operator=(const FOnlineSubsystemAccelByteConfig&) = delete;
	FOnlineSubsystemAccelByteConfig& operator=(FOnlineSubsystemAccelByteConfig&&) = delete;

};
