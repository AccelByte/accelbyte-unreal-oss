// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteGetUserRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteReplaceUserRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteDeleteUserRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteBulkGetPublicUserRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteGetGameRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteReplaceGameRecord.h"

bool FOnlineCloudSaveAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineCloudSaveAccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetCloudSaveInterface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineCloudSaveAccelByte::GetFromWorld(const UWorld* World, FOnlineCloudSaveAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineCloudSaveAccelByte::GetUserRecord(int32 LocalUserNum, const FString& Key)
{
	return GetUserRecord(LocalUserNum, Key, false);
}

bool FOnlineCloudSaveAccelByte::GetUserRecord(int32 LocalUserNum, const FString& Key, const FUniqueNetIdAccelByteUserRef& RecordUserId)
{
	const FString UserId = RecordUserId->GetAccelByteId();
	return GetUserRecord(LocalUserNum, Key, false, UserId);
}

bool FOnlineCloudSaveAccelByte::GetPublicUserRecord(int32 LocalUserNum, const FString& Key)
{
	return GetUserRecord(LocalUserNum, Key, true);
}

bool FOnlineCloudSaveAccelByte::GetPublicUserRecord(int32 LocalUserNum, const FString& Key, const FUniqueNetIdAccelByteUserRef& RecordUserId)
{
	const FString UserId = RecordUserId->GetAccelByteId();
	return GetUserRecord(LocalUserNum, Key, true, UserId);
}

bool FOnlineCloudSaveAccelByte::ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest)
{
	return ReplaceUserRecord(LocalUserNum, Key, RecordRequest, false);
}

bool FOnlineCloudSaveAccelByte::ReplacePublicUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest)
{
	return ReplaceUserRecord(LocalUserNum, Key, RecordRequest, true);
}

bool FOnlineCloudSaveAccelByte::DeleteUserRecord(int32 LocalUserNum, const FString& Key)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteUserRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key);
				return true;
			}
			const FString ErrorStr = TEXT("delete-user-record-failed-userid-invalid");
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
			return false;
		}
	}

	const FString ErrorStr = TEXT("delete-user-record-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
	return false;
}

bool FOnlineCloudSaveAccelByte::BulkGetPublicUserRecord(int32 LocalUserNum, const FString& Key, const TArray<FString>& UserIds)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key, UserIds);
				return true;
			}
			const FString ErrorStr = TEXT("bulk-get-public-user-record-failed-userid-invalid");
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, false, FListAccelByteModelsUserRecord(), ErrorStr);
			return false;
		}
	}

	const FString ErrorStr = TEXT("bulk-get-public-user-records-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, false, FListAccelByteModelsUserRecord(), ErrorStr);
	return false;
}

bool FOnlineCloudSaveAccelByte::BulkGetPublicUserRecord(int32 LocalUserNum, const FString& Key, const TArray<FUniqueNetIdAccelByteUserRef>& UniqueNetIds)
{
	TArray<FString> UserIds{};
	for (const auto& UniqueNetId : UniqueNetIds)
	{
		UserIds.Add(UniqueNetId->GetAccelByteId());
	}
	return BulkGetPublicUserRecord(LocalUserNum, Key, UserIds);
}

bool FOnlineCloudSaveAccelByte::GetGameRecord(int32 LocalUserNum, const FString& Key, bool bAlwaysRequestToService)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetGameRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key, bAlwaysRequestToService);
				return true;
			}
			const FString ErrorStr = TEXT("get-game-record-failed-userid-invalid");
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, false, FAccelByteModelsGameRecord(), ErrorStr);
			return false;
		}
	}

	const FString ErrorStr = TEXT("get-game-records-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, false, FAccelByteModelsGameRecord(), ErrorStr);
	return false;
}

bool FOnlineCloudSaveAccelByte::ReplaceGameRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceGameRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key, RecordRequest);
				return true;
			}
			const FString ErrorStr = TEXT("replace-game-record-failed-userid-invalid");
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
			return false;
		}
	}

	const FString ErrorStr = TEXT("replace-game-record-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
	return false;
}

bool FOnlineCloudSaveAccelByte::GetGameRecordFromCache(const FString& Key, FAccelByteModelsGameRecord& OutRecord)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Get Game Record, Key: %s"), *Key);
	FScopeLock ScopeLock(&GameRecordMapLock);

	if (GameRecordMap.Num() > 0)
	{
		auto Record = GameRecordMap.Find(Key);
		if (Record != nullptr)
		{
			OutRecord = Record->Get();
			AB_OSS_INTERFACE_TRACE_END(TEXT("Game Record found for key %s"), *Key);
			return true;
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Game Record not found for key %s"), *Key);
	return false;
}

bool FOnlineCloudSaveAccelByte::GetUserRecord(int32 LocalUserNum, const FString& Key, bool IsPublic, const FString& UserId)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (!IsRunningDedicatedServer())
			{
				if (UserIdPtr.IsValid())
				{
					AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserRecord>(AccelByteSubsystem, LocalUserNum, *UserIdPtr.Get(), Key, IsPublic, UserId);
					return true;
				}
				const FString ErrorStr = TEXT("get-user-record-failed-userid-invalid");
				AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, false, FAccelByteModelsUserRecord(), ErrorStr);
				return false;
			}
			else
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserRecord>(AccelByteSubsystem, LocalUserNum, *UserIdPtr.Get(), Key, IsPublic, UserId);
				return true;
			}
		}
	}

	const FString ErrorStr = TEXT("get-user-record-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, false, FAccelByteModelsUserRecord(), ErrorStr);
	return false;
}

bool FOnlineCloudSaveAccelByte::ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, bool IsPublic)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceUserRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key, RecordRequest, IsPublic);
				return true;
			}
			const FString ErrorStr = TEXT("replace-user-record-failed-userid-invalid");
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
			return false;
		}
	}

	const FString ErrorStr = TEXT("replace-user-record-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, false, ErrorStr);
	return false;
}


void FOnlineCloudSaveAccelByte::AddGameRecordToMap(const FString& Key, const TSharedRef<FAccelByteModelsGameRecord>& Record)
{
	FScopeLock ScopeLock(&GameRecordMapLock);
	GameRecordMap.Add(Key, Record);
}