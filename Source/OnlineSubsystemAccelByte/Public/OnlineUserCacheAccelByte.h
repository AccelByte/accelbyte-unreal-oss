// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByteTypes.h"
#include "Dom/JsonObject.h"
#include "InterfaceModels/OnlineUserInterfaceAccelByteModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

class FOnlineSubsystemAccelByte;
class IOnlineSubsystem;

/**
 * @brief 3rd party platform information associated with an user AccelByte account 
 */
class ONLINESUBSYSTEMACCELBYTE_API FAccelByteUserPlatformLinkInformation : public FUserOnlineAccount
{
public:
	explicit FAccelByteUserPlatformLinkInformation(const FString& InUserId = TEXT(""));

	explicit FAccelByteUserPlatformLinkInformation(const TSharedRef<const FUniqueNetId>& InUserId);

	explicit FAccelByteUserPlatformLinkInformation(const TSharedRef<const FUniqueNetId>& InUserId, const FString& InDisplayName);

	explicit FAccelByteUserPlatformLinkInformation(const FAccelByteUniqueIdComposite& InCompositeId);

	explicit FAccelByteUserPlatformLinkInformation(const FPlatformLink& InPlatfromLinked);

	virtual ~FAccelByteUserPlatformLinkInformation() override = default;

	//~ Begin FOnlineUser Interface
	virtual TSharedRef<const FUniqueNetId> GetUserId() const override { return UserIdRef; }
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	virtual bool SetUserLocalAttribute(const FString& AttrName, const FString& InAttrValue) override;
	//~ End FOnlineUser Interface

	//~ Begin FUserOnlineAccount Interface
	virtual FString GetAccessToken() const override;
	virtual bool SetUserAttribute(const FString& AttrName, const FString& AttrValue) override;
	virtual bool GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	//~ End FUserOnlineAccount Interface

	//~ Begin AccelByte-specific implementation
	/**
	 * @brief Set user's 3rd party display name that linked with AccelByte account.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 *
	 * @param InDisplayName The user's display name.
	 */
	void SetDisplayName(const FString& InDisplayName);

	/**
	 * @brief Get user's email address from the 3rd party that linked with AccelByte account.
	 *
	 * @return The user's email address.
	 */
	FString GetEmailAddress();

	/**
	 * @brief Set user's email address from the 3rd party that linked with AccelByte account.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 * 
	 * @param InEmailAddress The user's email address.
	 */
	void SetEmailAddress(const FString& InEmailAddress);

	/**
	 * @brief Get the time when player that linked their 3rd party account with AccelByte account.
	 *
	 * @return The time when player that linked their 3rd party account.
	 */
	FString GetLinkedAt();

	/**
	 * @brief Set the time when player that linked their 3rd party account with AccelByte account.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 *
	 * @param InTimeLinkedAt The time user linked their account.
	 */
	void SetLinkedAt(const FString& InTimeLinkedAt);

	/**
	 * @brief Get the user's namespace that linked with AccelByte account.
	 *
	 * @return The user's namespace.
	 */
	FString GetNamespace();

	/**
	 * @brief Set user's namespace that linked with AccelByte account.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 *
	 * @param InNamespace The user's namespace.
	 */
	void SetNamespace(const FString& InNamespace);

	/**
	 * @brief Get the 3rd party platform ID.
	 *
	 * @return The 3rd party platform ID.
	 */
	FString GetPlatformId();

	/**
	 * @brief Set the 3rd party platform ID.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 *
	 * @param PlatformId The 3rd party platform ID.
	 */
	void SetPlatformId(const FString& InPlatformId);

	/**
	 * @brief Get user's ID from the 3rd party platform.
	 *
	 * @return User ID from 3rd party platform.
	 */
	FString GetPlatformUserId();

	/**
	 * @brief Set user's ID from the 3rd party platform.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 *
	 * @param InPlatformUserId The user's ID from the 3rd party platform.
	 */
	void SetPlatformUserId(const FString& InPlatformUserId);

	/**
	 * @brief Get user's AccelByte account ID.
	 *
	 * @return The user's AccelByte ID.
	 */
	FString GetAccelByteUserId();

	/**
	 * @brief Set user's AccelByte account ID.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 *
	 * @param InAccelByteUserId The user's AccelByte ID.
	 */
	void SetAccelByteUserId(const FString& InAccelByteUserId);

	/**
	 * @brief Set user's 3rd party platform account group.
	 * Note: The changes will not affect the value in AccelByte backend.
	 * Only cached value that will be affected.
	 *
	 * @param InAccountGroup The user's 3rd party account group.
	 */
	void SetAccountGroup(const FString& InAccountGroup);

	/**
	 * @brief Get user's 3rd party account group.
	 *
	 * @return The user's 3rd party account group.
	 */
	FString GetAccountGroup();
	//~ End AccelByte-specific implementation

private:
	/** User Id represented as a FUniqueNetId */
	FUniqueNetIdAccelByteUserRef UserIdRef = FUniqueNetIdAccelByteUser::Invalid();

