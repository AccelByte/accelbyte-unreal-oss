// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteQueryEntitlements.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteQueryPlatformSubscription.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteConsumeEntitlement.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncPlatformPurchase.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncDLC.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteGetUserEntitlementHistory.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncPlatformGooglePlay.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncIOSAppStore.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncMetaQuestDLC.h"
#include "AsyncTasks/Entitlements/OnlineAsyncTaskAccelByteSyncMetaQuestIAP.h"

#pragma region FOnlineEntitlementAccelByte Methods
bool FOnlineEntitlementAccelByte::GetAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	if(!ExtraAttributes.Contains(AttrName))
	{
		return false;
	}

	OutAttrValue = ExtraAttributes[AttrName];
	return true;
}

FAccelByteModelsEntitlementInfo FOnlineEntitlementAccelByte::GetBackendEntitlementInfo() const
{
	return BackendEntitlementInfo;
}

bool FOnlineEntitlementAccelByte::SetAttribute(const FString& AttrName, const FString& AttrValue)
{
	ExtraAttributes.Emplace(AttrName, AttrValue);

	return true;
}

bool FOnlineEntitlementAccelByte::SetBackendEntitlementInfo(const FAccelByteModelsEntitlementInfo Info)
{
	BackendEntitlementInfo = Info;
	SetAttribute(ENTITLEMENT_ATTR_KEY_SKU, Info.Sku);
	return true;
}

FOnlineEntitlementAccelByte::~FOnlineEntitlementAccelByte()
{}
#pragma endregion 

FOnlineEntitlementsAccelByte::FOnlineEntitlementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
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

void FOnlineEntitlementsAccelByte::AddUserEntitlementHistoryToMap(const FUniqueNetIdAccelByteUserRef& UserId
	, const FUniqueEntitlementId& EntitlementId
	, TArray<FAccelByteModelsUserEntitlementHistory> UserEntitlementHistory)
{
	FScopeLock ScopeLock(&EntitlementMapLock);

	FEntitlementHistory& EntitlementHistoryMap = UserEntitlementHistoryMap.FindOrAdd(UserId);

	EntitlementHistoryMap.Emplace(EntitlementId, UserEntitlementHistory);
}

void FOnlineEntitlementsAccelByte::AddCurrentUserEntitlementHistoryToMap(const FUniqueNetIdAccelByteUserRef& UserId
	, TArray<FAccelByteModelsBaseUserEntitlementHistory> CurrentUserEntitlementHistory)
{
	FScopeLock ScopeLock(&EntitlementMapLock);

	// Get current user id
	FCurrentUserEntitlementHistory& EntitlementHistoryMap = CurrentUserEntitlementHistoryMap.FindOrAdd(UserId);

	// Extract each available entitlement id and group them
	TMap<FString, TArray<FAccelByteModelsBaseUserEntitlementHistory>> TempMapEntitlementHistory = ExtractUserEntitlementId(CurrentUserEntitlementHistory);

	for (const auto& CurrentHistory : TempMapEntitlementHistory)
	{
		EntitlementHistoryMap.Emplace(CurrentHistory.Key, CurrentHistory.Value);
	}
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
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryEntitlements>(AccelByteSubsystemPtr.Get(), UserId, Namespace, Page);
	return true;
}

void FOnlineEntitlementsAccelByte::ConsumeEntitlement(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId, int32 UseCount, TArray<FString> Options, const FString& RequestId)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteConsumeEntitlement>(AccelByteSubsystemPtr.Get(), UserId, EntitlementId, UseCount, Options, RequestId);
}

void FOnlineEntitlementsAccelByte::SyncPlatformPurchase(int32 LocalUserNum, FAccelByteModelsEntitlementSyncBase EntitlementSyncBase, const FOnRequestCompleted& CompletionDelegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncPlatformPurchase>(AccelByteSubsystemPtr.Get(), LocalUserNum, EntitlementSyncBase, CompletionDelegate);
}

void FOnlineEntitlementsAccelByte::SyncPlatformPurchase(const FUniqueNetId& InLocalUserId, const FOnRequestCompleted& CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""))

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	if (AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Oculus"))
		|| AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Meta")))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncMetaQuestIAP>(AccelByteSubsystemPtr.Get(), InLocalUserId, CompletionDelegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sync MetaQuest IAP task dispatched."))
	}
	else
	{
		CompletionDelegate.ExecuteIfBound(false, TEXT("Function not supported for the used platform!"));
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
	}
}

