// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineError.h"

/**
 * Delegate that denotes when a user report has completed.
 *
 * @param bWasSuccessful true if the report was sent successfully, false otherwise.
 */
DECLARE_DELEGATE_OneParam(FOnReportUserComplete, bool /*bWasSuccessful*/);

/**
 * Delegate that denotes when create user profile has completed.
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if user profile created and/or acquired successfully, false otherwise.
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnCreateUserProfileComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/);
typedef FOnCreateUserProfileComplete::FDelegate FOnCreateUserProfileCompleteDelegate;

/**
 * Delegate used when the userProfile query request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UserIds list of user ids that were queried
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnQueryUserProfileComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FUniqueNetIdRef>& /*UserIds*/, const FOnlineError& /*Error*/);
typedef FOnQueryUserProfileComplete::FDelegate FOnQueryUserProfileCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnListUserByUserIdComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FListUserDataResponse& /*Data*/, const FOnlineError  & /* OnlineError  */);
typedef FOnListUserByUserIdComplete::FDelegate FOnListUserByUserIdCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLinkOtherPlatformComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
typedef FOnLinkOtherPlatformComplete::FDelegate FOnLinkOtherPlatformCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnlinkOtherPlatformComplete, bool /*bWasSuccessful*/,  const FOnlineError & /*OnlineError*/);
typedef FOnUnlinkOtherPlatformComplete::FDelegate FOnUnlinkOtherPlatformCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
typedef FOnLinkOtherPlatformIdComplete::FDelegate FOnLinkOtherPlatformIdCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnlinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
typedef FOnUnlinkOtherPlatformIdComplete::FDelegate FOnUnlinkOtherPlatformIdCompleteDelegate; 

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnCheckUserAccountAvailabilityComplete, bool /*bWasSuccessful*/, bool /*bUserExisted*/, const FOnlineError & /*OnlineError*/);
typedef FOnCheckUserAccountAvailabilityComplete::FDelegate FOnCheckUserAccountAvailabilityCompleteDelegate;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineUserAccelByte : public IOnlineUser, public TSharedFromThis<FOnlineUserAccelByte, ESPMode::ThreadSafe>
{
public:
	FOnlineUserAccelByte(FOnlineSubsystemAccelByte* InSubsystem);
	virtual ~FOnlineUserAccelByte() override = default;

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Starts an async task that create the profiles for the requesting user. Will trigger OnCreateUserProfileComplete Online Delegate when Complete
	 *
	 * @param UserId the user id requesting the create
	 */
	virtual bool CreateUserProfile(const FUniqueNetId& UserId);

	/**
	 * Delegate used when the userProfile create request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnCreateUserProfileComplete, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the userProfile query request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UserIds list of user ids that were queried
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnQueryUserProfileComplete, bool /*bWasSuccessful*/, const TArray< FUniqueNetIdRef >& /*UserIds*/, const FOnlineError& /*Error*/);

	/**
	 * Starts an async task that queries/reads the profiles for a list of users
	 *
	 * @param LocalUserNum the user requesting the query
	 * @param UserIds list of users to read info about
	 */
	virtual bool QueryUserProfile(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds);

	//~ Begin IOnlineUser overrides
	virtual bool QueryUserInfo(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds) override;
	virtual bool GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<class FOnlineUser>>& OutUsers) override;
	virtual TSharedPtr<FOnlineUser> GetUserInfo(int32 LocalUserNum, const class FUniqueNetId& UserId) override;
	virtual bool QueryUserIdMapping(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FOnQueryUserMappingComplete& Delegate = FOnQueryUserMappingComplete()) override;
	virtual bool QueryExternalIdMappings(const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const FOnQueryExternalIdMappingsComplete& Delegate = FOnQueryExternalIdMappingsComplete()) override;
	virtual void GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<TSharedPtr<const FUniqueNetId>>& OutIds) override;
	virtual TSharedPtr<const FUniqueNetId> GetExternalIdMapping(const FExternalIdQueryOptions& QueryOptions, const FString& ExternalId) override;
	//~ End IOnlineUser overrides

	/**
	 * Get bulk list user by UserId. Only for request by Game Server
	 * 
	 * @param LocalUserNum Index of user(server) that is attempting to create the stats
	 * @param UserIds User to create stats for 
	 */
	virtual void ListUserByUserId(const int32 LocalUserNum, const TArray<FString>& UserIds);
	
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnListUserByUserIdComplete, bool /*bWasSuccessful*/, const FListUserDataResponse& /* ListUser*/, const FOnlineError & /* OnlineError */);

	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnLinkOtherPlatformComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	/*
	 * Links user's current account to their other account in other platform.
	 * Ticket from platform (Platform Token/Authorization Code), can be obtained from Platform OSS or Plugin. 
	 * The browser will redirect the URL to a site with a code in form of parameter URL.
	 */
	void LinkOtherPlatform(const FUniqueNetId& UserId, EAccelBytePlatformType PlatformType, const FString& Ticket);
		
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnUnlinkOtherPlatformComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	/*
	 * Unlinks user's current account from their other account in other platform
	 */
	void UnlinkOtherPlatform(const FUniqueNetId& UserId, EAccelBytePlatformType PlatformType);

	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnLinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	/*
	 * Links user's current account to their other account in other platform, especially to support OIDC.
	 * Ticket from platform (Platform Token/Authorization Code), can be obtained from Platform OSS or Plugin.
	 * The browser will redirect the URL to a site with a code in form of parameter URL.
	 */
	void LinkOtherPlatformId(const FUniqueNetId& UserId, const FString& PlatformId, const FString& Ticket);

	/*
	 * Unlinks user's current account from their other account in other platform, especially to support OIDC
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnUnlinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	void UnlinkOtherPlatformId(const FUniqueNetId& UserId, const FString& PlatformId);

	/**
	 * Delegate called when a controller-user check user account availability.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnCheckUserAccountAvailabilityComplete, bool /*bWasSuccessful*/, bool /*bUserExisted*/, const FOnlineErrorAccelByte & /*OnlineError*/);
	/**
	 *  Check users's account availability, available only using displayName field.   
	 *
	 * @param UserId The user's user ID
	 * @param DisplayName User's display name value to be checked.
	 */
	void CheckUserAccountAvailability(const FUniqueNetId& UserId, const FString& DisplayName);
	
