// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlinePresenceInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Online.h"
#include "Core/AccelByteMultiRegistry.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserPresence.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteSetUserPresence.h"

FOnlinePresenceAccelByte::FOnlinePresenceAccelByte(FOnlineSubsystemAccelByte* InSubsystem) 
	: AccelByteSubsystem(InSubsystem)
{
}

IOnlinePresencePtr FOnlinePresenceAccelByte::GetPlatformOnlinePresenceInterface() const 
{
	IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	return (NativeSubsystem != nullptr) ? NativeSubsystem->GetPresenceInterface() : nullptr;
}

void FOnlinePresenceAccelByte::SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate) 
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(User);

		if (UserAccount.IsValid())
		{
			const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
			if(UserAccountAccelByte->IsConnectedToLobby())
			{
				// Async task to set presence from AccelByte backend
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSetUserPresence>(AccelByteSubsystem, User, Status, Delegate);
				return;
			}
		}
	}
	Delegate.ExecuteIfBound(User, false);
}

void FOnlinePresenceAccelByte::QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate) 
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		int32 LocalUserNum = IdentityInterface->GetLocalUserNumCached();

		// Async task to query presence from AccelByte backend
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserPresence>(AccelByteSubsystem, User, Delegate, LocalUserNum);
	}
}

EOnlineCachedResult::Type FOnlinePresenceAccelByte::GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) 
{
	TSharedRef<const FUniqueNetIdAccelByteUser> CompositeId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(User.AsShared());
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
	if (AccelByteSubsystem->GetAppId() == AppId)
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

TSharedRef<FOnlineUserPresenceAccelByte> FOnlinePresenceAccelByte::FindOrCreatePresence(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId) 
{
	TSharedRef<FOnlineUserPresenceAccelByte>* UserPresence = CachedPresenceByUserId.Find(UserId->GetAccelByteId());
	if (UserPresence == nullptr)
	{
		UserPresence = &(CachedPresenceByUserId.Add(UserId->GetAccelByteId(), MakeShared<FOnlineUserPresenceAccelByte>()));
	}

	return *UserPresence;
}
