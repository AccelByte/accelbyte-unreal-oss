// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineUserCloudInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/UserCloud/OnlineAsyncTaskAccelByteEnumerateUserFiles.h"
#include "AsyncTasks/UserCloud/OnlineAsyncTaskAccelByteReadUserFile.h"
#include "AsyncTasks/UserCloud/OnlineAsyncTaskAccelByteWriteUserFile.h"
#include "AsyncTasks/UserCloud/OnlineAsyncTaskAccelByteDeleteUserFile.h"
#include "OnlineSubsystemUtils.h"

using namespace AccelByte;

FOnlineUserCloudAccelByte::FOnlineUserCloudAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{
}

bool FOnlineUserCloudAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineUserCloudAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(Subsystem->GetUserCloudInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineUserCloudAccelByte::GetFromWorld(const UWorld* World, FOnlineUserCloudAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

void FOnlineUserCloudAccelByte::EnumerateUserFiles(const FUniqueNetId& UserId)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteEnumerateUserFiles>(AccelByteSubsystemPtr.Get(), UserId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatched async task to enumerate user files for user '%s'!"), *UserId.ToDebugString());
}

void FOnlineUserCloudAccelByte::AddCloudHeaders(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const TMap<FString, FCloudFileHeader>& InFileNamesToCloudHeaders)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	// Check if we already have a header map associated with the user
	FFileNameToFileHeaderMap* FoundHeaders = UserIdToFileNameFileHeaderMap.Find(UserId);
	if (FoundHeaders != nullptr)
	{
		// We do, add the new cloud headers to the end of the existing header map
		FoundHeaders->Append(InFileNamesToCloudHeaders);
	}
	else
	{
		// We don't, just add a new entry mapping the user ID to the cloud headers we want to add
		UserIdToFileNameFileHeaderMap.Add(UserId, InFileNamesToCloudHeaders);
	}
}

void FOnlineUserCloudAccelByte::AddFileContentsToReadCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName, TArray<uint8>&& FileContents)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	// Check if we already have a file read cache associated with the user
	FFileNameToFileContentsMap* FoundReadCache = UserIdToFileNameFileContentsMap.Find(UserId);
	if (FoundReadCache != nullptr)
	{
		// We already have a read cache associated, now check if we already have an array of bytes for this file
		TArray<uint8>* FoundCachedContents = FoundReadCache->Find(FileName);
		if (FoundCachedContents != nullptr)
		{
			// We do, just update the cached contents to be the new file contents read
			*FoundCachedContents = FileContents;
		}
	}
	else
	{
		// We don't have a read cache associated, create a new cache map, add the file contents that we just read to it and
		// add that cache map to the user ID to cache mapping
		FFileNameToFileContentsMap UserReadCacheMap;
		UserReadCacheMap.Add(FileName, FileContents);
		UserIdToFileNameFileContentsMap.Add(UserId, UserReadCacheMap);
	}
}

void FOnlineUserCloudAccelByte::AddSlotIdToCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName, const FString& SlotId)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	FFileNameToSlotIdMap* FoundSlotCache = UserIdToFileNameSlotIdMap.Find(UserId);
	if (FoundSlotCache != nullptr && !FoundSlotCache->Contains(FileName))
	{
		FoundSlotCache->Add(FileName, SlotId);
	}
	else
	{
		FFileNameToSlotIdMap FileNameToSlotIdMap;
		FileNameToSlotIdMap.Add(FileName, SlotId);
		UserIdToFileNameSlotIdMap.Add(UserId, FileNameToSlotIdMap);
	}
}

void FOnlineUserCloudAccelByte::RemoveSlotIdFromCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	FFileNameToSlotIdMap* FoundSlotCache = UserIdToFileNameSlotIdMap.Find(UserId);
	if (FoundSlotCache != nullptr && FoundSlotCache->Contains(FileName))
	{
		FoundSlotCache->Remove(FileName);
	}
}

