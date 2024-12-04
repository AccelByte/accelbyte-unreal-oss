// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelBytePackage.h"
#include "Containers/Map.h"

using FBulkReplaceUserRecordMap = TMap< FUniqueNetIdAccelByteUserRef, FAccelByteModelsBulkReplaceUserRecordResponse>;
using FBulkGetUserRecordMap = TMap< FUniqueNetIdAccelByteUserRef, FAccelByteModelsUserRecord>;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetUserRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/,const FString& /*Key*/, const FAccelByteModelsUserRecord&);
typedef FOnGetUserRecordCompleted::FDelegate FOnGetUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBulkGetUserRecordCompleted, const FOnlineError& /*Result*/,const FString& /*Key*/, const FBulkGetUserRecordMap&);
typedef FOnBulkGetUserRecordCompleted::FDelegate FOnBulkGetUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnReplaceUserRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/,const FString& /*Key*/);
typedef FOnReplaceUserRecordCompleted::FDelegate FOnReplaceUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBulkReplaceUserRecordCompleted, const FOnlineError& /*Result*/,const FString& /*Key*/, const FBulkReplaceUserRecordMap& /*Result*/);
typedef FOnBulkReplaceUserRecordCompleted::FDelegate FOnBulkReplaceUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDeleteUserRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/,const FString&/*Key*/);
typedef FOnDeleteUserRecordCompleted::FDelegate FOnDeleteUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBulkGetPublicUserRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FListAccelByteModelsUserRecord&);
typedef FOnBulkGetPublicUserRecordCompleted::FDelegate FOnBulkGetPublicUserRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetGameRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/,const FString& /*Key*/, const FAccelByteModelsGameRecord&);
typedef FOnGetGameRecordCompleted::FDelegate FOnGetGameRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnReplaceGameRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/,const FString& /*Key*/);
typedef FOnReplaceGameRecordCompleted::FDelegate FOnReplaceGameRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDeleteGameRecordTTLConfigCompleted, const FOnlineError& /*Result*/,const FString&/*Key*/);
typedef FOnDeleteGameRecordTTLConfigCompleted::FDelegate FOnDeleteGameRecordTTLConfigCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetAdminGameRecordCompleted, const FOnlineError& /*Result*/, const FString& /*Key*/, const FAccelByteModelsAdminGameRecord&);
typedef FOnGetAdminGameRecordCompleted::FDelegate FOnGetAdminGameRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnModifyAdminGameRecordCompleted, const FOnlineError& /*Result*/, const FString& /*Key*/);
typedef FOnModifyAdminGameRecordCompleted::FDelegate FOnModifyAdminGameRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDeleteAdminGameRecordTTLConfigCompleted, const FOnlineError& /*Result*/,const FString&/*Key*/);
typedef FOnDeleteAdminGameRecordTTLConfigCompleted::FDelegate FOnDeleteAdminGameRecordTTLConfigCompletedDelegate;

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
	/** Critical sections for thread safe operation of GameRecordMap */
	mutable FCriticalSection GameRecordMapLock;

	TMap<FString, TSharedRef<FAccelByteModelsAdminGameRecord>> AdminGameRecordMap;
	/** Critical sections for thread safe operation of AdminGameRecordMap */
	mutable FCriticalSection AdminGameRecordMapLock;
		
	void AddGameRecordToMap(const FString& Key, const TSharedRef<FAccelByteModelsGameRecord>& Record);
	void AddAdminGameRecordToMap(const FString& Key, const TSharedRef<FAccelByteModelsAdminGameRecord>& Record);
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

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetUserRecordCompleted, const FOnlineError&, const FString&, const FAccelByteModelsUserRecord&);
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnBulkGetUserRecordCompleted, const FOnlineError&,const FString&, const FBulkGetUserRecordMap&)
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnReplaceUserRecordCompleted, const FOnlineError&,const FString&);
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnBulkReplaceUserRecordCompleted, const FOnlineError&,const FString&, const FBulkReplaceUserRecordMap&)
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnDeleteUserRecordCompleted, const FOnlineError&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnBulkGetPublicUserRecordCompleted, const FOnlineError&, const FListAccelByteModelsUserRecord&);
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetGameRecordCompleted, const FOnlineError&, const FString&, const FAccelByteModelsGameRecord&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnReplaceGameRecordCompleted, const FOnlineError&,const FString&);
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnDeleteGameRecordTTLConfigCompleted, const FOnlineError&, const FString&);
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnGetAdminGameRecordCompleted, const FOnlineError&, const FString&, const FAccelByteModelsAdminGameRecord&);
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnModifyAdminGameRecordCompleted, const FOnlineError&,const FString&);
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnDeleteAdminGameRecordTTLConfigCompleted, const FOnlineError&, const FString&);

	/**
	 * @brief Get a record (arbitrary JSON data) by its key in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 */
	bool GetUserRecord(int32 LocalUserNum, const FString& Key);

	/**
	 * @brief Get a record (arbitrary JSON data) by its key in user-level. Only for request by Game Server
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 * @param RecordUserId The UserId of the record owner
	 */
	bool GetUserRecord(int32 LocalUserNum, const FString& Key, const FUniqueNetIdAccelByteUserRef& RecordUserId);

	/**
	 * @brief Get a public record (arbitrary JSON data) by its key in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 */
	bool GetPublicUserRecord(int32 LocalUserNum, const FString& Key);

	/**
	 * @brief Get a public record (arbitrary JSON data) by its key in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 * @param RecordUserId The UserId of the record owner
	 */
	bool GetPublicUserRecord(int32 LocalUserNum, const FString& Key, const FUniqueNetIdAccelByteUserRef& RecordUserId);

	/**
	 * @brief Replace a record in user-level. If the record doesn't exist, it will create and save the record. If already exists, it will replace the existing one.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to replace
	 * @param RecordRequest Record to replace in the form of Json object
	 * @param TargetUserId Target of AccelByte User Net Id for server to call the request
	 */
	bool ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, const FUniqueNetIdAccelByteUserRef& TargetUserId = FUniqueNetIdAccelByteUser::Invalid());
	
	/**
	 * @brief Replace a record in user-level. If the record doesn't exist, it will create and save the record. If already exists, it will replace the existing one.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to replace
	 * @param RecordRequest Record to replace in the form of Json object
	 * @param TargetUserId Target of AccelByte User Net Id for server to call the request
	 */
	bool ReplacePublicUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, const FUniqueNetIdAccelByteUserRef& TargetUserId = FUniqueNetIdAccelByteUser::Invalid());

	/**
	* @brief Replace a record in user-level. If the record doesn't exist, it will create and save the record. If already exists, it will replace the existing one.
	*
	* @param Key The record key to replace
	* @param Request contain the userId and the record value to replace
	*/
	bool BulkReplaceUserRecord(const FString& Key, const TMap <FUniqueNetIdAccelByteUserRef, FJsonObject>& Request);

	/**
	 * @brief Delete a record under the given key in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get record
	 * @param Key The record key to delete
	 * @param TargetUserId Target of AccelByte User Net Id for server to call the request
	 */
	bool DeleteUserRecord(int32 LocalUserNum, const FString& Key, const FUniqueNetIdAccelByteUserRef& TargetUserId = FUniqueNetIdAccelByteUser::Invalid());

	/**
	* @brief Get a record (arbitrary JSON data) by its key and userId in user-level. Only for request by Game Server
	*
	* @param Key Key of record.
	* @param UniqueNetIds List UniqueNetId(UserId)(s) of the record owner.
	*/
	bool BulkGetUserRecord(const FString& Key, const TArray<FUniqueNetIdAccelByteUserRef>& UniqueNetIds);

	/**
	 * @brief Get a public record (arbitrary JSON data) by its key and userId in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get bulk public user record
	 * @param Key Key of record.
	 * @param UserIds List UserId(s) of the record owner.
	 */
	bool BulkGetPublicUserRecord(int32 LocalUserNum, const FString& Key, const TArray<FString>& UserIds);

	/**
	 * @brief Get a public record (arbitrary JSON data) by its key and userId in user-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get bulk public user record
	 * @param Key Key of record.
	 * @param UniqueNetIds List UniqueNetId(UserId)(s) of the record owner.
	 */
	bool BulkGetPublicUserRecord(int32 LocalUserNum, const FString& Key, const TArray<FUniqueNetIdAccelByteUserRef>& UniqueNetIds);

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

	/**
	 * @brief Replace a record by its key in namespace-level.
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 * @param SetBy Metadata record set by server or client
	 * @param RecordRequest Record to replace in the form of Json object
	 * @param TTLConfig The configuration to control the action taken if the record has expired. If the action set to NONE, then it will not send the TTL (Time to live) meta data.
	 */
	bool ReplaceGameRecord(int32 LocalUserNum, const FString& Key, ESetByMetadataRecord SetBy, const FJsonObject& RecordRequest, const FTTLConfig& TTLConfig);

	/**
	 * @brief Delete the ttl config of the game record.
	 *
	 * @param LocalUserNum Index of user that is attempting to delete the ttl config
	 * @param Key The record key to delete
	 */
	bool DeleteGameRecordTTLConfig(int32 LocalUserNum, const FString& Key);
	
	/**
	 * @brief Get a record by its key in namespace-level. Only for request by Game Server
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 */
	bool GetAdminGameRecord(int32 LocalUserNum, const FString& Key, bool bAlwaysRequestToService = false);

	/**
	 * @brief Get a record by its key in namespace-level from cache. Only for request by Game Server
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 */
	bool GetAdminGameRecordFromCache(const FString& Key, FAccelByteModelsAdminGameRecord& OutRecord);

	/**
	 * @brief Replace a record by its key in namespace-level. Only for request by Game Server
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 * @param RecordRequest Record to replace in the form of Json object
	 * @param Tags the tags of the record.
	 * @param TTLConfig The configuration to control the action taken if the record has expired. If the action set to NONE, then it will not send the TTL (Time to live) meta data.
	 */
	bool CreateAdminGameRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, const TArray<FString>& Tags, const FTTLConfig& TTLConfig);

	/**
	 * @brief Replace a record by its key in namespace-level. Only for request by Game Server
	 *
	 * @param LocalUserNum Index of user that is attempting to get game record
	 * @param Key The record key to get
	 * @param RecordRequest Record to replace in the form of Json object
	 * @param TTLConfig The configuration to control the action taken if the record has expired. If the action set to NONE, then it will not send the TTL (Time to live) meta data.
	 */
	bool ReplaceAdminGameRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, const FTTLConfig& TTLConfig);

	/**
	 * @brief Delete the ttl config of the admin game record.
	 *
	 * @param LocalUserNum Index of user that is attempting to delete the ttl config
	 * @param Key The record key to delete
	 */
	bool DeleteAdminGameRecordTTLConfig(int32 LocalUserNum, const FString& Key);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineCloudSaveAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	bool GetUserRecord(int32 LocalUserNum, const FString& Key, bool IsPublic, const FString& UserId = TEXT(""));
	bool ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, bool IsPublic, const FUniqueNetIdAccelByteUserRef& TargetUserId = FUniqueNetIdAccelByteUser::Invalid());
};