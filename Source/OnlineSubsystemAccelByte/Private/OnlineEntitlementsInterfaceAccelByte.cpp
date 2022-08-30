﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineEntitlementsInterfaceAccelByte.h"

#include "AsyncTasks/OnlineAsyncTaskAccelByteQueryEntitlements.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteSyncPlatformPurchase.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteSyncDLC.h"

FOnlineEntitlementsAccelByte::FOnlineEntitlementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
{
}

void FOnlineEntitlementsAccelByte::AddEntitlementToMap(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, TSharedRef<FOnlineEntitlement> Entitlement)
{
	FScopeLock ScopeLock(&EntitlementMapLock);
	FEntitlementMap& EntMap = EntitlementMap.FindOrAdd(UserId);
	FItemEntitlementMap& ItemEntMap = ItemEntitlementMap.FindOrAdd(UserId);
	
	EntMap.Emplace(Entitlement->Id, Entitlement);
	ItemEntMap.Emplace(Entitlement->ItemId, Entitlement);
}

TSharedPtr<FOnlineEntitlement> FOnlineEntitlementsAccelByte::GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
	FScopeLock ScopeLock(&EntitlementMapLock);
	FEntitlementMap* EntMapPtr = EntitlementMap.Find(SharedUserId);
	if(EntMapPtr)
	{
		TSharedRef<FOnlineEntitlement>* Result = EntMapPtr->Find(EntitlementId);
		if(Result)
		{
			return *Result;
		}
	}
	return nullptr;
}

TSharedPtr<FOnlineEntitlement> FOnlineEntitlementsAccelByte::GetItemEntitlement(const FUniqueNetId& UserId, const FString& ItemId)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
	FScopeLock ScopeLock(&EntitlementMapLock);
	FItemEntitlementMap* EntMapPtr = ItemEntitlementMap.Find(SharedUserId);
	if(EntMapPtr)
	{
		TSharedRef<FOnlineEntitlement>* Result = EntMapPtr->Find(ItemId);
		if(Result)
		{
			return *Result;
		}
	}
	return nullptr;
}

void FOnlineEntitlementsAccelByte::GetAllEntitlements(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineEntitlement>>& OutUserEntitlements)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
	FScopeLock ScopeLock(&EntitlementMapLock);
	FEntitlementMap* EntMapPtr = EntitlementMap.Find(SharedUserId);
	if(EntMapPtr)
	{
		EntMapPtr->GenerateValueArray(OutUserEntitlements);
	}
}

bool FOnlineEntitlementsAccelByte::QueryEntitlements(const FUniqueNetId& UserId, const FString& Namespace, const FPagedQuery& Page)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryEntitlements>(AccelByteSubsystem, UserId, Namespace, Page);
	return true;
}

void FOnlineEntitlementsAccelByte::SyncPlatformPurchase(int32 LocalUserNum, FAccelByteModelsEntitlementSyncBase EntitlementSyncBase, const FOnRequestCompleted& CompletionDelegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncPlatformPurchase>(AccelByteSubsystem, LocalUserNum, EntitlementSyncBase, CompletionDelegate);
}

void FOnlineEntitlementsAccelByte::SyncDLC(const FUniqueNetId& InLocalUserId, const FOnRequestCompleted& CompletionDelegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncDLC>(AccelByteSubsystem, InLocalUserId, CompletionDelegate);
}