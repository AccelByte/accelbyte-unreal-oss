// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "OnlineUserCacheAccelByte.h"
#include "Containers/Map.h"
#include "Models/AccelByteCloudSaveModels.h"
#include "OnlineSubsystemAccelBytePackage.h"


DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSaveUserBinaryRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FString& /*Key*/);
typedef FOnSaveUserBinaryRecordCompleted::FDelegate FOnSaveUserBinaryRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDeleteUserBinaryRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FString& /*Key*/);
typedef FOnDeleteUserBinaryRecordCompleted::FDelegate FOnDeleteUserBinaryRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBulkGetUserBinaryRecordsCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsListUserBinaryRecords&);
typedef FOnBulkGetUserBinaryRecordsCompleted::FDelegate FOnBulkGetUserBinaryRecordsCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetUserBinaryRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsUserBinaryRecord);
typedef FOnGetUserBinaryRecordCompleted::FDelegate FOnGetUserBinaryRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetGameBinaryRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsGameBinaryRecord);
typedef FOnGetGameBinaryRecordCompleted::FDelegate FOnGetGameBinaryRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBulkGetGameBinaryRecordsCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsListGameBinaryRecords);
typedef FOnBulkGetGameBinaryRecordsCompleted::FDelegate FOnBulkGetGameBinaryRecordsCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBulkQueryGameBinaryRecordsCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsPaginatedGameBinaryRecords);
typedef FOnBulkQueryGameBinaryRecordsCompleted::FDelegate FOnBulkQueryGameBinaryRecordsCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnUpdateUserBinaryRecordCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsUserBinaryRecord);
typedef FOnUpdateUserBinaryRecordCompleted::FDelegate FOnUpdateUserBinaryRecordCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBulkQueryUserBinaryRecordsCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsPaginatedUserBinaryRecords);
typedef FOnBulkQueryUserBinaryRecordsCompleted::FDelegate FOnBulkQueryUserBinaryRecordsCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnRequestUserBinaryRecordPresignedUrlCompleted, int32 /*LocalUserNum*/, const FOnlineError& /*Result*/, const FAccelByteModelsBinaryInfo);
typedef FOnRequestUserBinaryRecordPresignedUrlCompleted::FDelegate FOnRequestUserBinaryRecordPresignedUrlCompletedDelegate;

