// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlinePresenceInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Online.h"

#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserPresence.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteSetUserPresence.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteBulkQueryUserPresence.h"

FOnlinePresenceAccelByte::FOnlinePresenceAccelByte(FOnlineSubsystemAccelByte* InSubsystem) 
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{
}

bool FOnlinePresenceAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlinePresenceAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlinePresenceAccelByte>(Subsystem->GetPresenceInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlinePresenceAccelByte::GetFromWorld(const UWorld* World, FOnlinePresenceAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

IOnlinePresencePtr FOnlinePresenceAccelByte::GetPlatformOnlinePresenceInterface() const 
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return nullptr;
	}
	
	IOnlineSubsystem* NativeSubsystem = AccelByteSubsystemPtr->GetNativePlatformSubsystem();
	return (NativeSubsystem != nullptr) ? NativeSubsystem->GetPresenceInterface() : nullptr;
}

void FOnlinePresenceAccelByte::OnFriendStatusChangedNotificationReceived(const FAccelByteModelsUsersPresenceNotice& Notification, int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	// First, we want to get our own net ID, as delegates will require it
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Received a notification for a friend status changed, but cannot act on it as the identity interface is not valid!"));
		return;
	}

	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Received a notification for a friend status changed, but cannot act on it as the current user's ID is not valid!"));
		return;
	}

	// Next, we assume that we don't have this user in our friends list already as they just sent us an invite, so just
	// create an async task to get data about that friend and then add them to the list afterwards, and fire off the delegates
	FAccelByteUniqueIdComposite FriendCompositeId;
	FriendCompositeId.Id = Notification.UserID;
	FUniqueNetIdAccelByteUserRef FriendId = FUniqueNetIdAccelByteUser::Create(FriendCompositeId);

	TSharedPtr<FOnlineUserPresenceAccelByte> FriendPresence = FindOrCreatePresence(FriendId);
	
	FOnlineUserPresenceStatusAccelByte PresenceStatus;

	PresenceStatus.StatusStr = Notification.Activity;
	PresenceStatus.SetPresenceStatus(Notification.Availability);

	FriendPresence->Status = PresenceStatus;
	FriendPresence->bIsOnline = Notification.Availability == EAvailability::Online ? true : false;
	FriendPresence->bIsPlayingThisGame = Notification.Availability == EAvailability::Online ? true : false;
	FriendPresence->LastOnline = Notification.LastSeenAt;
	FriendPresence->Status.Properties.Add(DefaultPlatformKey, Notification.Platform);

	TriggerOnPresenceReceivedDelegates(*FriendId, FriendPresence.ToSharedRef());
}

TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>> FOnlinePresenceAccelByte::GetCachedPresence() const
{
	return CachedPresenceByUserId;
}

void FOnlinePresenceAccelByte::SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate) 
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(User);

		if (UserAccount.IsValid())
		{
			const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
			if(UserAccountAccelByte->IsConnectedToLobby())
			{
				// Async task to set presence from AccelByte backend
				AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSetUserPresence>(AccelByteSubsystemPtr.Get(), User, Status, Delegate);
				return;
			}
		}
	}
	Delegate.ExecuteIfBound(User, false);
}

void FOnlinePresenceAccelByte::UpdatePresenceCache(const TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>>& NewPresences)
{
	FScopeLock ScopeLock(&PresenceCacheLock);
	
	CachedPresenceByUserId.Append(NewPresences);
}

void FOnlinePresenceAccelByte::UpdatePresenceCache(const FString& UserID, const TSharedRef<FOnlineUserPresenceAccelByte>& Presence)
{
	FScopeLock ScopeLock(&PresenceCacheLock);
	
	CachedPresenceByUserId.Emplace(UserID, Presence);
}

void FOnlinePresenceAccelByte::QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate) 
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	int32 LocalUserNum = AccelByteSubsystemPtr->GetLocalUserNumCached();

	// Async task to query presence from AccelByte backend
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserPresence>(AccelByteSubsystemPtr.Get(), User, Delegate, LocalUserNum);
}

void FOnlinePresenceAccelByte::BulkQueryPresence(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& UserIds)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkQueryUserPresence>(AccelByteSubsystemPtr.Get(), LocalUserId, UserIds);
}

EOnlineCachedResult::Type FOnlinePresenceAccelByte::GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) 
{
	TSharedRef<const FUniqueNetIdAccelByteUser> CompositeId = FUniqueNetIdAccelByteUser::CastChecked(User);
	TSharedRef<FOnlineUserPresenceAccelByte>* FoundPresence = CachedPresenceByUserId.Find(CompositeId->GetAccelByteId());
	if (FoundPresence != nullptr)
	{
		OutPresence = *FoundPresence;
		return EOnlineCachedResult::Success;
	}

	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlinePresenceAccelByte::GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence>& OutPresence) 
{
	EOnlineCachedResult::Type Result = EOnlineCachedResult::NotFound;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return Result;
	}
	
	if (AccelByteSubsystemPtr->GetAppId() == AppId)
	{
		Result = GetCachedPresence(User, OutPresence);
	}

	return Result;
}

void FOnlinePresenceAccelByte::PlatformQueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate) 
{
	IOnlinePresencePtr NativePresenceInterface = GetPlatformOnlinePresenceInterface();
	if (!NativePresenceInterface.IsValid())
	{
		return;
	}

	// #NOTE (Maxwell): This should be the native platform ID itself, so just pass this in directly
	NativePresenceInterface->QueryPresence(User, Delegate);
}

EOnlineCachedResult::Type FOnlinePresenceAccelByte::GetPlatformCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) 
{
	IOnlinePresencePtr NativePresenceInterface = GetPlatformOnlinePresenceInterface();
	if (!NativePresenceInterface.IsValid())
	{
		return EOnlineCachedResult::NotFound;
	}

	// #NOTE (Maxwell): This should be the native platform ID itself, so just pass this in directly
	return NativePresenceInterface->GetCachedPresence(User, OutPresence);
}

void FOnlinePresenceAccelByte::RegisterRealTimeLobbyDelegates(int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	// Get our identity interface to retrieve the API client for this user
	AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
	if (!ApiClient.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to register real-time lobby as an Api client could not be retrieved for user num %d!"), LocalUserNum);
		return;
	}

	const auto Lobby = ApiClient->GetLobbyApi().Pin();
	if (!Lobby.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to register real-time lobby as an Lobby Api could not be retrieved for user num %d!"), LocalUserNum);
		return;
	}

	// Set each delegate for the corresponding API client to be a new realtime delegate
	AccelByte::Api::Lobby::FFriendStatusNotif OnFriendStatusChangedNotificationReceivedDelegate = AccelByte::Api::Lobby::FFriendStatusNotif::CreateThreadSafeSP(AsShared(), &FOnlinePresenceAccelByte::OnFriendStatusChangedNotificationReceived, LocalUserNum);
	Lobby->SetUserPresenceNotifDelegate(OnFriendStatusChangedNotificationReceivedDelegate);
}

TSharedRef<FOnlineUserPresenceAccelByte> FOnlinePresenceAccelByte::FindOrCreatePresence(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId) 
{
	TSharedRef<FOnlineUserPresenceAccelByte>* UserPresence = CachedPresenceByUserId.Find(UserId->GetAccelByteId());
	if (UserPresence == nullptr)
	{
		UserPresence = &(CachedPresenceByUserId.Add(UserId->GetAccelByteId(), MakeShared<FOnlineUserPresenceAccelByte>()));
	}

	return *UserPresence;
}