FString FOnlineUserCloudAccelByte::GetSlotIdFromCache(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, const FString& FileName)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	FFileNameToSlotIdMap* FoundSlotCache = UserIdToFileNameSlotIdMap.Find(UserId);
	if (FoundSlotCache != nullptr)
	{
		const FString* FoundCachedSlot = FoundSlotCache->Find(FileName);
		if (FoundCachedSlot != nullptr)
		{
			return *FoundCachedSlot;
		}
	}

	return FString();
}

bool FOnlineUserCloudAccelByte::ReadUserFile(const FUniqueNetId& UserId, const FString& FileName)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId.ToDebugString(), *FileName);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReadUserFile>(AccelByteSubsystemPtr.Get(), UserId, FileName);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatched async task to read file '%s' for user '%s'!"), *FileName, *UserId.ToDebugString());
	return true;
}

bool FOnlineUserCloudAccelByte::GetFileContents(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId.ToDebugString(), *FileName);

	// Check if we have a cache of files read for this user
	FFileNameToFileContentsMap* FoundReadCache = UserIdToFileNameFileContentsMap.Find(FUniqueNetIdAccelByteUser::CastChecked(UserId));
	if (FoundReadCache == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not get file (%s) contents as user (%s) has no read cache!"), *FileName, *UserId.ToDebugString());
		return false;
	}

	// Check if there is a byte array of file contents corresponding with the file name in the read cache
	if (!FoundReadCache->Contains(FileName))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Could not get file (%s) contents as the file was not found in user's (%s) read cache!"), *FileName, *UserId.ToDebugString());
		return false;
	}

	// Once we know we have a file in the read cache for the user, copy it to the FileContents array and remove the cached contents
	FileContents = FoundReadCache->FindAndRemoveChecked(FileName);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found file (%s) contents in user's (%s) read cache! Contents size: %d"), *FileName, *UserId.ToDebugString(), FileContents.Num());
	return true;
}

bool FOnlineUserCloudAccelByte::ClearFiles(const FUniqueNetId& UserId)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	FFileNameToFileContentsMap* FoundContentsMap = UserIdToFileNameFileContentsMap.Find(FUniqueNetIdAccelByteUser::CastChecked(UserId));
	if (FoundContentsMap != nullptr)
	{
		FoundContentsMap->Empty();
		return true;
	}

	return false;
}

bool FOnlineUserCloudAccelByte::ClearFile(const FUniqueNetId& UserId, const FString& FileName)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	FFileNameToFileContentsMap* FoundContentsMap = UserIdToFileNameFileContentsMap.Find(FUniqueNetIdAccelByteUser::CastChecked(UserId));
	if (FoundContentsMap != nullptr)
	{
		if (FoundContentsMap->Contains(FileName))
		{
			FoundContentsMap->Remove(FileName);
			return true;
		}
	}

	return false;
}

void FOnlineUserCloudAccelByte::GetUserFileList(const FUniqueNetId& UserId, TArray<FCloudFileHeader>& UserFiles)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	FFileNameToFileHeaderMap* UserCloudFiles = UserIdToFileNameFileHeaderMap.Find(FUniqueNetIdAccelByteUser::CastChecked(UserId));
	if (UserCloudFiles != nullptr)
	{
		UserFiles.Empty(UserCloudFiles->Num());
		UserCloudFiles->GenerateValueArray(UserFiles);
	}
}

void FOnlineUserCloudAccelByte::DumpCloudState(const FUniqueNetId& UserId)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	// at some point we may want this to log how many slots the user has used and how many the maximum is? along with maximum size per slot?
	UE_LOG_AB(Warning, TEXT("AccelByte OSS UserCloud implementation currently does not support DumpCloudState."));
}

