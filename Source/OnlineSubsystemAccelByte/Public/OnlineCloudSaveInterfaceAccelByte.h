// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetUserRecordCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsUserRecord&, const FString& /*Error*/);
typedef FOnGetUserRecordCompleted::FDelegate FOnGetUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnReplaceUserRecordCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FString& /*Error*/);
typedef FOnReplaceUserRecordCompleted::FDelegate FOnReplaceUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDeleteUserRecordCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FString& /*Error*/);
typedef FOnDeleteUserRecordCompleted::FDelegate FOnDeleteUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnBulkGetPublicUserRecordCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FListAccelByteModelsUserRecord&, const FString& /*Error*/);
typedef FOnBulkGetPublicUserRecordCompleted::FDelegate FOnBulkGetPublicUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetGameRecordCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsGameRecord&, const FString& /*Error*/);
typedef FOnGetGameRecordCompleted::FDelegate FOnGetGameRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnReplaceGameRecordCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FString& /*Error*/);
typedef FOnReplaceGameRecordCompleted::FDelegate FOnReplaceGameRecordCompletedDelegate;

/**
 * Implementation of Cloud Save service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineCloudSaveAccelByte : public TSharedFromThis<FOnlineCloudSaveAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	FOnlineCloudSaveAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
		: AccelByteSubsystem(InSubsystem)
	{}

	TMap<FString, TSharedRef<FAccelByteModelsGameRecord>> GameRecordMap;
	/** Critical sections for thread safe operation of CurrencyCodeToCurrencyListMap */
	mutable FCriticalSection GameRecordMapLock;
public:
	virtual ~FOnlineCloudSaveAccelByte() {};

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineCloudSaveAccelBytePtr& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, FOnlineCloudSaveAccelBytePtr& OutInterfaceInstance);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetUserRecordCompleted, bool, const FAccelByteModelsUserRecord&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnReplaceUserRecordCompleted, bool, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnDeleteUserRecordCompleted, bool, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnBulkGetPublicUserRecordCompleted, bool, const FListAccelByteModelsUserRecord&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetGameRecordCompleted, bool, const FAccelByteModelsGameRecord&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnReplaceGameRecordCompleted, bool, const FString&);

	/**
	 * @brief Get a record (arbitrary JSON data) by its key in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 */
	bool GetUserRecord(int32 LocalUserNum, const FString& Key);

	/**
	 * @brief Get a public record (arbitrary JSON data) by its key in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 */
	bool GetPublicUserRecord(int32 LocalUserNum, const FString& Key);

	/**
	 * @brief Replace a record in user-level. If the record doesn't exist, it will create and save the record. If already exists, it will replace the existing one.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to replace
	 * @param RecordRequest Record to replace in the form of Json object
	 */
	bool ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest);
	
	/**
	 * @brief Replace a record in user-level. If the record doesn't exist, it will create and save the record. If already exists, it will replace the existing one.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to replace
	 * @param RecordRequest Record to replace in the form of Json object
	 */
	bool ReplacePublicUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest);

	/**
	 * @brief Delete a record under the given key in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 */
	bool DeleteUserRecord(int32 LocalUserNum, const FString& Key);

	/**
	 * @brief Get a public record (arbitrary JSON data) by its key and userId in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get bulk public user record
	 * @param Key Key of record.
	 * @param UserIds List UserId(s) of the record owner.
	 */
	bool BulkGetPublicUserRecord(int32 LocalUserNum, const FString& Key, const TArray<FString>& UserIds);

	/**
	 * @brief Get a record by its key in namespace-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 */
	bool GetGameRecord(int32 LocalUserNum, const FString& Key, bool bAlwaysRequestToService = false);

	/**
	 * @brief Get a record by its key in namespace-level from cache.
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 */
	bool GetGameRecordFromCache(const FString& Key, FAccelByteModelsGameRecord& OutRecord);


	/**
	 * @brief Replace a record by its key in namespace-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 * @param RecordRequest Record to replace in the form of Json object
	 */
	bool ReplaceGameRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest);
	void AddGameRecordToMap(const FString& Key, const TSharedRef<FAccelByteModelsGameRecord>& Record);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineCloudSaveAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	bool GetUserRecord(int32 LocalUserNum, const FString& Key, bool IsPublic);
	bool ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, bool IsPublic);
};