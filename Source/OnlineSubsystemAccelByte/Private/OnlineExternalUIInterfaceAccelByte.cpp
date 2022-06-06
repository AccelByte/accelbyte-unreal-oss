// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineExternalUIInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "Interfaces/OnlineIdentityInterface.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineExternalUIAccelByte"

bool FOnlineExternalUIAccelByte::ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowLoginUI(ControllerIndex, bShowOnlineOnly, bShowSkipButton, Delegate);
}

bool FOnlineExternalUIAccelByte::ShowAccountCreationUI(const int ControllerIndex, const FOnAccountCreationUIClosedDelegate& Delegate)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowAccountCreationUI(ControllerIndex, Delegate);
}

bool FOnlineExternalUIAccelByte::ShowFriendsUI(int32 LocalUserNum)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowFriendsUI(LocalUserNum);
}

bool FOnlineExternalUIAccelByte::ShowInviteUI(int32 LocalUserNum, FName SessionName)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowInviteUI(LocalUserNum, SessionName);
}

bool FOnlineExternalUIAccelByte::ShowAchievementsUI(int32 LocalUserNum)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowAchievementsUI(LocalUserNum);
}

bool FOnlineExternalUIAccelByte::ShowLeaderboardUI(const FString& LeaderboardName)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowLeaderboardUI(LeaderboardName);
}

bool FOnlineExternalUIAccelByte::ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowWebURL(Url, ShowParams, Delegate);
}

bool FOnlineExternalUIAccelByte::CloseWebURL()
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->CloseWebURL();
}

bool FOnlineExternalUIAccelByte::ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	TSharedRef<const FUniqueNetIdAccelByteUser> RequestorCompositeId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Requestor.AsShared());
	TSharedRef<const FUniqueNetIdAccelByteUser> RequesteeCompositeId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Requestee.AsShared());

	if (!RequesteeCompositeId->HasPlatformInformation() || !RequestorCompositeId->HasPlatformInformation())
	{
		return false;
	}

	if (!RequestorCompositeId->GetPlatformUniqueId() || !RequesteeCompositeId->GetPlatformUniqueId())
	{
		return false;
	}

	return NativeExternalUI->ShowProfileUI(RequestorCompositeId->GetPlatformUniqueId().ToSharedRef().Get(), RequesteeCompositeId->GetPlatformUniqueId().ToSharedRef().Get(), Delegate);
}

bool FOnlineExternalUIAccelByte::ShowAccountUpgradeUI(const FUniqueNetId& UniqueId)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	TSharedRef<const FUniqueNetIdAccelByteUser> CompositeUniqueId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UniqueId.AsShared());
	if (!CompositeUniqueId->HasPlatformInformation())
	{
		return false;
	}

	return NativeExternalUI->ShowAccountUpgradeUI(CompositeUniqueId->GetPlatformUniqueId().ToSharedRef().Get());
}

bool FOnlineExternalUIAccelByte::ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowStoreUI(LocalUserNum, ShowParams, Delegate);
}

bool FOnlineExternalUIAccelByte::ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate)
{
	IOnlineExternalUIPtr NativeExternalUI = GetNativePlatformExternalUI();
	if (!NativeExternalUI.IsValid())
	{
		return false;
	}

	return NativeExternalUI->ShowSendMessageUI(LocalUserNum, ShowParams, Delegate);
}

IOnlineExternalUIPtr FOnlineExternalUIAccelByte::GetNativePlatformExternalUI()
{
	IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		return nullptr;
	}

	return NativeSubsystem->GetExternalUIInterface();
}

#undef ONLINE_ERROR_NAMESPACE
