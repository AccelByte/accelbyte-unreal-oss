// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineUserCloudInterface.h"

class FOnlineSubsystemAccelByte;
class IOnlineSubsystem;

using FFileNameToFileContentsMap = TMap<FString, TArray<uint8>>;
using FUserIdToFileNameFileContentsMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FFileNameToFileContentsMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FFileNameToFileContentsMap>>;

using FFileNameToFileHeaderMap = TMap<FString, FCloudFileHeader>;
using FUserIdToFileNameFileHeaderMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FFileNameToFileHeaderMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FFileNameToFileHeaderMap>>;

using FFileNameToSlotIdMap = TMap<FString, FString>;
using FUserIdToFileNameSlotIdMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FFileNameToSlotIdMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FFileNameToSlotIdMap>>;

/**
 * Implementation of the UserCloud interface using AccelByte services.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineUserCloudAccelByte : public IOnlineUserCloud, public TSharedFromThis<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe>
{
private:

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineUserCloudAccelByte()
		: AccelByteSubsystem(nullptr) {}

PACKAGE_SCOPE:

	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineUserCloudAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/**
	 * Used by async tasks to pass file name to file header associations back to this interface
	 */
	void AddCloudHeaders(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TMap<FString, FCloudFileHeader>& InFileNamesToCloudHeaders);

	/**
	 * Used by async tasks to pass file contents to the read cache for a user
	 */
	void AddFileContentsToReadCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName, TArray<uint8>&& FileContents);

	/** Used by async tasks to cache a slot ID associated with a file name for a user. */
	void AddSlotIdToCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName, const FString& SlotId);

	/** Used by the delete file async task to pop a slot ID from the cache */
	void RemoveSlotIdFromCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName);

	/**
	 * Used by async tasks to attempt to get a cached slot ID from a file name and user ID.
	 * 
	 * @return the ID of the slot associated with the file name, or blank if none found
	 */
	FString GetSlotIdFromCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName);

public:
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	//~ Begin IOnlineUserCloud async methods
	virtual void EnumerateUserFiles(const FUniqueNetId& UserId) override;
	virtual bool ReadUserFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual bool WriteUserFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents, bool bCompressBeforeUpload = false) override;
	virtual void CancelWriteUserFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual bool DeleteUserFile(const FUniqueNetId& UserId, const FString& FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete) override;
	virtual bool RequestUsageInfo(const FUniqueNetId& UserId) override;
	//~ End IOnlineUserCloud async methods

	//~ Begin IOnlineUserCloud cached methods
	virtual bool GetFileContents(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents) override;
	virtual bool ClearFiles(const FUniqueNetId& UserId) override;
	virtual bool ClearFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual void GetUserFileList(const FUniqueNetId& UserId, TArray<FCloudFileHeader>& UserFiles) override;
	virtual void DumpCloudState(const FUniqueNetId& UserId) override;
	virtual void DumpCloudFileState(const FUniqueNetId& UserId, const FString& FileName) override;
	//~ End IOnlineUserCloud cached methods

private:

	/** Cached map of file contents mapped to file names per user ID, will persist until GetFileContents is called. */
	FUserIdToFileNameFileContentsMap UserIdToFileNameFileContentsMap;

	/** Cached map of file headers mapped to file names per user ID */
	FUserIdToFileNameFileHeaderMap UserIdToFileNameFileHeaderMap;

	/**
	 * Cached map of file names to slot IDs per user ID.
	 *
	 * Intended to cut down on extra calls to GetAllSlots to resolve a file name to a slot ID.
	 */
	FUserIdToFileNameSlotIdMap UserIdToFileNameSlotIdMap;

};