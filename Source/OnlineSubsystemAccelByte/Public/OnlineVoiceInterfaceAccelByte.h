// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "VoiceInterfaceImpl.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelBytePackage.h"

class ONLINESUBSYSTEMACCELBYTE_API FOnlineVoiceAccelByte
	: public FOnlineVoiceImpl
	, public TSharedFromThis<FOnlineVoiceAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	FOnlineVoiceAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

public:
	virtual bool Init() override;
	/**
	 * Registers array of  the unique player ids as a talker
	 *
	 * @param Player ids Players to add
	 */
	void RegisterTalkers(const TArray<TSharedRef<const FUniqueNetId>>& Players);

	/**
	 * Registers the unique player id as a talker
	 *
	 * @param Player Player to add
	 */
	void RegisterTalker(const FUniqueNetIdRef Player);

	/**
	 * Registers the unique player id as a talker with session name
	 *
	 * @param Player Player to add
	 * @param SessionName Session name
	 */
	void RegisterTalker(const FUniqueNetIdRef Player, const FNamedOnlineSession& SessionName);

	/**
	 * Remove all talker
	 */
	void RemoveAllTalkers();

	/**
	* @brief Check if voice chat is enabled
	*
	* @return true if voice chat is enabled
	*/
	bool IsVoiceEnabled();

protected:
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	FOnlineVoiceAccelByte() = delete;
	int32 GetPlayerTeamIndex(TArray<FAccelByteModelsV2GameSessionTeam> Teams, const FUniqueNetIdRef UserId);
	bool bInitialized = false;
	bool bIsEnabled = false;
};