void FOnlineUserCloudAccelByte::DumpCloudFileState(const FUniqueNetId& UserId, const FString& FileName)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	UE_LOG_AB(Log, TEXT("State for file '%s' owned by user '%s':"), *FileName, *UserId.ToDebugString());

	// First try and dump information about the header of the file that is owned by the user
	const FFileNameToFileHeaderMap* FoundHeaderMap = UserIdToFileNameFileHeaderMap.Find(FUniqueNetIdAccelByteUser::CastChecked(UserId));
	if (FoundHeaderMap != nullptr)
	{
		const FCloudFileHeader* FoundHeader = FoundHeaderMap->Find(FileName);
		if (FoundHeader != nullptr)
		{
			UE_LOG_AB(Log, TEXT("    File name: %s; File size: %d"), *FoundHeader->FileName, FoundHeader->FileSize);
		}
		else
		{
			UE_LOG_AB(Log, TEXT("    Unable to get file header."));
		}
	}
	else
	{
		UE_LOG_AB(Log, TEXT("    Unable to get file header."));
	}

	// Then try and dump information about the cached contents, really just the size of the contents we have
	const FFileNameToFileContentsMap* FoundContentsMap = UserIdToFileNameFileContentsMap.Find(FUniqueNetIdAccelByteUser::CastChecked(UserId));
	if (FoundContentsMap != nullptr)
	{
		const TArray<uint8>* FoundContents = FoundContentsMap->Find(FileName);
		if (FoundContents != nullptr)
		{
			UE_LOG_AB(Log, TEXT("    Cached contents size: %d"), FoundContents->Num());
		}
		else
		{
			UE_LOG_AB(Log, TEXT("    Unable to get cached file contents."));
		}
	}
	else
	{
		UE_LOG_AB(Log, TEXT("    Unable to get cached file contents."));
	}
}

bool FOnlineUserCloudAccelByte::WriteUserFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents, bool bCompressBeforeUpload)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s; FileContents Size: %d; bCompressBeforeUpload: %s"), *UserId.ToDebugString(), *FileName, FileContents.Num(), LOG_BOOL_FORMAT(bCompressBeforeUpload));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteWriteUserFile>(AccelByteSubsystemPtr.Get(), UserId, FileName, FileContents, bCompressBeforeUpload);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatched async task to write user file to CloudStorage."));
	return true;
}

void FOnlineUserCloudAccelByte::CancelWriteUserFile(const FUniqueNetId& UserId, const FString& FileName)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	// I don't believe that canceling requests is supported by the SDK currently?
	UE_LOG_AB(Warning, TEXT("AccelByte OSS UserCloud implementation currently does not support CancelWriteUserFile."));
	AccelByteSubsystemPtr->ExecuteNextTick([UserCloudInterface = AsShared(), NetId = UserId.AsShared(), FileName]() {
		UserCloudInterface->TriggerOnWriteUserFileCanceledDelegates(false, NetId.Get(), FileName);
	});
}

bool FOnlineUserCloudAccelByte::DeleteUserFile(const FUniqueNetId& UserId, const FString& FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s; bShouldCloudDelete: %s; bShouldLocallyDelete: %s"), *UserId.ToDebugString(), *FileName, LOG_BOOL_FORMAT(bShouldCloudDelete), LOG_BOOL_FORMAT(bShouldLocallyDelete));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteUserFile>(AccelByteSubsystemPtr.Get(), UserId, FileName, bShouldCloudDelete, bShouldLocallyDelete);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatched async task to delete user file from CloudStorage."));
	return true;
}

bool FOnlineUserCloudAccelByte::RequestUsageInfo(const FUniqueNetId& UserId)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Cloud Storage is deprecated - please use Binary Cloudsave for the replacement"));
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	// SDK nor backend I don't think has a way to see how many slots/how much space a user is using relative to the
	// configured limits? I think technically I could run GetAllSlots and get the file size for each slot as the
	// "total used", however I don't think I'll be able to get the quota value?
	AccelByteSubsystemPtr->ExecuteNextTick([UserCloudInterface = AsShared(), NetId = UserId.AsShared()]() {
		UserCloudInterface->TriggerOnRequestUsageInfoCompleteDelegates(false, NetId.Get(), 0, 0);
	});
	return false;
}

