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
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteBulkReplaceUserRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteBulkGetUserRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteGetAdminGameRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteCreateAdminGameRecord.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteReplaceAdminGameRecord.h"
#include "OnlineError.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineCloudSaveAccelByte"

bool FOnlineCloudSaveAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem
	, FOnlineCloudSaveAccelBytePtr& OutInterfaceInstance)
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

bool FOnlineCloudSaveAccelByte::GetFromWorld(const UWorld* World
	, FOnlineCloudSaveAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

#pragma region GetUserRecord
bool FOnlineCloudSaveAccelByte::GetUserRecord(int32 LocalUserNum
	, FString const& Key)
{
	return GetUserRecord(LocalUserNum, Key, false);
}

bool FOnlineCloudSaveAccelByte::GetUserRecord(int32 LocalUserNum
	, FString const& Key
	, FUniqueNetIdAccelByteUserRef const& RecordUserId)
{
	const FString UserId = RecordUserId->GetAccelByteId();
	return GetUserRecord(LocalUserNum, Key, false, UserId);
}

bool FOnlineCloudSaveAccelByte::GetPublicUserRecord(int32 LocalUserNum
	, FString const& Key)
{
	return GetUserRecord(LocalUserNum, Key, true);
}

bool FOnlineCloudSaveAccelByte::GetPublicUserRecord(int32 LocalUserNum
	, FString const& Key
	, FUniqueNetIdAccelByteUserRef const& RecordUserId)
{
	const FString UserId = RecordUserId->GetAccelByteId();
	return GetUserRecord(LocalUserNum, Key, true, UserId);
}

bool FOnlineCloudSaveAccelByte::GetUserRecord(int32 LocalUserNum
	, FString const& Key
	, bool IsPublic
	, FString const& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	FAccelByteModelsUserRecord TempRecord{};
	TempRecord.Key = Key;
	TempRecord.UserId = UserId;

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, IsPublic
		, UserId);
	return true;
}

bool FOnlineCloudSaveAccelByte::BulkGetUserRecord(FString const& Key
	, TArray<FUniqueNetIdAccelByteUserRef> const& UniqueNetIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get user records, key:%s, total user:%d"), *Key, UniqueNetIds.Num());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetUserRecord>(AccelByteSubsystemPtr.Get()
		, Key
		, UniqueNetIds);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineCloudSaveAccelByte::BulkGetPublicUserRecord(int32 LocalUserNum
	, FString const& Key\
	, TArray<FString> const& UserIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
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

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, UserIds);
	return true;
}

bool FOnlineCloudSaveAccelByte::BulkGetPublicUserRecord(int32 LocalUserNum
	, FString const& Key
	, TArray<FUniqueNetIdAccelByteUserRef> const& UniqueNetIds)
{
	TArray<FString> UserIds{};
	for (const auto& UniqueNetId : UniqueNetIds)
	{
		UserIds.Add(UniqueNetId->GetAccelByteId());
	}
	return BulkGetPublicUserRecord(LocalUserNum, Key, UserIds);
}
#pragma endregion

#pragma region ReplaceUserRecord
bool FOnlineCloudSaveAccelByte::ReplaceUserRecord(int32 LocalUserNum
	, FString const& Key
	, FJsonObject const& RecordRequest
	, FUniqueNetIdAccelByteUserRef const& TargetUserId)
{
	return ReplaceUserRecord(LocalUserNum, Key, RecordRequest, false, TargetUserId);
}

bool FOnlineCloudSaveAccelByte::ReplacePublicUserRecord(int32 LocalUserNum
	, FString const& Key
	, FJsonObject const& RecordRequest
	, FUniqueNetIdAccelByteUserRef const& TargetUserId)
{
	return ReplaceUserRecord(LocalUserNum, Key, RecordRequest, true, TargetUserId);
}

bool FOnlineCloudSaveAccelByte::BulkReplaceUserRecord(FString const& Key
	, TMap<FUniqueNetIdAccelByteUserRef, FJsonObject> const& Request)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Replacing user records, key:%s, total record:%d"), *Key, Request.Num());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkReplaceUserRecord>(AccelByteSubsystemPtr.Get()
		, Key
		, Request);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}
#pragma endregion

bool FOnlineCloudSaveAccelByte::DeleteUserRecord(int32 LocalUserNum
	, FString const& Key
	, FUniqueNetIdAccelByteUserRef const& TargetUserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		TriggerOnDeleteUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::Unknown), Key);
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteUserRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, TargetUserId->GetAccelByteId());
	return true;
}

bool FOnlineCloudSaveAccelByte::GetGameRecord(int32 LocalUserNum, FString const& Key, bool bAlwaysRequestToService)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	FAccelByteModelsGameRecord TempGameRecord{};
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to get game record, identity interface is invalid!"));
		TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key, FAccelByteModelsGameRecord());
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key, FAccelByteModelsGameRecord());
		return false;
	}

	if (!IsRunningDedicatedServer())
	{
		const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (!LocalUserId.IsValid())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
			TriggerOnGetGameRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key, FAccelByteModelsGameRecord());
			return false;
		}
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetGameRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, bAlwaysRequestToService);
	return true;
}

