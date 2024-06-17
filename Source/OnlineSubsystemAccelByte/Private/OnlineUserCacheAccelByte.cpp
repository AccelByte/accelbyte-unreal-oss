// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Containers/UnrealString.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUsersByIds.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserProfile.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"

FAccelByteUserPlatformLinkInformation::FAccelByteUserPlatformLinkInformation
	(const FString& InUserId /*= TEXT("")*/)
{
	UserIdRef = FUniqueNetIdAccelByteUser::Create(InUserId);
}

FAccelByteUserPlatformLinkInformation::FAccelByteUserPlatformLinkInformation
	(const TSharedRef<const FUniqueNetId>& InUserId)
	: UserIdRef(FUniqueNetIdAccelByteUser::CastChecked(InUserId)) 
{
}

FAccelByteUserPlatformLinkInformation::FAccelByteUserPlatformLinkInformation
(const TSharedRef<const FUniqueNetId>& InUserId
	, const FString& InDisplayName)
	: UserIdRef(FUniqueNetIdAccelByteUser::CastChecked(InUserId))
	, DisplayName(InDisplayName) 
{
}

FAccelByteUserPlatformLinkInformation::FAccelByteUserPlatformLinkInformation
	(const FAccelByteUniqueIdComposite& InCompositeId)
	: UserIdRef(FUniqueNetIdAccelByteUser::Create(InCompositeId))
{
}

FAccelByteUserPlatformLinkInformation::FAccelByteUserPlatformLinkInformation
(const FPlatformLink& InPlatfromLinked)
	: DisplayName(InPlatfromLinked.DisplayName)
	, EmailAddress(InPlatfromLinked.EmailAddress)
	, LinkedAt(InPlatfromLinked.LinkedAt)
	, Namespace(InPlatfromLinked.Namespace)
	, PlatformId(InPlatfromLinked.PlatformId)
	, PlatformUserId(InPlatfromLinked.PlatformUserId)
	, UserId(InPlatfromLinked.UserId)
	, AccountGroup(InPlatfromLinked.AccountGroup)
{
}

FString FAccelByteUserPlatformLinkInformation::GetRealName() const
{
	return DisplayName;
}

FString FAccelByteUserPlatformLinkInformation::GetDisplayName(const FString& Platform) const
{
	return DisplayName;
}

bool FAccelByteUserPlatformLinkInformation::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	UE_LOG_AB(Warning, TEXT("Cannot configure user attribute as the field is not supported!"));

	return false;
}

bool FAccelByteUserPlatformLinkInformation::SetUserLocalAttribute(const FString& AttrName, const FString& InAttrValue)
{
	UE_LOG_AB(Warning, TEXT("Cannot configure user attribute as the field is not supported!"));

	return false;
}

FString FAccelByteUserPlatformLinkInformation::GetAccessToken() const
{
	UE_LOG_AB(Warning, TEXT("Cannot configure access token as the field is not supported!"));

	return TEXT("");
}

bool FAccelByteUserPlatformLinkInformation::SetUserAttribute(const FString& AttrName, const FString& AttrValue)
{
	UE_LOG_AB(Warning, TEXT("Cannot configure access token as the field is not supported!"));

	return false;
}

bool FAccelByteUserPlatformLinkInformation::GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	UE_LOG_AB(Warning, TEXT("Cannot configure access token as the field is not supported!"));

	return false;
}

void FAccelByteUserPlatformLinkInformation::SetDisplayName(const FString& InDisplayName)
{
	DisplayName = InDisplayName;
}

FString FAccelByteUserPlatformLinkInformation::GetEmailAddress()
{
	return EmailAddress;
}

void FAccelByteUserPlatformLinkInformation::SetEmailAddress(const FString& InEmailAddress)
{
	EmailAddress = InEmailAddress;
}

FString FAccelByteUserPlatformLinkInformation::GetLinkedAt()
{
	return LinkedAt;
}

