// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSubsystemAccelBytePackage.h"

class FOnlineSubsystemAccelByte;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineExternalUIAccelByte : public IOnlineExternalUI
{
public:

	virtual ~FOnlineExternalUIAccelByte() = default;

	//~ Begin Override IOnlineExternalUI
	virtual bool ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate = FOnLoginUIClosedDelegate()) override;
	virtual bool ShowAccountCreationUI(const int ControllerIndex, const FOnAccountCreationUIClosedDelegate& Delegate = FOnAccountCreationUIClosedDelegate()) override;
	virtual bool ShowFriendsUI(int32 LocalUserNum) override;
	virtual bool ShowInviteUI(int32 LocalUserNum, FName SessionName = NAME_GameSession) override;
	virtual bool ShowAchievementsUI(int32 LocalUserNum) override;
	virtual bool ShowLeaderboardUI(const FString& LeaderboardName) override;
	virtual bool ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate = FOnShowWebUrlClosedDelegate()) override;
	virtual bool CloseWebURL() override;
	virtual bool ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate = FOnProfileUIClosedDelegate()) override;
	virtual bool ShowAccountUpgradeUI(const FUniqueNetId& UniqueId) override;
	virtual bool ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate = FOnShowStoreUIClosedDelegate()) override;
	virtual bool ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate = FOnShowSendMessageUIClosedDelegate()) override;
	//~ End Override IOnlineExternalUI

PACKAGE_SCOPE:

	/**
	 * Constructs an instance of our external UI interface implementation.
	 *
	 * @param InSubsystem AccelByte subsystem instance that owns this external UI instance
	 */
	explicit FOnlineExternalUIAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
		: AccelByteSubsystem(InSubsystem)
	{
	}

private:

	/** Subsystem that owns this external UI interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem;

	/**
	 * Attempt to get the external UI interface for the native platform of the user
	 */
	IOnlineExternalUIPtr GetNativePlatformExternalUI();

};

typedef TSharedPtr<FOnlineExternalUIAccelByte, ESPMode::ThreadSafe> FOnlineExternalUIAccelBytePtr;