	/** Display name for the related platform associated with AccelByte account */
	FString DisplayName;

	/** Email address for the related platform associated with AccelByte account */
	FString EmailAddress;

	/** The time that the related platform linked with AccelByte account */
	FString LinkedAt;

	/** Namespace for the related platform associated with AccelByte account */
	FString Namespace;

	/** Platform ID for the related platform associated with AccelByte account */
	FString PlatformId;

	/** Platform User ID for the related platform associated with AccelByte account */
	FString PlatformUserId;

	/** AccelByte account User Id */
	FString UserId;

	/** The group for the related 3rd party account */
	FString AccountGroup;
};

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
	TSharedPtr<const FUniqueNetIdAccelByteUser> Id{ nullptr };

	/**
	 * @brief Display name for the user on our platform
	 */
	FString DisplayName{};

	/**
	 * @brief Platform display name for the user
	 */
	FString UniqueDisplayName{};

	/**
	 * @brief Generated public user identifier code, usually used as a friend code
	 */
	FString PublicCode{};

	/**
	 * @brief URL for an avatar for this user at the game level, may be blank if the user does not have one
	 */
	FString GameAvatarUrl{};

	/**
	 * @brief URL for an avatar for this user at the publisher level, may be blank if the user does not have one
	 */
	FString PublisherAvatarUrl{};

	/**
	 * @brief Custom attributes of the user's profile
	 */
	FJsonObject CustomAttributes{};

	/**
	 * @brief Array of user linked platform information
	 */
	TArray<FAccelByteLinkedUserInfo> LinkedPlatformInfo;

private:

	/**
	 * Flag determining whether or not this user will always be relevant to the player, such as if they are the user's friend.
	 * If this is true, then this user will never be removed from the cache. This should only be set at query time.
	 */
	bool bIsImportant{ false };

	/**
	 * Timestamp denoting the last time that this particular user has been grabbed from the cache. If this exceeds the
	 * maximum value set in the user cache, and if the user is not marked as important, they will be purged from the cache.
	 */
	double LastAccessedTimeInSeconds{ 0.0 };

	/**
	 * UTC date and time of this user's data last updated. Data will be considered 'stale' after exceed the stale time that configured on the User Cache Interface.
	 * Once a user is stale, their cached data is not returned to the client directly, instead a request for new data will be made.
	 */
	FDateTime LastUpdatedTime{ 0 };

	/**
	 * Whether this cached player's data should be considered stale regardless of whether the 'LastUpdatedTime' has been exceed the stale state.
	 * Set through 'FOnlineUserCacheAccelByte::SetUserDataAsStale'.
	 */
	bool bIsForcedStale { false };

	/**
	 * Copy UserInfo value 
	 */
	void CopyValue(const FAccelByteUserInfo& Data);

	/**
	 * Setting the query async task as a friend class to set importance and last accessed
	 */
	friend class FOnlineAsyncTaskAccelByteQueryUsersByIds;

	/**
	 * Setting the user cache as a friend class to set importance and last accessed
	 */
	friend class FOnlineUserCacheAccelByte;

};

typedef TSharedRef<FAccelByteUserInfo, ESPMode::ThreadSafe> FAccelByteUserInfoRef;
typedef TSharedPtr<FAccelByteUserInfo, ESPMode::ThreadSafe> FAccelByteUserInfoPtr;

/**
 * Delegate for when querying a user through the user cache finishes.
 *
 * @param bIsSuccessful Whether or not the query overall was a success
 * @param UserIds IDs of the users that we were successfully able to query, and thus are in the cache
 */
DECLARE_DELEGATE_TwoParams(FOnQueryUsersComplete, bool /*bIsSuccessful*/, TArray<FAccelByteUserInfoRef> /*UsersQueried*/);

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
class ONLINESUBSYSTEMACCELBYTE_API FOnlineUserCacheAccelByte
{
public:
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineUserCacheAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineUserCacheAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

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
	bool QueryUsersByAccelByteIds(int32 LocalUserNum, const TArray<FString>& AccelByteIds, const FOnQueryUsersComplete& Delegate, bool bIsImportant = false);

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
	TSharedPtr<const FAccelByteUserInfo, ESPMode::ThreadSafe> GetUser(const FUniqueNetId& UserId);

	/**
	 * Attempt to get a user from the cache by a composite ID structure.
	 */
	TSharedPtr<const FAccelByteUserInfo, ESPMode::ThreadSafe> GetUser(const FAccelByteUniqueIdComposite& UserId);

	/**
	 * Checks if staleness checks for cached user data is enabled.
	 */
	bool IsStalenessCheckEnabled() const;

	/**
	 * Returns the length of time in seconds that it takes for a user's cached data to become stale.
	 */
	double GetTimeUntilStaleSeconds() const;

	/**
	 * Enable or disable staleness checks in the user cache. Intended for runtime testing of cache.
	 * Prefer using ini or command line to enable or disable staleness checks.
	 */
	void SetStalenessCheckEnabled(bool bInEnableStalenessChecking);

