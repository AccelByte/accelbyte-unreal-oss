// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#include "OnlineSubsystemAccelByteTypes.h"

class FOnlineSubsystemAccelByte;

/**
 * @brief Plain data structure representing an AccelByte user that is cached locally.
 * 
 * Will contain their composite ID, as well as basic data about that user.
 */
struct FAccelByteUserInfo
{
public:

	/**
	 * @brief Composite ID representation for this user, platform information may be blank if we cannot retrieve these values.
	 */
	TSharedPtr<const FUniqueNetIdAccelByteUser> Id;

	/**
	 * @brief Display name for the user on our platform
	 */
	FString DisplayName;

	/**
	 * @brief URL for the user's Avatar icon on AccelByte platform, or blank if none found
	 */
	FString AvatarUrl;

	/**
	 * @brief URL for the user's small Avatar icon on AccelByte platform, or blank if none found
	 */
	FString AvatarSmallUrl;

	/**
	 * @brief URL for the user's large Avatar icon on AccelByte platform, or blank if none found
	 */
	FString AvatarLargeUrl;

	/**
	 * @brief Custom attributes of the user's profile
	 */
	FJsonObject CustomAttributes;

private:

	/**
	 * Flag determining whether or not this user will always be relevant to the player, such as if they are the user's friend.
	 * If this is true, then this user will never be removed from the cache. This should only be set at query time.
	 */
	bool bIsImportant = false;

	/**
	 * Timestamp denoting the last time that this particular user has been grabbed from the cache. If this exceeds the
	 * maximum value set in the user cache, and if the user is not marked as important, they will be purged from the cache.
	 */
	double LastAccessedTimeInSeconds = 0.0;

	/**
	 * Setting the query async task as a friend class to set importance and last accessed
	 */
	friend class FOnlineAsyncTaskAccelByteQueryUsersByIds;

	/**
	 * Setting the user cache as a friend class to set importance and last accessed
	 */
	friend class FOnlineUserCacheAccelByte;

};

/**
 * Delegate for when querying a user through the user cache finishes.
 * 
 * @param bIsSuccessful Whether or not the query overall was a success
 * @param UserIds IDs of the users that we were successfully able to query, and thus are in the cache
 */
DECLARE_DELEGATE_TwoParams(FOnQueryUsersComplete, bool /*bIsSuccessful*/, TArray<TSharedRef<FAccelByteUserInfo>> /*UsersQueried*/);

/**
 * Manages users that are queried from the AccelByte backend, making bulk calls to retrieve user data, as well as getting
 * extra necessary information for those users, such as platform IDs relevant to the current native platform.
 * 
 * As an explanation of how this works under the hood, the cache is made of shared FAccelByteUserInfo instances that are
 * put into two maps. One map is for mapping the AccelByte ID to the user's information, and one is for mapping platform
 * information to the user's information. This way either can be queried seamlessly to get the same user data. The only
 * time you won't be able to query a user by their platform IDs is if they are not on the same platform as you, in which
 * you can only query by AccelByte ID.
 * 
 * User data will be kept cached based on how long it has been since they have been accessed. You can configure how long
 * users will stay in cache with the `UserCachePurgeTimeoutSeconds` variable in the `OnlineSubsystemAccelByte` settings
 * in `DefaultEngine.ini`. Users will also not be purged if they were marked as important when queried.
 */
class FOnlineUserCacheAccelByte
{
public:

	/**
	 * Queries all of the IDs listed in the array on the AccelByte backend for user information, including platform IDs.
	 * If platform IDs are found and match the current platform that we are on, extra queries will be made through the platform OSSes.
	 * 
	 * Caches any results that we get from the backend, as well as will not query from the backend again if a duplicate is found.
	 * 
	 * @param LocalUserNum Index of the user that is attempting to query for other users
	 * @param AccelByteIds Array of strings that represent an ID for a single user
	 * @param Delegate Delegate fired when the query is complete
	 * @param bIsImportant Whether or not we want to mark these users as important so that they stay in the cache, defaults to false.
	 * This should only be used for users that we want to persist for the length of the game session, such as friends.
	 */
	bool QueryUsersByAccelByteIds(int32 LocalUserNum, const TArray<FString>& AccelByteIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant=false);

	/**
	 * Tries to query all platform IDs listed for the particular platform specified on the AccelByte backend to find
	 * AccelByte user matches. If a matches are found, a subsequent query and cache will be performed to get information
	 * from the AccelByte backend on those users.
	 * 
	 * @param LocalUserNum Index of the user that is attempting to query for other users
	 * @param PlatformType String representing the type of platform that these IDs belong to
	 * @param PlatformIds Array of strings that represent an ID for a single user
	 * @param Delegate Delegate fired when the query is complete
	 * @param bIsImportant Whether or not we want to mark these users as important so that they stay in the cache, defaults to false.
	 * This should only be used for users that we want to persist for the length of the game session, such as friends.
	 */
	bool QueryUsersByPlatformIds(int32 LocalUserNum, const FString& PlatformType, const TArray<FString>& PlatformIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant = false);