void FAccelByteUserPlatformLinkInformation::SetLinkedAt(const FString& InTimeLinkedAt)
{
	LinkedAt = InTimeLinkedAt;
}

FString FAccelByteUserPlatformLinkInformation::GetNamespace()
{
	return Namespace;
}

void FAccelByteUserPlatformLinkInformation::SetNamespace(const FString& InNamespace)
{
	Namespace = InNamespace;
}

FString FAccelByteUserPlatformLinkInformation::GetPlatformId()
{
	return PlatformId;
}

void FAccelByteUserPlatformLinkInformation::SetPlatformId(const FString& InPlatformId)
{
	PlatformId = InPlatformId;
}

FString FAccelByteUserPlatformLinkInformation::GetPlatformUserId()
{
	return PlatformUserId;
}

void FAccelByteUserPlatformLinkInformation::SetPlatformUserId(const FString& InPlatformUserId)
{
	PlatformUserId = InPlatformUserId;
}

FString FAccelByteUserPlatformLinkInformation::GetAccelByteUserId()
{
	return UserId;
}

void FAccelByteUserPlatformLinkInformation::SetAccelByteUserId(const FString& InAccelByteUserId)
{
	UserId = InAccelByteUserId;
}

void FAccelByteUserPlatformLinkInformation::SetAccountGroup(const FString& InAccountGroup)
{
	AccountGroup = InAccountGroup;
}

FString FAccelByteUserPlatformLinkInformation::GetAccountGroup()
{
	return AccountGroup;
}

bool IsInvalidAccelByteId(const FString& Id)
{
	return !IsAccelByteIDValid(Id);
}

FOnlineUserCacheAccelByte::FOnlineUserCacheAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: Subsystem(InSubsystem)
{
}

void FOnlineUserCacheAccelByte::Init()
{
	if (!FAccelByteUtilities::LoadABConfigFallback(TEXT("OnlineSubsystemAccelByte")
		, TEXT("bEnableStalenessChecking")
		, bEnableStalenessChecking))
	{
		UE_LOG_AB(Verbose, TEXT("'bEnableStalenessChecking' is not specified in DefaultEngine.ini, or on command line. Defaulting to '%s'."), LOG_BOOL_FORMAT(bEnableStalenessChecking));
	}

	// Using int here as 'LoadABConfigFallback' does not have an override for double values
	int32 TimeUntilStaleSecondsInt { static_cast<int32>(TimeUntilStaleSeconds) };
	if (!FAccelByteUtilities::LoadABConfigFallback(TEXT("OnlineSubsystemAccelByte")
		, TEXT("TimeUntilStaleSeconds")
		, TimeUntilStaleSecondsInt))
	{
		UE_LOG_AB(Verbose, TEXT("'TimeUntilStaleSeconds' is not specified in DefaultEngine.ini, or on command line. Defaulting to %d seconds."), TimeUntilStaleSecondsInt);
	}
	TimeUntilStaleSeconds = static_cast<double>(TimeUntilStaleSecondsInt);
}