void FOnlineEntitlementsAccelByte::SyncPlatformPurchase(int32 LocalUserNum, const TSharedRef<FPurchaseReceipt>& Receipt, const FOnRequestCompleted& CompletionDelegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	if(AccelByteSubsystemPtr->GetNativePlatformNameString().Contains(TEXT("google")))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncGooglePlay>(AccelByteSubsystemPtr.Get(), LocalUserNum, Receipt, CompletionDelegate);
		return;
	}
	else if (AccelByteSubsystemPtr->GetNativePlatformNameString().Contains(TEXT("apple")))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncIOSAppStore>(AccelByteSubsystemPtr.Get(), LocalUserNum, Receipt, CompletionDelegate);
		return;
	}
	else if (AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Oculus"))
		|| AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Meta")))
	{
		const auto IdentityInterfacePtr = AccelByteSubsystemPtr->GetIdentityInterface();
		if (IdentityInterfacePtr.IsValid())
		{
			const auto UserId = IdentityInterfacePtr->GetUniquePlayerId(LocalUserNum);
			if (UserId != nullptr && UserId.IsValid())
			{
				SyncPlatformPurchase(*UserId.Get(), CompletionDelegate);
				return;
			}
		}
	}

	CompletionDelegate.ExecuteIfBound(false, TEXT("Receipt not supported!"));
}

void FOnlineEntitlementsAccelByte::SyncPlatformPurchase(int32 LocalUserNum, const FAccelByteModelsPlatformSyncMobileGoogle& Request, const FOnRequestCompleted& CompletionDelegate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncGooglePlay>(AccelByteSubsystemPtr.Get(), LocalUserNum, Request, CompletionDelegate);
}

void FOnlineEntitlementsAccelByte::SyncDLC(const FUniqueNetId& InLocalUserId, const FOnRequestCompleted& CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""))

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	if (AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Oculus"))
		|| AccelByteSubsystemPtr->GetSecondaryPlatformSubsystemName().ToString().Contains(TEXT("Meta")))
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncMetaQuestDLC>(AccelByteSubsystemPtr.Get(), InLocalUserId, CompletionDelegate);
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Sync oculus DLC task dispatched."))
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSyncDLC>(AccelByteSubsystemPtr.Get(), InLocalUserId, CompletionDelegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""))
}

void FOnlineEntitlementsAccelByte::GetCurrentUserEntitlementHistory(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId)
{
	GetCurrentUserEntitlementHistory(UserId, false, EntitlementId);
}

void FOnlineEntitlementsAccelByte::GetCurrentUserEntitlementHistory(const FUniqueNetId& UserId, const EAccelByteEntitlementClass& EntitlementClass, FDateTime StartDate, FDateTime EndDate, const FPagedQuery& Page)
{
	GetCurrentUserEntitlementHistory(UserId, true, TEXT(""), EntitlementClass, StartDate, EndDate, Page);
}

void FOnlineEntitlementsAccelByte::GetCurrentUserEntitlementHistory(const FUniqueNetId& UserId, bool bForceUpdate, const FUniqueEntitlementId& EntitlementId, const EAccelByteEntitlementClass& EntitlementClass, FDateTime StartDate, FDateTime EndDate, const FPagedQuery& Page)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory>(AccelByteSubsystemPtr.Get(), UserId, bForceUpdate, EntitlementId, EntitlementClass, StartDate, EndDate, Page);
}

void FOnlineEntitlementsAccelByte::GetCurrentUserEntitlementHistory(int32 LocalUserNum, const FUniqueEntitlementId& EntitlementId)
{
	GetCurrentUserEntitlementHistory(LocalUserNum, false, EntitlementId);
}

void FOnlineEntitlementsAccelByte::GetCurrentUserEntitlementHistory(int32 LocalUserNum, const EAccelByteEntitlementClass& EntitlementClass, FDateTime StartDate, FDateTime EndDate, const FPagedQuery& Page)
{
	GetCurrentUserEntitlementHistory(LocalUserNum, true, TEXT(""), EntitlementClass, StartDate, EndDate, Page);
}

void FOnlineEntitlementsAccelByte::GetCurrentUserEntitlementHistory(int32 LocalUserNum, bool bForceUpdate, const FUniqueEntitlementId& EntitlementId, const EAccelByteEntitlementClass& EntitlementClass, FDateTime StartDate, FDateTime EndDate, const FPagedQuery& Page)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory>(AccelByteSubsystemPtr.Get(), LocalUserNum, bForceUpdate, EntitlementId, EntitlementClass, StartDate, EndDate, Page);
}

