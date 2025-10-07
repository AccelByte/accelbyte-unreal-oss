// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteSaveUserBinaryRecord.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteGetCurentUserBinaryRecord.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteBulkGetCurentUserBinaryRecords.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteBulkGetPublicUserBinaryRecords.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteBulkQueryPublicUserBinaryRecords.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteBulkQueryCurentUserBinaryRecords.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteGetPublicUserBinaryRecord.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteUpdateUserBinaryRecordFile.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteUpdateUserBinaryRecordMetadata.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteDeleteUserBinaryRecord.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteRequestUserBinaryRecordPresignedUrl.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteGetGameBinaryRecord.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteBulkGetGameBinaryRecords.h"
#include "AsyncTasks/BinaryCloudSave/OnlineAsyncTaskAccelByteBulkQueryGameBinaryRecords.h"
#include "OnlineError.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteDeleteAdminGameRecordTTLConfig.h"
#include "AsyncTasks/CloudSave/OnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineBinaryCloudSaveAccelByte"

bool FOnlineBinaryCloudSaveAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem
	, FOnlineBinaryCloudSaveAccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetBinaryCloudSaveInterface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineBinaryCloudSaveAccelByte::GetFromWorld(const UWorld* World
	, FOnlineBinaryCloudSaveAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineBinaryCloudSaveAccelByte::SaveUserBinaryRecord(int32 LocalUserNum, FString const &Key, FString const &FileType, bool bIsPublic) {
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSaveUserBinaryRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, FileType
		, bIsPublic);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::SaveUserBinaryRecord(int32 LocalUserNum, FString const &Key, EAccelByteFileType FileType, bool bIsPublic) {
	FString FileTypeString = FAccelByteUtilities::GetUEnumValueAsString(FileType);
	return SaveUserBinaryRecord(LocalUserNum, Key, FileTypeString, bIsPublic);
}

bool FOnlineBinaryCloudSaveAccelByte::GetCurrentUserBinaryRecord(int32 LocalUserNum, FString const& Key)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetCurentUserBinaryRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::GetPublicUserBinaryRecord(int32 LocalUserNum, FString const& Key, FString const& User)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetPublicUserBinaryRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, User);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::BulkGetCurrentUserBinaryRecords(int32 LocalUserNum, TArray<FString> const& Keys)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetCurentUserBinaryRecords>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Keys);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::BulkGetPublicUserBinaryRecords(int32 LocalUserNum, const TArray<FString>& Keys, FString const& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetPublicUserBinaryRecords>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Keys
		, UserId);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::BulkGetPublicUserBinaryRecords(int32 LocalUserNum, FString const& Key, const TArray<FString>& UserIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, UserIds);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::BulkQueryCurrentUserBinaryRecords(int32 LocalUserNum, FString const& Query, int32 const& Offset, int32 const& Limit)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkQueryCurentUserBinaryRecords>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Query
		, Offset
		, Limit);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::BulkQueryPublicUserBinaryRecords(int32 LocalUserNum, FString const& UserId, int32 const& Offset, int32 const& Limit)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkQueryPublicUserBinaryRecords>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, UserId
		, Offset
		, Limit);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::UpdateUserBinaryRecordFile(int32 LocalUserNum, FString const& Key, FString const& FileType, FString const& FileLocation)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateUserBinaryRecordFile>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, FileType
		, FileLocation);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::UpdateUserBinaryRecordFile(int32 LocalUserNum, FString const& Key, EAccelByteFileType ContentType, FString const& FileLocation)
{
	FString FileTypeString = FAccelByteUtilities::GetUEnumValueAsString(ContentType);
	return UpdateUserBinaryRecordFile(LocalUserNum, Key, FileTypeString, FileLocation);
}

bool FOnlineBinaryCloudSaveAccelByte::UpdateUserBinaryRecordMetadata(int32 LocalUserNum, FString const& Key, bool bIsPublic)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateUserBinaryRecordMetadata>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, bIsPublic);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::DeleteUserBinaryRecord(int32 LocalUserNum, FString const& Key)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteDeleteUserBinaryRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::RequestUserBinaryRecordPresignedUrl(int32 LocalUserNum, FString const& Key, FString const& FileType)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteRequestUserBinaryRecordPresignedUrl>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key
		, FileType);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::RequestUserBinaryRecordPresignedUrl(int32 LocalUserNum, FString const& Key, EAccelByteFileType FileType)
{
	FString FileTypeString = FAccelByteUtilities::GetUEnumValueAsString(FileType);
	return RequestUserBinaryRecordPresignedUrl(LocalUserNum, Key, FileTypeString);
}

bool FOnlineBinaryCloudSaveAccelByte::GetGameBinaryRecord(int32 LocalUserNum, FString const& Key)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetGameBinaryRecord>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Key);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::BulkGetGameBinaryRecords(int32 LocalUserNum, TArray<FString> const& Keys)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Keys);
	return true;
}

bool FOnlineBinaryCloudSaveAccelByte::BulkQueryGameBinaryRecords(int32 LocalUserNum, FString const& Query, int32 const& Offset, int32 const& Limit)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkQueryGameBinaryRecords>(AccelByteSubsystemPtr.Get()
		, LocalUserNum
		, Query
		, Offset
		, Limit);
	return true;
}

#undef ONLINE_ERROR_NAMESPACE