bool FOnlineUserCacheAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineUserCacheAccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetUserCache();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineUserCacheAccelByte::GetFromWorld(const UWorld* World, FOnlineUserCacheAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

int32 FOnlineUserCacheAccelByte::Purge()
{
	// Lock while we attempt to purge from the caches
	FScopeLock ScopeLock(&CacheLock);

	// Filter all of the users in the map that have gone past their elapsed time and aren't marked as important so we can
	// purge them from the user cache maps
	const double CurrentTimeInSeconds = FPlatformTime::Seconds();

	TMap<FString, FAccelByteUserInfoRef> FilteredUserMap;
	FilteredUserMap.Reserve(AccelByteIdToUserInfoMap.Num());
	for (const TPair<FString, FAccelByteUserInfoRef>& UserIdToUserInfo : AccelByteIdToUserInfoMap)
	{
		const double ElapsedTimeInSeconds = CurrentTimeInSeconds - UserIdToUserInfo.Value->LastAccessedTimeInSeconds;
		if (ElapsedTimeInSeconds >= UserCachePurgeTimeoutSeconds && !UserIdToUserInfo.Value->bIsImportant)
		{
			FilteredUserMap.Add(UserIdToUserInfo);
		}
	}
	
	int32 ItemsPurged = 0;

	// Now iterate through the users that we got back from the filter and remove them from the real maps
	for (const TPair<FString, FAccelByteUserInfoRef>& UserIdToUserInfo : FilteredUserMap)
	{
		// Remove the user from the AccelByte ID cache map
		TSharedPtr<const FAccelByteUserInfo, ESPMode::ThreadSafe> UserInfo = UserIdToUserInfo.Value;
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

void FOnlineUserCacheAccelByte::GetQueryAndCacheArrays(const TArray<FString>& AccelByteIds, TArray<FString>& UsersToQuery, TArray<FAccelByteUserInfoRef>& UsersInCache)
{
	for (const FString& AccelByteId : AccelByteIds)
	{
		const FAccelByteUserInfoRef* FoundCachedUser = AccelByteIdToUserInfoMap.Find(AccelByteId);
		if (FoundCachedUser != nullptr)
		{
			// We have found a user in our cache, check if their data is stale
			const bool bIsStale = bEnableStalenessChecking && IsUserDataStale(*(*FoundCachedUser)->Id.Get());
			if (!bIsStale)
			{
				// Data is not stale, return this user as a cached user and continue to next ID to check
				UsersInCache.Add(*FoundCachedUser);
				continue;
			}

			// #NOTE New data requested from backend will end up overwriting the data in the cache. With this in mind,
			// there is no need to set 'bIsForcedStale' to false, as the overwrite will handle it for us.
		}
	
		UsersToQuery.Add(AccelByteId);
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
		Delegate.ExecuteIfBound(true, TArray<FAccelByteUserInfoRef>());
		return false;
	}

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());

	//Run QueryUserProfile after QueryUsersByIds to get Info like FriendId
	Subsystem->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteQueryUsersByIds>(Subsystem, LocalUserNum, FilteredIds, bIsImportant, Delegate);
	Subsystem->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteQueryUserProfile>(Subsystem, LocalUserNum, FilteredIds, UserInterface->OnQueryUserProfileCompleteDelegates[LocalUserNum]);
	return true;
}

bool FOnlineUserCacheAccelByte::QueryUsersByPlatformIds(int32 LocalUserNum, const FString& PlatformType, const TArray<FString>& PlatformIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant /*= false*/)
{
	if (PlatformType.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with a blank platform type, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<FAccelByteUserInfoRef>());
		return false;
	}

	if (PlatformIds.Num() <= 0)
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with an empty array of IDs, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<FAccelByteUserInfoRef>());
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
		Delegate.ExecuteIfBound(true, TArray<FAccelByteUserInfoRef>());
		return false;
	}

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());

	int32 LocalUserNum;
	IdentityInterface->GetLocalUserNum(UserId, LocalUserNum);

	//Run QueryUserProfile after QueryUsersByIds to get Info like FriendId
	Subsystem->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteQueryUsersByIds>(Subsystem, UserId, FilteredIds, bIsImportant, Delegate);
	Subsystem->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteQueryUserProfile>(Subsystem, UserId, FilteredIds, UserInterface->OnQueryUserProfileCompleteDelegates[LocalUserNum]);
	return true;
}

