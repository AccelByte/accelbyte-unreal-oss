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

bool FOnlineCloudSaveAccelByte::ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, const FUniqueNetIdAccelByteUserRef& TargetUserId)
{
	return ReplaceUserRecord(LocalUserNum, Key, RecordRequest, false, TargetUserId);
}

bool FOnlineCloudSaveAccelByte::ReplacePublicUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, const FUniqueNetIdAccelByteUserRef& TargetUserId)
{
	return ReplaceUserRecord(LocalUserNum, Key, RecordRequest, true, TargetUserId);
}

bool FOnlineCloudSaveAccelByte::DeleteUserRecord(int32 LocalUserNum, const FString& Key, const FUniqueNetIdAccelByteUserRef& TargetUserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	if (IsRunningDedicatedServer() && TargetUserId->GetAccelByteId() == ACCELBYTE_INVALID_ID_VALUE)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Target User Id is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), Key);
		return false;
	}
	
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

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key);
		return false;
	}

	if (!IsRunningDedicatedServer() && TargetUserId->GetAccelByteId() != ACCELBYTE_INVALID_ID_VALUE)
	{
		const FUniqueNetIdAccelByteUserPtr CurrentUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId.ToSharedRef());
		if (CurrentUserId->GetAccelByteId() != TargetUserId->GetAccelByteId())
		{
			AB_OSS_INTERFACE_TRACE_END(TEXT("Target User Id should be matched with the current instance of the user"));
			TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), Key);
			return false;
		}
	}
	
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteUserRecord>(AccelByteSubsystem, *LocalUserId, Key, LocalUserNum, TargetUserId->GetAccelByteId());
	return true;
}

bool FOnlineCloudSaveAccelByte::BulkGetPublicUserRecord(int32 LocalUserNum, const FString& Key, const TArray<FString>& UserIds)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	if (IsRunningDedicatedServer())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to bulk get public user record as the current game instance is a dedicated server!"));
		TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::NotImplemented), FListAccelByteModelsUserRecord());
		return false;
	}
	
	FListAccelByteModelsUserRecord TempUserRecords{};
	for (const auto& UserId : UserIds) 
	{
		FAccelByteModelsUserRecord TempUserRecord{};
		TempUserRecord.Key = Key;
		TempUserRecord.UserId = UserId;
		TempUserRecords.Data.Add(TempUserRecord);
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to get bulk user record, identity interface is invalid!"));
		TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), TempUserRecords);
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), TempUserRecords);
		return false;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnBulkGetPublicUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), TempUserRecords);
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord>(AccelByteSubsystem, *LocalUserId, Key, UserIds);
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
	FAccelByteModelsGameRecord TempGameRecord{};
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

	if (!IsRunningDedicatedServer())
	{
		const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (!LocalUserId.IsValid())
		{
			AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key, FAccelByteModelsGameRecord());
			return false;
		}
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetGameRecord>(AccelByteSubsystem, LocalUserNum, Key, bAlwaysRequestToService);
	return true;
}

bool FOnlineCloudSaveAccelByte::ReplaceGameRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest)
{
	// Currently we expecting this method is always called by the server, so we currently set the metadata record to ESetByMetadataRecord::SERVER
	return ReplaceGameRecord(LocalUserNum, Key, ESetByMetadataRecord::SERVER, RecordRequest, FTTLConfig{});
}

bool FOnlineCloudSaveAccelByte::ReplaceGameRecord(int32 LocalUserNum, const FString& Key, ESetByMetadataRecord SetBy,
	const FJsonObject& RecordRequest, const FTTLConfig& TTLConfig)
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

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceGameRecord>(AccelByteSubsystem, LocalUserNum, Key, SetBy, RecordRequest, TTLConfig);
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

	FAccelByteModelsUserRecord TempRecord{};
	TempRecord.Key = Key;
	TempRecord.UserId = UserId;
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Failed to get user record, identity interface is invalid!"));
		TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key, TempRecord);
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key, TempRecord);
		return false;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnGetUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key, TempRecord);
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserRecord>(AccelByteSubsystem, LocalUserNum, *LocalUserId, Key, IsPublic, UserId);
	return true;
}

bool FOnlineCloudSaveAccelByte::ReplaceUserRecord(int32 LocalUserNum, const FString& Key, const FJsonObject& RecordRequest, bool IsPublic, const FUniqueNetIdAccelByteUserRef& TargetUserId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	if (IsRunningDedicatedServer() && TargetUserId->GetAccelByteId() == ACCELBYTE_INVALID_ID_VALUE)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Target User Id is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), Key);
		return false;
	}

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

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key);
		return false;
	}

	if (!IsRunningDedicatedServer() && TargetUserId->GetAccelByteId() != ACCELBYTE_INVALID_ID_VALUE)
	{
		const FUniqueNetIdAccelByteUserPtr CurrentUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId.ToSharedRef());
		if (CurrentUserId->GetAccelByteId() != TargetUserId->GetAccelByteId())
		{
			AB_OSS_INTERFACE_TRACE_END(TEXT("Target User Id should be matched with the current instance of the user"));
			TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), Key);
			return false;
		}
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceUserRecord>(AccelByteSubsystem, *LocalUserId, Key, RecordRequest, IsPublic, LocalUserNum, TargetUserId->GetAccelByteId());
	return true;
}


void FOnlineCloudSaveAccelByte::AddGameRecordToMap(const FString& Key, const TSharedRef<FAccelByteModelsGameRecord>& Record)
{
	FScopeLock ScopeLock(&GameRecordMapLock);
	GameRecordMap.Add(Key, Record);
}

#undef ONLINE_ERROR_NAMESPACE