	/**
	 * Set the amount of time in seconds that it takes for a user's cached data to become stale.
	 * Intended for runtime testing of cache. Prefer using ini or command line to control staleness time.
	 */
	void SetTimeUntilStaleSeconds(double InTimeUntilStaleSeconds);

	/**
	 * Attempts to mark the referenced user's cached data as stale. If found, the force stale flag on the user's data
	 * will be updated, and the next request to get user data will request new data for this user, regardless of whether
	 * the stale datetime has been reached. This flag will be reset after the request is processed.
	 *
	 * @param UserUniqueId Unique ID of the user that we wish to set as stale, must be an AccelByte unique ID
	 * @return true if flag was set, false otherwise
	 */
	bool SetUserDataAsStale(const FUniqueNetId& UserUniqueId);

	/**
	 * Attempts to mark the referenced user's cached data as stale. If found, the force stale flag on the user's data
	 * will be updated, and the next request to get user data will request new data for this user, regardless of whether
	 * the stale datetime has been reached. This flag will be reset after the request is processed.
	 *
	 * @param InAccelByteId String representation of the user ID that you wish to mark stale
	 * @return true if flag was set, false otherwise
	 */
	bool SetUserDataAsStale(const FString& InAccelByteId);

	/**
	* Check whether user's cached data has become stale.
	*
	* @param UserUniqueId Unique ID of the user that we wish to check the staleness, must be an AccelByte unique ID
	* @return true if data stale, false otherwise
	*/
	bool IsUserDataStale(const FUniqueNetId& UserUniqueId);

	/**
	* Check whether user's cached data has become stale.
	*
	* @param InAccelByteId String representation of the user ID that you wish to check the staleness
	* @return true if data stale, false otherwise
	*/
	bool IsUserDataStale(const FString& InAccelByteId);

PACKAGE_SCOPE:

	/**
	 * Constructs a user cache instance internally, should only be one of these in existence. Will be owned by the
	 * subsystem instance that created it.
	 */
	FOnlineUserCacheAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/**
	 * Initialize the user cache. This will load configuration values from the engine INI for the cache. Called from
	 * the subsystem initialization method.
	 */
	void Init();

	/**
	 * Add an array of freshly queried users to the user cache
	 */
	void AddUsersToCache(const TArray<FAccelByteUserInfoRef>& UsersQueried);

	/**
	 * Add PublicCode to a user cache, create new if not exist
	 */
	void AddPublicCodeToCache(const FUniqueNetId& UserId, const FString& PublicCode);

	/**
	 * Add PublicCode to a user cache, create new if not exist
	 */
	void AddPublicCodeToCache(const FAccelByteUniqueIdComposite& UserId, const FString& PublicCode);

	/**
	 * Adds linked platform information to the user cache for a given user ID.
	 * Creates a new cache entry if the user does not already exist in the cache.
	 */
	void AddLinkedPlatformInfoToCache(const FUniqueNetId& UserId, const TArray<FAccelByteLinkedUserInfo>& LinkedPlatformInfo);

	/**
	 * Adds linked platform information to the user cache for a given composite user ID.
	 * Creates a new cache entry if the user does not already exist in the cache.
	 */
	void AddLinkedPlatformInfoToCache(const FAccelByteUniqueIdComposite& UserId, const TArray<FAccelByteLinkedUserInfo>& LinkedPlatformInfo);

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
	void GetQueryAndCacheArrays(const TArray<FString>& AccelByteIds, TArray<FString>& UsersToQuery, TArray<FAccelByteUserInfoRef>& UsersInCache);

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
	TMap<FString, FAccelByteUserInfoRef> AccelByteIdToUserInfoMap;

	/**
	 * User cache that maps platform type and ID to shared user instances. The key is just
	 * a string that combines both type and ID, in the following format: "TYPE;ID".
	 */
	TMap<FString, FAccelByteUserInfoRef> PlatformIdToUserInfoMap;

	/**
	 * AccelByte online subsystem instance that owns this user cache.
	 */
	FOnlineSubsystemAccelByte* Subsystem;

	/**
	 * Should the user cache take staleness into account? If false, users in cache will never become stale
	 * and their data will never be queried after the initial request. Staleness checks are enabled by default.
	 */
	bool bEnableStalenessChecking { true };

	/**
	 * Amount of time in seconds that a user's cached data will be considered stale. Once this time is reached, any
	 * future call to query that user's data will result in a new request. By default, a user's cached data is stale
	 * after ten minutes.
	 */
	double TimeUntilStaleSeconds { 600.0 };

	/**
	 * Default constructor deleted, as we only want to be able to have an instance owned by a subsystem
	 */
	FOnlineUserCacheAccelByte() = delete;

	/**
	 * Internal convenience method to convert platform strings to a key for the PlatformIdToUserInfoMap.
	 */
	FString ConvertPlatformTypeAndIdToCacheKey(const FString& Type, const FString& Id) const;

};