	/**
	 * Queries all of the IDs listed in the array on the AccelByte backend for user information, including platform IDs.
	 * If platform IDs are found and match the current platform that we are on, extra queries will be made through the platform OSSes.
	 *
	 * Caches any results that we get from the backend, as well as will not query from the backend again if a duplicate is found.
	 *
	 * @param UserId FUniqueNetId of the user that is attempting to query for users
	 * @param AccelByteIds Array of strings that represent an ID for a single user
	 * @param Delegate Delegate fired when the query is complete
	 * @param bIsImportant Whether or not we want to mark these users as important so that they stay in the cache, defaults to false.
	 * This should only be used for users that we want to persist for the length of the game session, such as friends.
	 */
	bool QueryUsersByAccelByteIds(const FUniqueNetId& UserId, const TArray<FString>& AccelByteIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant = false);

	/**
	 * Tries to query all platform IDs listed for the particular platform specified on the AccelByte backend to find
	 * AccelByte user matches. If a matches are found, a subsequent query and cache will be performed to get information
	 * from the AccelByte backend on those users.
	 *
	 * @param UserId FUniqueNetId of the user that is attempting to query for users
	 * @param PlatformType String representing the type of platform that these IDs belong to
	 * @param PlatformIds Array of strings that represent an ID for a single user
	 * @param Delegate Delegate fired when the query is complete
	 * @param bIsImportant Whether or not we want to mark these users as important so that they stay in the cache, defaults to false.
	 * This should only be used for users that we want to persist for the length of the game session, such as friends.
	 */
	bool QueryUsersByPlatformIds(const FUniqueNetId& UserId, const FString& PlatformType, const TArray<FString>& PlatformIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant = false);

	/**
	 * Attempt to get a user from the cache by an AccelByte unique ID. This ID comes from either an FAccelByteUserInfo::Id
	 * field, or from the result of a query users call.
	 */
	TSharedPtr<const FAccelByteUserInfo> GetUser(const FUniqueNetId& UserId);

	/**
	 * Attempt to get a user from the cache by a composite ID structure.
	 */
	TSharedPtr<const FAccelByteUserInfo> GetUser(const FAccelByteUniqueIdComposite& UserId);

PACKAGE_SCOPE:

	/**
	 * Constructs a user cache instance internally, should only be one of these in existence. Will be owned by the
	 * subsystem instance that created it.
	 */
	FOnlineUserCacheAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/**
	 * Add an array of freshly queried users to the user cache
	 */
	void AddUsersToCache(const TArray<TSharedRef<FAccelByteUserInfo>>& UsersQueried);

	/**
	 * Searches through the user caches for a user that hasn't been accessed in longer than the maximum time set for this
	 * cache. If a user is found that exceeds this max time, and they are not marked as important, they will be removed
	 * from the cache entirely.
	 * 
	 * Will return the number of users purged from the cache.
	 * 
	 * Do not call this method directly, it will be called from the owning OnlineSubsystem's ticker!
	 */
	int32 Purge();

	/**
	 * Check whether a user exists already in the cache so that we don't requery them.
	 */
	bool IsUserCached(const FAccelByteUniqueIdComposite& Id);

	/**
	 * Method used by user queries to get an array of users that we still need to query, as well as shared instances to
	 * users that we have already queried and can retrieve from the cache
	 */
	void GetQueryAndCacheArrays(const TArray<FString>& AccelByteIds, TArray<FString>& UsersToQuery, TArray<TSharedRef<FAccelByteUserInfo>>& UsersInCache);

private:

	/**
	 * Mutex used to lock maps while we add to or retrieve from cache
	 */
	FCriticalSection CacheLock;

	/**
	 * Length of time in seconds that a user will stay in the cache without being accessed before being purged.
	 * Defaults to 600 seconds, or 10 minutes.
	 */
	double UserCachePurgeTimeoutSeconds = 600.0;

	/**
	 * User cache that maps AccelByte IDs to shared user instances
	 */
	TMap<FString, TSharedRef<FAccelByteUserInfo>> AccelByteIdToUserInfoMap;

	/**
	 * User cache that maps platform type and ID to shared user instances. The key is just
	 * a string that combines both type and ID, in the following format: "TYPE;ID".
	 */
	TMap<FString, TSharedRef<FAccelByteUserInfo>> PlatformIdToUserInfoMap;

	/**
	 * AccelByte online subsystem instance that owns this user cache.
	 */
	FOnlineSubsystemAccelByte* Subsystem;

	/**
	 * Default constructor deleted, as we only want to be able to have an instance owned by a subsystem
	 */
	FOnlineUserCacheAccelByte() = delete;

	/**
	 * Internal convenience method to convert platform strings to a key for the PlatformIdToUserInfoMap.
	 */
	FString ConvertPlatformTypeAndIdToCacheKey(const FString& Type, const FString& Id) const;

};