bool FOnlineCloudSaveAccelByte::ReplaceGameRecord(int32 LocalUserNum, FString const& Key, const FJsonObject& RecordRequest)
{
	// Currently we expecting this method is always called by the server, so we currently set the metadata record to ESetByMetadataRecord::SERVER
	return ReplaceGameRecord(LocalUserNum, Key, ESetByMetadataRecord::SERVER, RecordRequest, FTTLConfig{});
}

bool FOnlineCloudSaveAccelByte::ReplaceGameRecord(int32 LocalUserNum, FString const& Key, ESetByMetadataRecord SetBy,
	const FJsonObject& RecordRequest, const FTTLConfig& TTLConfig)
{	
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceGameRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, SetBy
		, RecordRequest
		, TTLConfig);
	return true;
}

bool FOnlineCloudSaveAccelByte::DeleteGameRecordTTLConfig(int32 LocalUserNum, FString const& Key)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key);
	return true;
}

bool FOnlineCloudSaveAccelByte::GetAdminGameRecord(int32 LocalUserNum, FString const& Key, bool bAlwaysRequestToService)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetAdminGameRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, bAlwaysRequestToService);
	return true;
}

bool FOnlineCloudSaveAccelByte::GetAdminGameRecordFromCache(FString const& Key, FAccelByteModelsAdminGameRecord& OutRecord)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Admin Game Record, Key: %s"), *Key);
	FScopeLock ScopeLock(&AdminGameRecordMapLock);

	if (AdminGameRecordMap.Num() > 0)
	{
		auto Record = AdminGameRecordMap.Find(Key);
		if (Record != nullptr)
		{
			OutRecord = Record->Get();
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Admin Game Record found for key %s"), *Key);
			return true;
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Admin Game Record not found for key %s"), *Key);
	return false;
}

bool FOnlineCloudSaveAccelByte::CreateAdminGameRecord(int32 LocalUserNum, FString const& Key, const FJsonObject& RecordRequest, const TArray<FString>& Tags, const FTTLConfig& TTLConfig)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateAdminGameRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, RecordRequest
		, Tags
		, TTLConfig);
	return true;
}

bool FOnlineCloudSaveAccelByte::GetGameRecordFromCache(FString const& Key, FAccelByteModelsGameRecord& OutRecord)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Get Game Record, Key: %s"), *Key);
	FScopeLock ScopeLock(&GameRecordMapLock);

	if (GameRecordMap.Num() > 0)
	{
		auto Record = GameRecordMap.Find(Key);
		if (Record != nullptr)
		{
			OutRecord = Record->Get();
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Game Record found for key %s"), *Key);
			return true;
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Game Record not found for key %s"), *Key);
	return false;
}

bool FOnlineCloudSaveAccelByte::ReplaceUserRecord(int32 LocalUserNum, FString const& Key, const FJsonObject& RecordRequest, bool IsPublic, const FUniqueNetIdAccelByteUserRef& TargetUserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	
	if (IsRunningDedicatedServer() && TargetUserId->GetAccelByteId() == ACCELBYTE_INVALID_ID_VALUE)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Target User Id is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), Key);
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to replace user record, identity interface is invalid!"));
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::MissingInterface), Key);
		return false;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidAuth), Key);
		return false;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
		TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidUser), Key);
		return false;
	}

	if (!IsRunningDedicatedServer() && TargetUserId->GetAccelByteId() != ACCELBYTE_INVALID_ID_VALUE)
	{
		const FUniqueNetIdAccelByteUserPtr CurrentUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId.ToSharedRef());
		if (CurrentUserId->GetAccelByteId() != TargetUserId->GetAccelByteId())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Target User Id should be matched with the current instance of the user"));
			TriggerOnReplaceUserRecordCompletedDelegates(LocalUserNum, ONLINE_ERROR(EOnlineErrorResult::InvalidParams), Key);
			return false;
		}
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceUserRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, RecordRequest
		, IsPublic
		, TargetUserId->GetAccelByteId());
	return true;
}


bool FOnlineCloudSaveAccelByte::ReplaceAdminGameRecord(int32 LocalUserNum, FString const& Key, const FJsonObject& RecordRequest, const FTTLConfig& TTLConfig)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteReplaceAdminGameRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, RecordRequest
		, TTLConfig);
	return true;
}

bool FOnlineCloudSaveAccelByte::DeleteAdminGameRecordTTLConfig(int32 LocalUserNum, FString const& Key)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key);
	return true;
}

void FOnlineCloudSaveAccelByte::AddGameRecordToMap(FString const& Key, const TSharedRef<FAccelByteModelsGameRecord>& Record)
{
	FScopeLock ScopeLock(&GameRecordMapLock);
	GameRecordMap.Add(Key, Record);
}

void FOnlineCloudSaveAccelByte::AddAdminGameRecordToMap(FString const& Key, const TSharedRef<FAccelByteModelsAdminGameRecord>& Record)
{
	FScopeLock ScopeLock(&AdminGameRecordMapLock);
	AdminGameRecordMap.Add(Key, Record);
}

#undef ONLINE_ERROR_NAMESPACE