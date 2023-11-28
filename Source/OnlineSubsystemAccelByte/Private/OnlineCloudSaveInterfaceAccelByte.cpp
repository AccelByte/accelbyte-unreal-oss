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
#include "OnlineError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineCloudSaveAccelByte"

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
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
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
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to delete user record, identity interface is invalid!"));
		TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key);
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key);
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserIdPtr.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key);
		return false;
	}
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteUserRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key);
	return true;
}

bool FOnlineCloudSaveAccelByte::BulkGetPublicUserRecord(int32 LocalUserNum, const FString& Key, const TArray<FString>& UserIds)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to get bulk user record, identity interface is invalid!"));
		TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), FListAccelByteModelsUserRecord());
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), FListAccelByteModelsUserRecord());
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserIdPtr.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), FListAccelByteModelsUserRecord());
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key, UserIds);
	return true;	
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
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to get game record, identity interface is invalid!"));
		TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key, FAccelByteModelsGameRecord());
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key, FAccelByteModelsGameRecord());
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserIdPtr.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key, FAccelByteModelsGameRecord());
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetGameRecord>(AccelByteSubsystem, LocalUserNum, Key, bAlwaysRequestToService);
	return true;
}

bool FOnlineCloudSaveAccelByte::ReplaceGameRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to replace game record, identity interface is invalid!"));
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key);
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key);
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceGameRecord>(AccelByteSubsystem, LocalUserNum, Key, RecordRequest);
	return true;
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
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to get user record, identity interface is invalid!"));
		TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key, FAccelByteModelsUserRecord());
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key, FAccelByteModelsUserRecord());
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserIdPtr.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key, FAccelByteModelsUserRecord());
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserRecord>(AccelByteSubsystem, LocalUserNum, *UserIdPtr.Get(), Key, IsPublic, UserId);
	return true;
}

bool FOnlineCloudSaveAccelByte::ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, bool IsPublic)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to replace user record, identity interface is invalid!"));
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key);
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key);
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserIdPtr.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key);
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceUserRecord>(AccelByteSubsystem, *UserIdPtr.Get(), Key, RecordRequest, IsPublic);
	return true;
}


void FOnlineCloudSaveAccelByte::AddGameRecordToMap(const FString& Key, const TSharedRef<FAccelByteModelsGameRecord>& Record)
{
	FScopeLock ScopeLock(&GameRecordMapLock);
	GameRecordMap.Add(Key, Record);
}

#undef ONLINE_ERROR_NAMESPACE