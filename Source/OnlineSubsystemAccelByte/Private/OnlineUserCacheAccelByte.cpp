// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteQueryUsersByIds.h"
#include "Containers/UnrealString.h"
#include "OnlineSubsystemAccelByteTypes.h"

bool IsInvalidAccelByteId(const FString& Id)
{
	return !IsAccelByteIDValid(Id);
}

FOnlineUserCacheAccelByte::FOnlineUserCacheAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: Subsystem(InSubsystem)
{
}

int32 FOnlineUserCacheAccelByte::Purge()
{
	// Lock while we attempt to purge from the caches
	FScopeLock ScopeLock(&CacheLock);

	// Filter all of the users in the map that have gone past their elapsed time and aren't marked as important so we can
	// purge them from the user cache maps
	const double CurrentTimeInSeconds = FPlatformTime::Seconds();

	TMap<FString, TSharedRef<FAccelByteUserInfo>> FilteredUserMap;
	FilteredUserMap.Reserve(AccelByteIdToUserInfoMap.Num());
	for (const TPair<FString, TSharedRef<FAccelByteUserInfo>>& UserIdToUserInfo : AccelByteIdToUserInfoMap)
	{
		const double ElapsedTimeInSeconds = CurrentTimeInSeconds - UserIdToUserInfo.Value->LastAccessedTimeInSeconds;
		if (ElapsedTimeInSeconds >= UserCachePurgeTimeoutSeconds && !UserIdToUserInfo.Value->bIsImportant)
		{
			FilteredUserMap.Add(UserIdToUserInfo);
		}
	}
	
	int32 ItemsPurged = 0;

	// Now iterate through the users that we got back from the filter and remove them from the real maps
	for (const TPair<FString, TSharedRef<FAccelByteUserInfo>>& UserIdToUserInfo : FilteredUserMap)
	{
		// Remove the user from the AccelByte ID cache map
		TSharedPtr<const FAccelByteUserInfo> UserInfo = UserIdToUserInfo.Value;
		AccelByteIdToUserInfoMap.Remove(UserIdToUserInfo.Key);

		// Check if the user has a platform ID, if so format it and remove them from the platform cache
		if (UserInfo->Id.IsValid() && UserInfo->Id->HasPlatformInformation())
		{
			const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(UserInfo->Id->GetPlatformType(), UserInfo->Id->GetPlatformId());
			PlatformIdToUserInfoMap.Remove(PlatformId);
		}

		ItemsPurged++;
	}

	return ItemsPurged;
}

bool FOnlineUserCacheAccelByte::IsUserCached(const FAccelByteUniqueIdComposite& Id)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// Start by checking the cache for the user associated with the AccelByte ID, if we have one to query
	if (!Id.Id.IsEmpty())
	{
		return AccelByteIdToUserInfoMap.Contains(Id.Id);
	}

	// Next, if we didn't already find the user using the AccelByte ID, and we have platform type and ID try and query by that
	if (!Id.PlatformType.IsEmpty() && !Id.PlatformId.IsEmpty())
	{
		const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(Id.PlatformType, Id.PlatformId);
		return PlatformIdToUserInfoMap.Contains(PlatformId);
	}

	return false;
}

void FOnlineUserCacheAccelByte::GetQueryAndCacheArrays(const TArray<FString>& AccelByteIds, TArray<FString>& UsersToQuery, TArray<TSharedRef<FAccelByteUserInfo>>& UsersInCache)
{
	for (const FString& AccelByteId : AccelByteIds)
	{
		const TSharedRef<FAccelByteUserInfo>* FoundCachedUser = AccelByteIdToUserInfoMap.Find(AccelByteId);
		if (FoundCachedUser != nullptr)
		{
			UsersInCache.Add(*FoundCachedUser);
		}
		else
		{
			UsersToQuery.Add(AccelByteId);
		}
	}
}

bool FOnlineUserCacheAccelByte::QueryUsersByAccelByteIds(int32 LocalUserNum, const TArray<FString>& AccelByteIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant/*=false*/)
{
	// Remove all IDs that are not valid AccelByte IDs
	TArray<FString> FilteredIds = AccelByteIds;
	FilteredIds.RemoveAll(IsInvalidAccelByteId);

	if (FilteredIds.Num() <= 0)
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByAccelByteIds called with an empty array of IDs, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<TSharedRef<FAccelByteUserInfo>>());
		return false;
	}

	Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUsersByIds>(Subsystem, LocalUserNum, FilteredIds, bIsImportant, Delegate);
	return true;
}