bool FOnlineUserCacheAccelByte::QueryUsersByPlatformIds(const FUniqueNetId& UserId, const FString& PlatformType, const TArray<FString>& PlatformIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant /*= false*/)
{
	if (PlatformType.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with a blank platform type, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<FAccelByteUserInfoRef>());
		return false;
	}

	if (PlatformIds.Num() <= 0)
	{
		UE_LOG_AB(Warning, TEXT("FOnlineUserStoreAccelByte::QueryUsersByPlatformIds called with an empty array of IDs, skipping this call!"));
		Delegate.ExecuteIfBound(true, TArray<FAccelByteUserInfoRef>());
		return false;
	}

	Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUsersByIds>(Subsystem, UserId, PlatformType, PlatformIds, bIsImportant, Delegate);
	return true;
}

TSharedPtr<const FAccelByteUserInfo, ESPMode::ThreadSafe> FOnlineUserCacheAccelByte::GetUser(const FUniqueNetId& UserId)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// If this unique ID is an AccelByte composite ID already, then forward to the GetUser using the composite structure
	if (UserId.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		FUniqueNetIdAccelByteUserRef AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
		return GetUser(AccelByteId->GetCompositeStructure());
	}

	// Otherwise, query as if it is a platform ID
	const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(UserId.GetType().ToString(), UserId.ToString());
	const FAccelByteUserInfoRef* FoundUserInfo = PlatformIdToUserInfoMap.Find(PlatformId);
	if (FoundUserInfo != nullptr)
	{
		(*FoundUserInfo)->LastAccessedTimeInSeconds = FPlatformTime::Seconds();
		return *FoundUserInfo;
	}

	return nullptr;
}

TSharedPtr<const FAccelByteUserInfo, ESPMode::ThreadSafe> FOnlineUserCacheAccelByte::GetUser(const FAccelByteUniqueIdComposite& UserId)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// Start by checking the cache for the user associated with the AccelByte ID, if we have one to query
	FAccelByteUserInfoRef* FoundUserInfo = nullptr;
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

bool FOnlineUserCacheAccelByte::IsStalenessCheckEnabled() const
{
	return bEnableStalenessChecking;
}

double FOnlineUserCacheAccelByte::GetTimeUntilStaleSeconds() const
{
	return TimeUntilStaleSeconds;
}

void FOnlineUserCacheAccelByte::SetStalenessCheckEnabled(bool bInEnableStalenessChecking)
{
	bEnableStalenessChecking = bInEnableStalenessChecking;
}

void FOnlineUserCacheAccelByte::SetTimeUntilStaleSeconds(double InTimeUntilStaleSeconds)
{
	TimeUntilStaleSeconds = InTimeUntilStaleSeconds;
}

bool FOnlineUserCacheAccelByte::SetUserDataAsStale(const FUniqueNetId& UserUniqueId)
{
	FUniqueNetIdAccelByteUserPtr UserUniqueAccelByteId = FUniqueNetIdAccelByteUser::TryCast(UserUniqueId);
	if (!UserUniqueId.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to 'SetUserDataAsStale' in user cache as the passed in unique ID was not a valid AccelByte user ID! Unique ID: %s"), *UserUniqueId.ToDebugString());
		return false;
	}

	return SetUserDataAsStale(UserUniqueAccelByteId->GetAccelByteId());
}

bool FOnlineUserCacheAccelByte::SetUserDataAsStale(const FString& InAccelByteId)
{
	const FAccelByteUserInfoRef* FoundCachedUser = AccelByteIdToUserInfoMap.Find(InAccelByteId);
	if (FoundCachedUser == nullptr)
	{
		return false;
	}

	(*FoundCachedUser)->bIsForcedStale = true;
	return true;
}

bool FOnlineUserCacheAccelByte::IsUserDataStale(const FUniqueNetId& UserUniqueId)
{
	FUniqueNetIdAccelByteUserPtr UserUniqueAccelByteId = FUniqueNetIdAccelByteUser::TryCast(UserUniqueId);
	if (!UserUniqueId.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to check 'bIsUserDataStale' in user cache as the passed in unique ID was not a valid AccelByte user ID! Unique ID: %s"), *UserUniqueId.ToDebugString());
		return false;
	}

	return IsUserDataStale(UserUniqueAccelByteId->GetAccelByteId());
}

