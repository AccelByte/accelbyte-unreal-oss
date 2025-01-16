// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineExternalUIInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemUtils.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineExternalUIAccelByte"

bool FOnlineExternalUIAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineExternalUIAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineExternalUIAccelByte>(Subsystem->GetExternalUIInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineExternalUIAccelByte::GetFromWorld(const UWorld* World, FOnlineExternalUIAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

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

	TSharedRef<const FUniqueNetIdAccelByteUser> RequestorCompositeId = FUniqueNetIdAccelByteUser::CastChecked(Requestor);
	TSharedRef<const FUniqueNetIdAccelByteUser> RequesteeCompositeId = FUniqueNetIdAccelByteUser::CastChecked(Requestee);

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

	TSharedRef<const FUniqueNetIdAccelByteUser> CompositeUniqueId = FUniqueNetIdAccelByteUser::CastChecked(UniqueId);
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

FOnlineExternalUIAccelByte::FOnlineExternalUIAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
	{
	}

IOnlineExternalUIPtr FOnlineExternalUIAccelByte::GetNativePlatformExternalUI()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return nullptr;
	}
	
	IOnlineSubsystem* NativeSubsystem = AccelByteSubsystemPtr->GetNativePlatformSubsystem();
	if (NativeSubsystem == nullptr)
	{
		return nullptr;
	}

	return NativeSubsystem->GetExternalUIInterface();
}

#undef ONLINE_ERROR_NAMESPACE