/**
 * Implementation of Cloud Save service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineBinaryCloudSaveAccelByte : public TSharedFromThis<FOnlineBinaryCloudSaveAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	FOnlineBinaryCloudSaveAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
	{}

public:
	virtual ~FOnlineBinaryCloudSaveAccelByte() {};

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineBinaryCloudSaveAccelBytePtr& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, FOnlineBinaryCloudSaveAccelBytePtr& OutInterfaceInstance);

	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnSaveUserBinaryRecordCompleted, const FOnlineError&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnDeleteUserBinaryRecordCompleted, const FOnlineError&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnBulkGetUserBinaryRecordsCompleted, const FOnlineError&, const FAccelByteModelsListUserBinaryRecords&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnGetUserBinaryRecordCompleted, const FOnlineError&, const FAccelByteModelsUserBinaryRecord&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnGetGameBinaryRecordCompleted, const FOnlineError&, const FAccelByteModelsGameBinaryRecord&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnBulkGetGameBinaryRecordsCompleted, const FOnlineError&, const FAccelByteModelsListGameBinaryRecords&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnBulkQueryGameBinaryRecordsCompleted, const FOnlineError&, const FAccelByteModelsPaginatedGameBinaryRecords&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnUpdateUserBinaryRecordCompleted, const FOnlineError&, const FAccelByteModelsUserBinaryRecord&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnBulkQueryUserBinaryRecordsCompleted, const FOnlineError&, const FAccelByteModelsPaginatedUserBinaryRecords&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnRequestUserBinaryRecordPresignedUrlCompleted, const FOnlineError&, const FAccelByteModelsBinaryInfo&);
	
	/**
	 * @brief Save a namespace-level user binary.
	 * If the binary doesn't exist, it will create the binary save, if already exists, it will append to the existing one.
	 *
	 * @param Key Key of the binary record.
	 * @param FileType File type of the binary (supported file types are jpeg, jpg, png, bmp, gif, mp3, webp, and bin).
	 * @param bIsPublic Whether to save the binary as a public or private record.
	 */
	bool SaveUserBinaryRecord(int32 LocalUserNum, FString const& Key, FString const& FileType, bool bIsPublic);

	/**
	 * @brief Save a namespace-level user binary.
	 * If the binary doesn't exist, it will create the binary save, if already exists, it will append to the existing one.
	 *
	 * @param Key Key of the binary record.
	 * @param FileType File type of the binary (supported file types are jpeg, jpg, png, bmp, gif, mp3, webp, and bin).
	 * @param bIsPublic Whether to save the binary as a public or private record.
	 */
	bool SaveUserBinaryRecord(int32 LocalUserNum, FString const& Key, EAccelByteFileType FileType, bool bIsPublic);
	
	/**
	 * @brief Get current user's binary record by its key in namespace-level.
	 *
	 * @param Key Key of the binary record.
	 */
	bool GetCurrentUserBinaryRecord(int32 LocalUserNum, FString const& Key);

	/**
	 * @brief Get a user's public binary record by its key and the owner's UserId in namespace-level.
	 *
	 * @param Key Key of the binary record.
	 * @param UserId UserId of the binary record owner.
	 */
	bool GetPublicUserBinaryRecord(int32 LocalUserNum, FString const& Key, FString const& UserId);

	/**
	 * @brief Bulk get current user's binary records by their keys.
	 *
	 * @param Keys List of keys of the binary records.
	 */
	bool BulkGetCurrentUserBinaryRecords(int32 LocalUserNum, const TArray<FString>& Keys);

	/**
	 * @brief Bulk get multiple public binary records from a single user by their keys.
	 *
	 * @param Keys List of key(s) of the binary record.
	 * @param UserId UserId of the binary record owner.
	 */
	bool BulkGetPublicUserBinaryRecords(int32 LocalUserNum, const TArray<FString>& Keys, FString const& UserId);

	/**
	 * @brief Bulk get public binary records with the same key from multiple users.
	 *
	 * @param Key Key of the binary record.
	 * @param UserIds UserId list of the binary record owner(s).
	 */
	bool BulkGetPublicUserBinaryRecords(int32 LocalUserNum, FString const& Key, const TArray<FString>& UserIds);

	/**
	 * @brief Bulk query current user's binary records.
	 *
	 * @param Query String that will be used to query the user's keys.
	 * @param Offset The offset of the result. Default value is 0.
	 * @param Limit The limit of the result. Default value is 20.
	 */
	bool BulkQueryCurrentUserBinaryRecords(int32 LocalUserNum, FString const& Query, int32 const& Offset = 0, int32 const& Limit = 20);

	/**
	 * @brief Bulk query all of a user's public binary records.
	 *
	 * @param UserId UserId of the binary record owner.
	 * @param Offset The offset of the result. Default value is 0.
	 * @param Limit The limit of the result. Default value is 20.
	 */
	bool BulkQueryPublicUserBinaryRecords(int32 LocalUserNum, FString const& UserId, int32 const& Offset = 0, int32 const& Limit = 20);

	/**
	 * @brief Update current user's binary record to point to an uploaded file.
	 *
	 * @param Key Key of the binary record.
	 * @param FileType File type of the uploaded binary (supported file types are jpeg, jpg, png, bmp, gif, mp3, webp, and bin).
	 * @param FileLocation Location of the uploaded binary file.
	 */
	bool UpdateUserBinaryRecordFile(int32 LocalUserNum, FString const& Key, FString const& FileType, FString const& FileLocation);

	/**
	 * @brief Update current user's binary record to point to an uploaded file.
	 *
	 * @param Key Key of the binary record.
	 * @param ContentType The specific type of the binary record created.
	 * @param FileLocation Location of the uploaded binary file.
	 */
	bool UpdateUserBinaryRecordFile(int32 LocalUserNum, FString const& Key, EAccelByteFileType ContentType, FString const& FileLocation);

	/**
	 * @brief Update current user's binary record's metadata in namespace-level.
	 *
	 * @param Key Key of the binary record.
	 * @param IsPublic Whether to update the binary into a public or private record.
	 */
	bool UpdateUserBinaryRecordMetadata(int32 LocalUserNum, FString const& Key, bool bIsPublic);

	/**
	 * @brief Delete current user's binary record under the given key in namespace-level.
	 *
	 * @param Key Key of the binary record.
	 */
	bool DeleteUserBinaryRecord(int32 LocalUserNum, FString const& Key);

	/**
	 * @brief Request a presigned url to upload current user's binary record file.
	 *
	 * @param Key Key of the binary record.
	 * @param FileType File type of the binary (supported file types are jpeg, jpg, png, bmp, gif, mp3, webp, and bin).
	 */
	bool RequestUserBinaryRecordPresignedUrl(int32 LocalUserNum, FString const& Key, FString const& FileType);

	/**
	 * @brief Request a presigned url to upload current user's binary record file.
	 *
	 * @param Key Key of the binary record.
	 * @param FileType File type of the binary (supported file types are jpeg, jpg, png, bmp, gif, mp3, webp, and bin).
	 */
	bool RequestUserBinaryRecordPresignedUrl(int32 LocalUserNum, FString const& Key, EAccelByteFileType FileType);

	/**
	 * @brief Get a game binary record by its key.
	 *
	 * @param Key Key of the binary record.
	 */
	bool GetGameBinaryRecord(int32 LocalUserNum, FString const& Key);

	/**
	 * @brief Bulk get game binary records by their keys.
	 *
	 * @param Keys List of keys of the binary record.
	 */
	bool BulkGetGameBinaryRecords(int32 LocalUserNum, TArray<FString> const& Keys);

	/**
	 * @brief Bulk query game binary records.
	 *
	 * @param Query String that will be used to query the game's binary record keys.
	 * @param Offset The offset of the result. Default value is 0.
	 * @param Limit The limit of the result. Default value is 20.
	 */
	bool BulkQueryGameBinaryRecords(int32 LocalUserNum, FString const& Query, int32 const& Offset = 0, int32 const& Limit = 20);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineBinaryCloudSaveAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;
};