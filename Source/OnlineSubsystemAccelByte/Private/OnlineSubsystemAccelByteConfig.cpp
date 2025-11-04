// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSubsystemAccelByteConfig.h"
#include "Core/AccelByteUtilities.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemAccelByteLog.h"

template <typename T>
struct ONLINESUBSYSTEMACCELBYTE_API TOnlineSubsystemAccelByteConfigValueConverter<TArray<T>>
{
	bool ConfigValueFromString(TWeakPtr<FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe>
		, const FString& InString
		, TArray<T>& OutValue)
	{
		// Stubbing out this method for any TArray as those should route to ConfigArrayValueFromStringArray.
		return false;
	}
};

template <>
struct ONLINESUBSYSTEMACCELBYTE_API TOnlineSubsystemAccelByteConfigValueConverter<bool>
{
	bool ConfigValueFromString(TWeakPtr<FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe>
		, const FString& InString
		, bool& OutValue)
	{
		// NOTE: Anything unparsable by ToBool will report as false
		OutValue = InString.ToBool();
		return true;
	}
};

template <>
struct ONLINESUBSYSTEMACCELBYTE_API TOnlineSubsystemAccelByteConfigValueConverter<FString>
{
	bool ConfigValueFromString(TWeakPtr<class FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe>
		, const FString& InString
		, FString& OutValue)
	{
		// #NOTE: Wasteful to do a copy here. Ideally we would just check TIsSame in a constexpr if in SetValueFromString.
		// However, 4.27 uses C++14 by default. When our minimum standard version is 17+, this should be removed.
		OutValue = InString;
		return true;
	}
};

template <>
struct ONLINESUBSYSTEMACCELBYTE_API TOnlineSubsystemAccelByteConfigValueConverter<FName>
{
	bool ConfigValueFromString(TWeakPtr<class FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe>
		, const FString& InString
		, FName& OutValue)
	{
		OutValue = FName { InString };
		return true;
	}
};

template <>
struct TOnlineSubsystemAccelByteConfigValueConverter<EAccelBytePlatformType>
{
	bool ConfigValueFromString(TWeakPtr<FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> WeakConfig
		, const FString& InString
		, EAccelBytePlatformType& OutValue)
	{
		if (InString.Equals(TEXT("default"), ESearchCase::IgnoreCase))
		{
			OutValue = EAccelBytePlatformType::None;
			return true;
		}

		if (InString.Equals(TEXT("native"), ESearchCase::IgnoreCase))
		{
			TSharedPtr<FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> Config = WeakConfig.Pin();
			if (!Config.IsValid())
			{
				return false;
			}

			const FString NativePlatformNameStr = Config->GetNativePlatformName().GetValue().ToString();

			const EAccelBytePlatformType PlatformType =
				FAccelByteUtilities::GetUEnumValueFromString<EAccelBytePlatformType>(NativePlatformNameStr);

			if (PlatformType == EAccelBytePlatformType::None)
			{
				UE_LOG_AB(Warning, TEXT("Unable to get platform type for native platform name: %s"), *NativePlatformNameStr);
			}
		
			OutValue = PlatformType;
			return true;
		}

		const EAccelBytePlatformType PlatformType = FAccelByteUtilities::GetUEnumValueFromString<EAccelBytePlatformType>(InString);
		if (PlatformType == EAccelBytePlatformType::None)
		{
			return false;
		}

		OutValue = PlatformType;
		return true;
	}
};

template <>
struct TOnlineSubsystemAccelByteConfigValueConverter<int64>
{
	bool ConfigValueFromString(TWeakPtr<FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe>
		, const FString& InString
		, int64& OutValue)
	{
		OutValue = FCString::Atoi64(*InString);
		return true;
	}
};

const FString BaseOnlineSubsystemSection = TEXT("OnlineSubsystem");

const FString OnlineSubsystemAccelByteSection = TEXT("OnlineSubsystemAccelByte");