PACKAGE_SCOPE:

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * Internal method for handling extra exec tests for this interface.
	 */
	bool TestExec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar);
#endif

	/**
	 * Internal method used by FOnlineAsyncTaskAccelByteQueryUserInfo to move user info instances back to this interface.
	 */
	void AddUserInfo(const TSharedRef<const FUniqueNetId>& UserId, const TSharedRef<FUserOnlineAccountAccelByte>& UserInfo)
	{
		IDToUserInfoMap.Add(UserId, UserInfo);
	}

	/**
	 * Internal method used by FOnlineAsyncTaskAccelByteQueryExternalIdMappings to move external ID to AccelByte ID references back to this interface.
	 */
	void AddExternalIdMappings(const TMap<FString, TSharedRef<const FUniqueNetId>>& InMap)
	{
		ExternalIDToAccelByteIDMap.Append(InMap);
	}

	/*
	 * Delegate for after completed get user profile on login
	 */
	void PostLoginBulkGetUserProfileCompleted(int32 LocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FOnlineError& ErrorStr);
	
private:

	/** Pointer to the AccelByte OSS instance that instantiated this online user interface. */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Map of AccelByte IDs to AccelByte accounts */
	TUniqueNetIdMap<TSharedRef<FUserOnlineAccountAccelByte>> IDToUserInfoMap;

	/** Map of external IDs (external platform user IDs or display names) to AccelByte IDs */
	TMap<FString, TSharedRef<const FUniqueNetId>> ExternalIDToAccelByteIDMap;
};