void FOnlineEntitlementsAccelByte::GetUserEntitlementHistory(int32 LocalUserNum, const FUniqueEntitlementId& EntitlementId, bool bForceUpdate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserEntitlementHistory>(AccelByteSubsystemPtr.Get(), LocalUserNum, EntitlementId, bForceUpdate);
}

void FOnlineEntitlementsAccelByte::GetUserEntitlementHistory(const FUniqueNetId& UserId, const FUniqueEntitlementId& EntitlementId, bool bForceUpdate)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserEntitlementHistory>(AccelByteSubsystemPtr.Get(), UserId, EntitlementId, bForceUpdate);
}

bool FOnlineEntitlementsAccelByte::GetCachedCurrentUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId, const FUniqueEntitlementId& InEntitlementId, TArray<FAccelByteModelsBaseUserEntitlementHistory>& OutEntitlementHistory)
{
	FScopeLock ScopeLock(&EntitlementMapLock);

	if (!InUserId->IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to get entitlement history from cache as user is invalid"));

		return false;
	}

	FCurrentUserEntitlementHistory& TempUserEntitlementHistoriesMap = *CurrentUserEntitlementHistoryMap.Find(InUserId);

	if (TempUserEntitlementHistoriesMap.Num() == 0)
	{
		UE_LOG_AB(Warning, TEXT("Failed to get entitlement history from cache as there are no entitlement histories stored for this user"));

		return false;
	}

	TArray<FAccelByteModelsBaseUserEntitlementHistory>& UserEntitlementHistories = *TempUserEntitlementHistoriesMap.Find(InEntitlementId);

	if (UserEntitlementHistories.Num() == 0)
	{
		UE_LOG_AB(Warning, TEXT("Failed to get entitlement history from cache as there are no history for this entitlement"));

		return false;
	}

	OutEntitlementHistory = UserEntitlementHistories;

	return true;
}

bool FOnlineEntitlementsAccelByte::DeleteCachedCurrentUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId, const FUniqueEntitlementId& InEntitlementId)
{
	FScopeLock ScopeLock(&EntitlementMapLock);

	if (!InUserId->IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to delete entitlement history from cache as user is invalid"));

		return false;
	}

	FCurrentUserEntitlementHistory& TempUserEntitlementHistoriesMap = *CurrentUserEntitlementHistoryMap.Find(InUserId);

	if (TempUserEntitlementHistoriesMap.Num() == 0)
	{
		UE_LOG_AB(Warning, TEXT("Failed to delete entitlement history from cache as there are no entitlement histories stored for this user"));

		return false;
	}

	TempUserEntitlementHistoriesMap.Remove(InEntitlementId);

	return true;
}

bool FOnlineEntitlementsAccelByte::GetCachedUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId, const FUniqueEntitlementId& InEntitlementId, TArray<FAccelByteModelsUserEntitlementHistory>& OutEntitlementHistory)
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

bool FOnlineEntitlementsAccelByte::DeleteCachedUserEntitlementHistory(const FUniqueNetIdAccelByteUserRef& InUserId, const FUniqueEntitlementId& InEntitlementId)
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

TMap<FString, TArray<FAccelByteModelsBaseUserEntitlementHistory>> FOnlineEntitlementsAccelByte::ExtractUserEntitlementId(TArray<FAccelByteModelsBaseUserEntitlementHistory> InCurrentUserEntitlementHistory)
{
	TArray<FAccelByteModelsBaseUserEntitlementHistory> TempArrayEntitlementHistories{};
	TMap<FString, TArray<FAccelByteModelsBaseUserEntitlementHistory>> MapEntitlementHistoryResult{};

	// Extract each available entitlement id
	for (const auto& CurrentHistory : InCurrentUserEntitlementHistory)
	{
		TempArrayEntitlementHistories = {};

		if (MapEntitlementHistoryResult.Contains(CurrentHistory.EntitlementId))
		{
			MapEntitlementHistoryResult[CurrentHistory.EntitlementId].Add(CurrentHistory);
		}
		else
		{
			TempArrayEntitlementHistories.Add(CurrentHistory);

			MapEntitlementHistoryResult.Add(CurrentHistory.EntitlementId, TempArrayEntitlementHistories);
		}
	};

	return MapEntitlementHistoryResult;
}

void FOnlineEntitlementsAccelByte::QueryPlatformSubscription(
	const int32& InLocalUserNum,
	const FOnlineQuerySubscriptionRequestAccelByte& QueryRequest)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryPlatformSubscription>(
		AccelByteSubsystemPtr.Get(),
		InLocalUserNum,
		QueryRequest);
}