bool FOnlineUserCacheAccelByte::IsUserDataStale(const FString& InAccelByteId)
{
	const FAccelByteUserInfoRef* FoundCachedUser = AccelByteIdToUserInfoMap.Find(InAccelByteId);
	if (FoundCachedUser == nullptr)
	{
		return true;
	}

	if ((*FoundCachedUser)->bIsForcedStale)
	{
		return true;
	}

	return ((*FoundCachedUser)->LastUpdatedTime + FTimespan::FromSeconds(GetTimeUntilStaleSeconds())) >= FDateTime::UtcNow();
}

void FOnlineUserCacheAccelByte::AddUsersToCache(const TArray<FAccelByteUserInfoRef>& UsersQueried)
{
	for (const FAccelByteUserInfoRef& User : UsersQueried)
	{
		if (User->PublicCode.IsEmpty())
		{
			const auto CachedUserInfo = GetUser(*User->Id.ToSharedRef());
			if (CachedUserInfo.IsValid())
			{
				if (!CachedUserInfo->PublicCode.IsEmpty())
				{
					User->PublicCode = CachedUserInfo->PublicCode;
				}
			}
		}

		// Add the user to the AccelByte ID mapping cache first
		if (AccelByteIdToUserInfoMap.Find(User->Id->GetAccelByteId()))
		{
			AccelByteIdToUserInfoMap[User->Id->GetAccelByteId()]->CopyValue(User.Get());
		}
		else
		{
			AccelByteIdToUserInfoMap.Emplace(User->Id->GetAccelByteId(), User);
		}

		// Try and add the user to the platform mapping cache if they have platform information
		if (User->Id->HasPlatformInformation())
		{
			const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(User->Id->GetPlatformType(), User->Id->GetPlatformId());
			if (PlatformIdToUserInfoMap.Find(PlatformId))
			{
				PlatformIdToUserInfoMap[PlatformId]->CopyValue(User.Get());
			}
			else
			{
				PlatformIdToUserInfoMap.Emplace(PlatformId, User);
			}
		}
	}
}

void FOnlineUserCacheAccelByte::AddPublicCodeToCache(const FUniqueNetId& UserId, const FString& PublicCode)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// If this unique ID is an AccelByte composite ID already, then forward to the GetUser using the composite structure
	if (UserId.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		FUniqueNetIdAccelByteUserRef AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
		AddPublicCodeToCache(AccelByteId->GetCompositeStructure(), PublicCode);
		return;
	}

	// Otherwise, query as if it is a platform ID
	const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(UserId.GetType().ToString(), UserId.ToString());
	const FAccelByteUserInfoRef* FoundUserInfo = PlatformIdToUserInfoMap.Find(PlatformId);
	if (FoundUserInfo != nullptr)
	{
		(*FoundUserInfo)->LastAccessedTimeInSeconds = FPlatformTime::Seconds();
		FoundUserInfo->Get().PublicCode = PublicCode;
	}
	else
	{
		// Create a new user info
		FAccelByteUniqueIdComposite CompositeId{UserId.ToString(), UserId.GetType().ToString(), PlatformId};
		FAccelByteUserInfoRef User = MakeShared<FAccelByteUserInfo, ESPMode::ThreadSafe>();
		User->Id = FUniqueNetIdAccelByteUser::Create(CompositeId);
		User->PublicCode = PublicCode;
		AddUsersToCache({ User });
	}
}

void FOnlineUserCacheAccelByte::AddPublicCodeToCache(const FAccelByteUniqueIdComposite& UserId, const FString& PublicCode)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// Start by checking the cache for the user associated with the AccelByte ID, if we have one to query
	FAccelByteUserInfoRef* FoundUserInfo = nullptr;
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
		FoundUserInfo->Get().PublicCode = PublicCode;
	}
	else
	{
		// Create a new user info
		FAccelByteUserInfoRef User = MakeShared<FAccelByteUserInfo, ESPMode::ThreadSafe>();
		User->Id = FUniqueNetIdAccelByteUser::Create(UserId);
		User->PublicCode = PublicCode;
		AddUsersToCache({ User });
	}
}