void FOnlineSubsystemAccelByteConfig::Reload()
{
	AutoConnectLobbyAfterLoginSuccess.Load();
	AutoConnectChatAfterLoginSuccess.Load();
	EnableManualNativePlatformTokenRefresh.Load();
	NativePlatformName.Load();
	SecondaryPlatformName.Load();

	// #NOTE: This config entry depends on the NativePlatformName entry when set to "native", always make sure this is
	// loaded after it.
	DisplayNameSource.Load();
	
	EnableAuthHandlerEncryption.Load();
	EOSRefreshIntervalMilliseconds.Load();
	AutoCheckMaximumChatMessageLimit.Load();
	EnableMatchTicketPolling.Load();
	MatchTicketPollStartDelaySeconds.Load();
	MatchTicketPollIntervalSeconds.Load();
	EnableServerInfoPolling.Load();
	ServerInfoPollStartDelaySeconds.Load();
	ServerInfoPollIntervalSeconds.Load();
	EnableSessionInvitePolling.Load();
	SessionInvitePollStartDelaySeconds.Load();
	SessionInvitePollIntervalSeconds.Load();
	EnableManualServerRegistration.Load();
	EnableStalenessChecking.Load();
	TimeUntilStaleSeconds.Load();
	EnableEntitlementGateCheck.Load();
	EntitlementGateCheckSkus.Load();
	EntitlementGateCheckItemIds.Load();
	EntitlementGateCheckAppIds.Load();
	SendTelemetryEventIntervalSeconds.Load();
	VoiceEnabled.Load();
	LoginQueuePresentationThresholdSeconds.Load();
	EnableManualLoginQueueClaim.Load();
	GameStandardEventSendIntervalSeconds.Load();
	PredefinedEventSendIntervalSeconds.Load();
}

void FOnlineSubsystemAccelByteConfig::Reset()
{
	AutoConnectLobbyAfterLoginSuccess.Reset();
	AutoConnectChatAfterLoginSuccess.Reset();
	EnableManualNativePlatformTokenRefresh.Reset();
	NativePlatformName.Reset();
	SecondaryPlatformName.Reset();
	DisplayNameSource.Reset();
	EnableAuthHandlerEncryption.Reset();
	EOSRefreshIntervalMilliseconds.Reset();
	AutoCheckMaximumChatMessageLimit.Reset();
	EnableMatchTicketPolling.Reset();
	MatchTicketPollStartDelaySeconds.Reset();
	MatchTicketPollIntervalSeconds.Reset();
	EnableServerInfoPolling.Reset();
	ServerInfoPollStartDelaySeconds.Reset();
	ServerInfoPollIntervalSeconds.Reset();
	EnableSessionInvitePolling.Reset();
	SessionInvitePollStartDelaySeconds.Reset();
	SessionInvitePollIntervalSeconds.Reset();
	EnableManualServerRegistration.Reset();
	EnableStalenessChecking.Reset();
	TimeUntilStaleSeconds.Reset();
	EnableEntitlementGateCheck.Reset();
	EntitlementGateCheckSkus.Reset();
	EntitlementGateCheckItemIds.Reset();
	EntitlementGateCheckAppIds.Reset();
	SendTelemetryEventIntervalSeconds.Reset();
	VoiceEnabled.Reset();
	LoginQueuePresentationThresholdSeconds.Reset();
	EnableManualLoginQueueClaim.Reset();
	GameStandardEventSendIntervalSeconds.Reset();
	PredefinedEventSendIntervalSeconds.Reset();
}

