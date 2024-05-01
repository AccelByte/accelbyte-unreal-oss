// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteQueryEntitlements.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteConsumeEntitlement.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncPlatformPurchase.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncDLC.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteGetUserEntitlementHistory.h"

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

void FOnlineEntitlementsAccelByte::AddUserEntitlementHistoryToMap(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId
	, const FUniqueEntitlementId& EntitlementId
	, TArray<FAccelByteModelsUserEntitlementHistory> UserEntitlementHistory)
{
	FScopeLock ScopeLock(&EntitlementMapLock);

	FEntitlementHistory& EntitlementHistoryMap = UserEntitlementHistoryMap.FindOrAdd(UserId);

	EntitlementHistoryMap.Emplace(EntitlementId, UserEntitlementHistory);
}

bool FOnlineEntitlementsAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineEntitlementsAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineEntitlementsAccelByte::GetFromWorld(const UWorld* World, FOnlineEntitlementsAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

TSharedPtr<FOnlineEntitlement> FOnlineEntitlementsAccelByte::GetEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
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
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
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
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
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

void FOnlineEntitlementsAccelByte::ConsumeEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId, int32 UseCount, TArray<FString> Options, const FString& RequestId)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteConsumeEntitlement>(AccelByteSubsystem, UserId, EntitlementId, UseCount, Options, RequestId);
}

void FOnlineEntitlementsAccelByte::GetUserEntitlementHistory(int32 LocalUserNum, const FUniqueEntitlementId& EntitlementId, bool bForceUpdate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserEntitlementHistory>(AccelByteSubsystem, LocalUserNum, EntitlementId, bForceUpdate);
}

bool FOnlineEntitlementsAccelByte::GetCachedUserEntitlementHistory(const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FUniqueEntitlementId& InEntitlementId, TArray<FAccelByteModelsUserEntitlementHistory>& OutEntitlementHistory)
{
	FScopeLock ScopeLock(&EntitlementMapLock);

	if (!InUserId->IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to get entitlement history from cache as user is invalid"));

		return false;
	}

	FEntitlementHistory& TempUserEntitlementHistoriesMap = *UserEntitlementHistoryMap.Find(InUserId);

	if (TempUserEntitlementHistoriesMap.Num() == 0)
	{
		UE_LOG_AB(Warning, TEXT("Failed to get entitlement history from cache as there are no entitlement histories stored for this user"));

		return false;
	}

	TArray<FAccelByteModelsUserEntitlementHistory>& UserEntitlementHistories = *TempUserEntitlementHistoriesMap.Find(InEntitlementId);

	if (UserEntitlementHistories.Num() == 0)
	{
		UE_LOG_AB(Warning, TEXT("Failed to get entitlement history from cache as there are no history for this entitlement"));

		return false;
	}

	OutEntitlementHistory = UserEntitlementHistories;

	return true;
}

bool FOnlineEntitlementsAccelByte::DeleteCachedUserEntitlementHistory(const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FUniqueEntitlementId& InEntitlementId)
{
	FScopeLock ScopeLock(&EntitlementMapLock);

	if (!InUserId->IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to delete entitlement history from cache as user is invalid"));

		return false;
	}

	FEntitlementHistory& TempUserEntitlementHistoriesMap = *UserEntitlementHistoryMap.Find(InUserId);

	if (TempUserEntitlementHistoriesMap.Num() == 0)
	{
		UE_LOG_AB(Warning, TEXT("Failed to delete entitlement history from cache as there are no entitlement histories stored for this user"));

		return false;
	}

	TempUserEntitlementHistoriesMap.Remove(InEntitlementId);

	return true;
}