void FOnlineUserCacheAccelByte::AddLinkedPlatformInfoToCache(const FUniqueNetId& UserId, const TArray<FAccelByteLinkedUserInfo>& LinkedPlatformInfo)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// If this unique ID is an AccelByte composite ID already, then forward to the GetUser using the composite structure
	if (UserId.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		FUniqueNetIdAccelByteUserRef AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
		AddLinkedPlatformInfoToCache(AccelByteId->GetCompositeStructure(), LinkedPlatformInfo);
		return;
	}

	// Otherwise, query as if it is a platform ID
	const FString PlatformId = ConvertPlatformTypeAndIdToCacheKey(UserId.GetType().ToString(), UserId.ToString());
	const FAccelByteUserInfoRef* FoundUserInfo = PlatformIdToUserInfoMap.Find(PlatformId);
	if (FoundUserInfo != nullptr)
	{
		(*FoundUserInfo)->LastAccessedTimeInSeconds = FPlatformTime::Seconds();
		FoundUserInfo->Get().LinkedPlatformInfo = LinkedPlatformInfo;
	}
	else
	{
		// Create a new user info
		FAccelByteUniqueIdComposite CompositeId{UserId.ToString(), UserId.GetType().ToString(), PlatformId};
		FAccelByteUserInfoRef User = MakeShared<FAccelByteUserInfo, ESPMode::ThreadSafe>();
		User->Id = FUniqueNetIdAccelByteUser::Create(CompositeId);
		User->LinkedPlatformInfo = LinkedPlatformInfo;
		AddUsersToCache({ User });
	}
}

void FOnlineUserCacheAccelByte::AddLinkedPlatformInfoToCache(const FAccelByteUniqueIdComposite& UserId, const TArray<FAccelByteLinkedUserInfo>& LinkedPlatformInfo)
{
	// Lock while we access the cache
	FScopeLock ScopeLock(&CacheLock);

	// Start by checking the cache for the user associated with the AccelByte ID, if we have one to query
	FAccelByteUserInfoRef* FoundUserInfo = nullptr;
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
		FoundUserInfo->Get().LinkedPlatformInfo = LinkedPlatformInfo;
	}
	else
	{
		// Create a new user info
		FAccelByteUserInfoRef User = MakeShared<FAccelByteUserInfo, ESPMode::ThreadSafe>();
		User->Id = FUniqueNetIdAccelByteUser::Create(UserId);
		User->LinkedPlatformInfo = LinkedPlatformInfo;
		AddUsersToCache({ User });
	}
}

FString FOnlineUserCacheAccelByte::ConvertPlatformTypeAndIdToCacheKey(const FString& Type, const FString& Id) const
{
	const FString PlatformId = FString::Printf(TEXT("%s;%s"), *Type, *Id);
	return PlatformId;
}

void FAccelByteUserInfo::CopyValue(const FAccelByteUserInfo& Data)
{
	Id = Data.Id;
	DisplayName = Data.DisplayName;
	UniqueDisplayName = Data.UniqueDisplayName;
	PublicCode = Data.PublicCode.IsEmpty() ? PublicCode : Data.PublicCode;
	GameAvatarUrl = Data.GameAvatarUrl;
	PublisherAvatarUrl = Data.PublisherAvatarUrl;
	CustomAttributes = Data.CustomAttributes;
	LinkedPlatformInfo = Data.LinkedPlatformInfo;
	bIsImportant = Data.bIsImportant;
	LastAccessedTimeInSeconds = FPlatformTime::Seconds();
	LastUpdatedTime = FDateTime::UtcNow();
	bIsForcedStale = false;
}