bool FOnlineUserCacheAccelByte::QueryUsersByPlatformIds(int32 LocalUserNum, const FString& PlatformType, const TArray<FString>& PlatformIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant /*= false*/)
{
	if (PlatformType.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with a blank platform type, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<TSharedRef<FAccelByteUserInfo>>());
		return false;
	}

	if (PlatformIds.Num() <= 0)
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with an empty array of IDs, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<TSharedRef<FAccelByteUserInfo>>());
		return false;
	}

	Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUsersByIds>(Subsystem, LocalUserNum, PlatformType, PlatformIds, bIsImportant, Delegate);
	return true;
}

bool FOnlineUserCacheAccelByte::QueryUsersByAccelByteIds(const FUniqueNetId& UserId, const TArray<FString>& AccelByteIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant /*= false*/)
{
	// Remove all IDs that are not valid AccelByte IDs
	TArray<FString> FilteredIds = AccelByteIds;
	FilteredIds.RemoveAll(IsInvalidAccelByteId);

	if (FilteredIds.Num() <= 0)
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByAccelByteIds called with an empty array of IDs, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<TSharedRef<FAccelByteUserInfo>>());
		return false;
	}

	Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUsersByIds>(Subsystem, UserId, FilteredIds, bIsImportant, Delegate);
	return true;
}

bool FOnlineUserCacheAccelByte::QueryUsersByPlatformIds(const FUniqueNetId& UserId, const FString& PlatformType, const TArray<FString>& PlatformIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant /*= false*/)
{
	if (PlatformType.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with a blank platform type, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<TSharedRef<FAccelByteUserInfo>>());
		return false;
	}

	if (PlatformIds.Num() <= 0)
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with an empty array of IDs, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<TSharedRef<FAccelByteUserInfo>>());
		return false;
	}

	Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUsersByIds>(Subsystem, UserId, PlatformType, PlatformIds, bIsImportant, Delegate);
	return true;
}

TSharedPtr<const FAccelByteUserInfo> FOnlineUserCacheAccelByte::GetUser(const FUniqueNetId& UserId)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// If this unique ID is an AccelByte composite ID already, then forward to the GetUser using the composite structure
	if (UserId.GetType() == ACCELBYTE_SUBSYSTEM)
	{
		TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
		return GetUser(AccelByteId->GetCompositeStructure());
	}

	// Otherwise, query as if it is a platform ID
	const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(UserId.GetType().ToString(), UserId.ToString());
	const TSharedRef<FAccelByteUserInfo>* FoundUserInfo = PlatformIdToUserInfoMap.Find(PlatformId);
	if (FoundUserInfo != nullptr)
	{
		(*FoundUserInfo)->LastAccessedTimeInSeconds = FPlatformTime::Seconds();
		return *FoundUserInfo;
	}

	return nullptr;
}

TSharedPtr<const FAccelByteUserInfo> FOnlineUserCacheAccelByte::GetUser(const FAccelByteUniqueIdComposite& UserId)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// Start by checking the cache for the user associated with the AccelByte ID, if we have one to query
	TSharedRef<FAccelByteUserInfo>* FoundUserInfo = nullptr;
	if (!UserId.Id.IsEmpty())
	{
		FoundUserInfo = AccelByteIdToUserInfoMap.Find(UserId.Id);
	}

	// Next, if we didn't already find the user using the AccelByte ID, and we have platform type and ID try and query by that
	if (FoundUserInfo == nullptr && (!UserId.PlatformType.IsEmpty() && !UserId.PlatformId.IsEmpty()))
	{
		const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(UserId.PlatformType, UserId.PlatformId);
		FoundUserInfo = PlatformIdToUserInfoMap.Find(PlatformId);
	}

	if (FoundUserInfo != nullptr)
	{
		(*FoundUserInfo)->LastAccessedTimeInSeconds = FPlatformTime::Seconds();
		return *FoundUserInfo;
	}

	return nullptr;
}

void FOnlineUserCacheAccelByte::AddUsersToCache(const TArray<TSharedRef<FAccelByteUserInfo>>& UsersQueried)
{
	for (const TSharedRef<FAccelByteUserInfo>& User : UsersQueried)
	{
		// Add the user to the AccelByte ID mapping cache first
		AccelByteIdToUserInfoMap.Add(User->Id->GetAccelByteId(), User);

		// Try and add the user to the platform mapping cache if they have platform information
		if (User->Id->HasPlatformInformation())
		{
			const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(User->Id->GetPlatformType(), User->Id->GetPlatformId());
			PlatformIdToUserInfoMap.Add(PlatformId, User);
		}
	}
}

FString FOnlineUserCacheAccelByte::ConvertPlatformTypeAndIdToCacheKey(const FString& Type, const FString& Id) const
{
	const FString PlatformId = FString::Printf(TEXT("%s;%s"), *Type, *Id);
	return PlatformId;
}