void FOnlineSubsystemAccelByteConfig::Dump()
{
	UE_LOG_AB(Log, TEXT("OnlineSubsystemAccelByte Configuration Settings:"));

	UE_LOG_AB(Log, TEXT("\tAutoConnectLobbyAfterLoginSuccess: %s"), LOG_BOOL_FORMAT(AutoConnectLobbyAfterLoginSuccess.GetValue()));
	UE_LOG_AB(Log, TEXT("\tAutoConnectChatAfterLoginSuccess: %s"), LOG_BOOL_FORMAT(AutoConnectChatAfterLoginSuccess.GetValue()));
	UE_LOG_AB(Log, TEXT("\tEnableManualNativePlatformTokenRefresh: %s"), LOG_BOOL_FORMAT(EnableManualNativePlatformTokenRefresh.GetValue()));
	UE_LOG_AB(Log, TEXT("\tNativePlatformName: %s"), *NativePlatformName.GetValue().ToString());
	UE_LOG_AB(Log, TEXT("\tSecondaryPlatformName: %s"), *SecondaryPlatformName.GetValue().ToString());
	UE_LOG_AB(Log, TEXT("\tDisplayNameSource: %s"), *FAccelByteUtilities::GetUEnumValueAsString(DisplayNameSource.GetValue()));
	UE_LOG_AB(Log, TEXT("\tEnableAuthHandlerEncryption: %s"), LOG_BOOL_FORMAT(EnableAuthHandlerEncryption.GetValue()));
	UE_LOG_AB(Log, TEXT("\tEOSRefreshIntervalMilliseconds: %lld"), EOSRefreshIntervalMilliseconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tAutoCheckMaximumChatMessageLimit: %s"), LOG_BOOL_FORMAT(AutoCheckMaximumChatMessageLimit.GetValue()));
	UE_LOG_AB(Log, TEXT("\tEnableMatchTicketPolling: %s"), LOG_BOOL_FORMAT(EnableMatchTicketPolling.GetValue()));
	UE_LOG_AB(Log, TEXT("\tMatchTicketPollStartDelaySeconds: %lld"), MatchTicketPollStartDelaySeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tMatchTicketPollIntervalSeconds: %lld"), MatchTicketPollIntervalSeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tEnableServerInfoPolling: %s"), LOG_BOOL_FORMAT(EnableServerInfoPolling.GetValue()));
	UE_LOG_AB(Log, TEXT("\tServerInfoPollStartDelaySeconds: %lld"), ServerInfoPollStartDelaySeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tServerInfoPollIntervalSeconds: %lld"), ServerInfoPollIntervalSeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tEnableSessionInvitePolling: %s"), LOG_BOOL_FORMAT(EnableSessionInvitePolling.GetValue()));
	UE_LOG_AB(Log, TEXT("\tSessionInvitePollStartDelaySeconds: %lld"), SessionInvitePollStartDelaySeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tSessionInvitePollIntervalSeconds: %lld"), SessionInvitePollIntervalSeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tEnableManualServerRegistration: %s"), LOG_BOOL_FORMAT(EnableManualServerRegistration.GetValue()));
	UE_LOG_AB(Log, TEXT("\tEnableStalenessChecking: %s"), LOG_BOOL_FORMAT(EnableStalenessChecking.GetValue()));
	UE_LOG_AB(Log, TEXT("\tTimeUntilStaleSeconds: %lld"), TimeUntilStaleSeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tEnableEntitlementGateCheck: %s"), LOG_BOOL_FORMAT(EnableEntitlementGateCheck.GetValue()));
	UE_LOG_AB(Log, TEXT("\tEntitlementGateCheckSkus: %s"), *FString::Join(EntitlementGateCheckSkus.GetValue(), TEXT(", ")));
	UE_LOG_AB(Log, TEXT("\tEntitlementGateCheckItemIds: %s"), *FString::Join(EntitlementGateCheckItemIds.GetValue(), TEXT(", ")));
	UE_LOG_AB(Log, TEXT("\tEntitlementGateCheckAppIds: %s"), *FString::Join(EntitlementGateCheckAppIds.GetValue(), TEXT(", ")));
	UE_LOG_AB(Log, TEXT("\tSendTelemetryEventIntervalSeconds: %lld"), SendTelemetryEventIntervalSeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tVoiceEnabled: %s"), LOG_BOOL_FORMAT(VoiceEnabled.GetValue()));
	UE_LOG_AB(Log, TEXT("\tLoginQueuePresentationThresholdSeconds: %lld"), LoginQueuePresentationThresholdSeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tEnableManualLoginQueueClaim: %s"), LOG_BOOL_FORMAT(EnableManualLoginQueueClaim.GetValue()));
	UE_LOG_AB(Log, TEXT("\tGameStandardEventSendIntervalSeconds: %lld"), GameStandardEventSendIntervalSeconds.GetValue());
	UE_LOG_AB(Log, TEXT("\tPredefinedEventSendIntervalSeconds: %lld"), PredefinedEventSendIntervalSeconds.GetValue());
}

bool FOnlineSubsystemAccelByteConfig::IsInitialized() const
{
	return bInitialized;
}

void FOnlineSubsystemAccelByteConfig::Init()
{
	if (bInitialized)
	{
		// No need to init again if already initialized, if the config needs to be reloaded, use Reload()
		return;
	}

	// Grab shared reference to self to pass into all config entries
	TSharedRef<FOnlineSubsystemAccelByteConfig, ESPMode::ThreadSafe> ThisRef { SharedThis(this) };

	// Construct all config entries
	AutoConnectLobbyAfterLoginSuccess.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bAutoLobbyConnectAfterLoginSuccess"),
		false
	);

	AutoConnectChatAfterLoginSuccess.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bAutoChatConnectAfterLoginSuccess"),
		false
	);

	EnableManualNativePlatformTokenRefresh.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bNativePlatformTokenRefreshManually"),
		false
	);

	NativePlatformName.Init(
		ThisRef,
		BaseOnlineSubsystemSection,
		TEXT("NativePlatformService"),
		FName{}
	);

	SecondaryPlatformName.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SecondaryPlatformName"),
		FName{}
	);

	DisplayNameSource.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("DisplayNameSource"),
		EAccelBytePlatformType::None
	);

	EnableAuthHandlerEncryption.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("EnabledEncryption"),
		false
	);

	EOSRefreshIntervalMilliseconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("EosRefreshOccurrenceTrackerIntervalMs"),
		60 * 1000, // One minute
		[](const int64& InValue) -> bool {
			// Ensure that the refresh interval is at least 5 seconds, but not more than a minute.
			return InValue >= 5 * 1000 && InValue <= 60 * 1000;
		}
	);

	AutoCheckMaximumChatMessageLimit.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bIsAutoCheckMaximumLimitChatMessage"),
		false
	);

	EnableMatchTicketPolling.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bEnableMatchTicketCheck"),
		true
	);

	MatchTicketPollStartDelaySeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("MatchTicketCheckInitialDelay"),
		30,
		[](const int64& InValue) -> bool {
			// Ensure that the start delay is at least one second.
			return InValue > 0;
		}
	);

	MatchTicketPollIntervalSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("MatchTicketCheckPollInterval"),
		15,
		[](const int64& InValue) -> bool {
			// Ensure that the poll interval at least one second.
			return InValue > 0;
		}
	);

	EnableServerInfoPolling.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bEnableSessionServerCheckPolling"),
		true
	);

	ServerInfoPollStartDelaySeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SessionServerCheckPollInitialDelay"),
		30,
		[](const int64& InValue) -> bool {
			// Ensure that the start delay is at least one second.
			return InValue > 0;
		}
	);

	ServerInfoPollIntervalSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SessionServerCheckPollInterval"),
		15,
		[](const int64& InValue) -> bool {
			// Ensure that the poll interval is at least one second.
			return InValue > 0;
		}
	);

	EnableSessionInvitePolling.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bEnableSessionInviteCheckPolling"),
		true
	);

	SessionInvitePollStartDelaySeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SessionInviteCheckPollInitialDelay"),
		30,
		[](const int64& InValue) -> bool {
			// Ensure that the start delay is at least one second.
			return InValue > 0;
		}
	);

	SessionInvitePollIntervalSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SessionInviteCheckPollInterval"),
		15,
		[](const int64& InValue) -> bool {
			// Ensure that the poll interval is at least one second.
			return InValue > 0;
		}
	);

	EnableManualServerRegistration.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bManualRegisterServer"),
		false
	);

	EnableStalenessChecking.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bEnableStalenessChecking"),
		true
	);

	TimeUntilStaleSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("TimeUntilStaleSeconds"),
		10 * 60, // 10 minutes
		[](const int64& InValue) -> bool {
			// Ensure that the poll interval is at least five minutes, but not more than an hour.
			return InValue >= 5 * 60 && InValue <= 60 * 60;
		}
	);

	EnableEntitlementGateCheck.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bEnableEntitlementGateCheck"),
		false
	);

	EntitlementGateCheckSkus.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("EntitlementGateCheckSkus"),
		{}
	);

	EntitlementGateCheckItemIds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("EntitlementGateCheckItemIds"),
		{}
	);

	EntitlementGateCheckAppIds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("EntitlementGateCheckAppIds"),
		{}
	);

	SendTelemetryEventIntervalSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SendTelemetryEventIntervalInSeconds"),
		1 * 60, // One minute
		[](const int64& InValue) -> bool {
			// Ensure that the interval is at least five seconds, but no more than an hour
			return InValue >= 5 && InValue <= 60 * 60;
		}
	);

	VoiceEnabled.Init(
		ThisRef,
		BaseOnlineSubsystemSection,
		TEXT("bHasVoiceEnabled"),
		false
	);

	LoginQueuePresentationThresholdSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("LoginQueuePresentationThreshold"),
		0
	);

	EnableManualLoginQueueClaim.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("bEnableManualClaimLoginQueue"),
		false
	);

	GameStandardEventSendIntervalSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SendGameStandardEventInterval"),
		1 * 60, // One minute
		[](const int64& InValue) -> bool {
			// Ensure that the send interval is at least five seconds, but no more than an hour
			return InValue >= 5 && InValue <= 60 * 60;
		}
	);

	PredefinedEventSendIntervalSeconds.Init(
		ThisRef,
		OnlineSubsystemAccelByteSection,
		TEXT("SendPredefinedEventInterval"),
		1 * 60, // One minute
		[](const int64& InValue) -> bool {
			// Ensure that the send interval is at least five seconds, but no more than an hour
			return InValue >= 5 && InValue <= 60 * 60;
		}
	);

	// Load all configuration entries
	Reload();

	bInitialized = true;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetAutoConnectLobbyAfterLoginSuccess()
{
	return AutoConnectLobbyAfterLoginSuccess;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetAutoConnectChatAfterLoginSuccess()
{
	return AutoConnectChatAfterLoginSuccess;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableManualNativePlatformTokenRefresh()
{
	return EnableManualNativePlatformTokenRefresh;
}

TOnlineSubsystemAccelByteConfigEntry<EAccelBytePlatformType>& FOnlineSubsystemAccelByteConfig::GetDisplayNameSource()
{
	return DisplayNameSource;
}

TOnlineSubsystemAccelByteConfigEntry<FName>& FOnlineSubsystemAccelByteConfig::GetNativePlatformName()
{
	return NativePlatformName;
}

TOnlineSubsystemAccelByteConfigEntry<FName>& FOnlineSubsystemAccelByteConfig::GetSecondaryPlatformName()
{
	return SecondaryPlatformName;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetEOSRefreshIntervalMilliseconds()
{
	return EOSRefreshIntervalMilliseconds;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableAuthHandlerEncryption()
{
	return EnableAuthHandlerEncryption;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetAutoCheckMaximumChatMessageLimit()
{
	return AutoCheckMaximumChatMessageLimit;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableMatchTicketPolling()
{
	return EnableMatchTicketPolling;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetMatchTicketPollStartDelaySeconds()
{
	return MatchTicketPollStartDelaySeconds;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetMatchTicketPollIntervalSeconds()
{
	return MatchTicketPollIntervalSeconds;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableServerInfoPolling()
{
	return EnableServerInfoPolling;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetServerInfoPollStartDelaySeconds()
{
	return ServerInfoPollStartDelaySeconds;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetServerInfoPollIntervalSeconds()
{
	return ServerInfoPollIntervalSeconds;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableSessionInvitePolling()
{
	return EnableSessionInvitePolling;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetSessionInvitePollStartDelaySeconds()
{
	return SessionInvitePollStartDelaySeconds;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetSessionInvitePollIntervalSeconds()
{
	return SessionInvitePollIntervalSeconds;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableManualServerRegistration()
{
	return EnableManualServerRegistration;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableStalenessChecking()
{
	return EnableStalenessChecking;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetTimeUntilStaleSeconds()
{
	return TimeUntilStaleSeconds;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableEntitlementGateCheck()
{
	return EnableEntitlementGateCheck;
}

TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& FOnlineSubsystemAccelByteConfig::GetEntitlementGateCheckSkus()
{
	return EntitlementGateCheckSkus;
}

TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& FOnlineSubsystemAccelByteConfig::GetEntitlementGateCheckItemIds()
{
	return EntitlementGateCheckItemIds;
}

TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& FOnlineSubsystemAccelByteConfig::GetEntitlementGateCheckAppIds()
{
	return EntitlementGateCheckAppIds;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetSendTelemetryEventIntervalSeconds()
{
	return SendTelemetryEventIntervalSeconds;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetVoiceEnabled()
{
	return VoiceEnabled;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetLoginQueuePresentationThresholdSeconds()
{
	return LoginQueuePresentationThresholdSeconds;
}

TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableManualLoginQueueClaim()
{
	return EnableManualLoginQueueClaim;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetGameStandardEventSendIntervalSeconds()
{
	return GameStandardEventSendIntervalSeconds;
}

TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetPredefinedEventSendIntervalSeconds()
{
	return PredefinedEventSendIntervalSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetAutoConnectLobbyAfterLoginSuccess() const
{
	return AutoConnectLobbyAfterLoginSuccess;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetAutoConnectChatAfterLoginSuccess() const
{
	return AutoConnectChatAfterLoginSuccess;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableManualNativePlatformTokenRefresh() const
{
	return EnableManualNativePlatformTokenRefresh;
}

const TOnlineSubsystemAccelByteConfigEntry<EAccelBytePlatformType>& FOnlineSubsystemAccelByteConfig::GetDisplayNameSource() const
{
	return DisplayNameSource;
}

const TOnlineSubsystemAccelByteConfigEntry<FName>& FOnlineSubsystemAccelByteConfig::GetNativePlatformName() const
{
	return NativePlatformName;
}

const TOnlineSubsystemAccelByteConfigEntry<FName>& FOnlineSubsystemAccelByteConfig::GetSecondaryPlatformName() const
{
	return SecondaryPlatformName;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetEOSRefreshIntervalMilliseconds() const
{
	return EOSRefreshIntervalMilliseconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableAuthHandlerEncryption() const
{
	return EnableAuthHandlerEncryption;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetAutoCheckMaximumChatMessageLimit() const
{
	return AutoCheckMaximumChatMessageLimit;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableMatchTicketPolling() const
{
	return EnableMatchTicketPolling;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetMatchTicketPollStartDelaySeconds() const
{
	return MatchTicketPollStartDelaySeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetMatchTicketPollIntervalSeconds() const
{
	return MatchTicketPollIntervalSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableServerInfoPolling() const
{
	return EnableServerInfoPolling;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetServerInfoPollStartDelaySeconds() const
{
	return ServerInfoPollStartDelaySeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetServerInfoPollIntervalSeconds() const
{
	return ServerInfoPollIntervalSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableSessionInvitePolling() const
{
	return EnableSessionInvitePolling;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetSessionInvitePollStartDelaySeconds() const
{
	return SessionInvitePollStartDelaySeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetSessionInvitePollIntervalSeconds() const
{
	return SessionInvitePollIntervalSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableManualServerRegistration() const
{
	return EnableManualServerRegistration;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableStalenessChecking() const
{
	return EnableStalenessChecking;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetTimeUntilStaleSeconds() const
{
	return TimeUntilStaleSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableEntitlementGateCheck() const
{
	return EnableEntitlementGateCheck;
}

const TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& FOnlineSubsystemAccelByteConfig::GetEntitlementGateCheckSkus() const
{
	return EntitlementGateCheckSkus;
}

const TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& FOnlineSubsystemAccelByteConfig::GetEntitlementGateCheckItemIds() const
{
	return EntitlementGateCheckItemIds;
}

const TOnlineSubsystemAccelByteConfigEntry<TArray<FString>>& FOnlineSubsystemAccelByteConfig::GetEntitlementGateCheckAppIds() const
{
	return EntitlementGateCheckAppIds;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetSendTelemetryEventIntervalSeconds() const
{
	return SendTelemetryEventIntervalSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetVoiceEnabled() const
{
	return VoiceEnabled;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetLoginQueuePresentationThresholdSeconds() const
{
	return LoginQueuePresentationThresholdSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<bool>& FOnlineSubsystemAccelByteConfig::GetEnableManualLoginQueueClaim() const
{
	return EnableManualLoginQueueClaim;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetGameStandardEventSendIntervalSeconds() const
{
	return GameStandardEventSendIntervalSeconds;
}

const TOnlineSubsystemAccelByteConfigEntry<int64>& FOnlineSubsystemAccelByteConfig::GetPredefinedEventSendIntervalSeconds() const
{
	return PredefinedEventSendIntervalSeconds;
}
