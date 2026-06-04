// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Online.h"
#include "Core/AccelByteError.h"
#include "Api/AccelByteUserProfileApi.h"
#include "ExecTests/ExecTestQueryExternalIds.h"
#include "ExecTests/ExecTestQueryUserIdMapping.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserInfo.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserIdMapping.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryExternalIdMappings.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserProfile.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteCreateUserProfile.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteUpdateUserProfile.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteListUserByUserId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteLinkOtherPlatform.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteUnlinkOtherPlatform.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteLinkOtherPlatformId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteUnlinkOtherPlatformId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteForcePlatformLinkV3.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteCheckUserAccountAvailability.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserIdMappingWithPlatform.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserIdMappingWithPlatformId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetUserPlatformLinks.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteValidateUserInput.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetInputValidations.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserIdsMapping.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetMyUserProfile.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetPublicUserProfileInfo.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetCustomAttributes.h"
// #include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetPublicCustomAttributes.h" // DEPRECATED - REMOVED
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteUpdateCustomAttributes.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetPrivateCustomAttributes.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGenerateUploadURL.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGenerateUploadURLForUserContent.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetUserProfile.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2.h"
#include "OnlineSubsystemAccelByteLog.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

FOnlineUserAccelByte::FOnlineUserAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{}

bool FOnlineUserAccelByte::GetFromWorld(const UWorld* World, FOnlineUserAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineUserAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineUserAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineUserAccelByte::GetFromSubsystem(const FOnlineSubsystemAccelByte* Subsystem, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	if (Subsystem == nullptr)
	{
		return false;
	}

	OutInterfaceInstance = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineUserAccelByte::CreateUserProfile(const FUniqueNetId& UserId)
{
	if (!UserId.IsValid())
	{
		return false;
	}
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCreateUserProfile>(AccelByteSubsystemPtr.Get(), UserId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get or create user profile!"));
	return true;
}

bool FOnlineUserAccelByte::UpdateUserProfile(int32 LocalUserNum, const FUniqueNetId& UserId, const FAccelByteModelsUserProfileUpdateRequest& UpdateRequest)
{
	if (!UserId.IsValid())
	{
		return false;
	}
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserId: %s"), LocalUserNum, *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateUserProfile>(AccelByteSubsystemPtr.Get(), LocalUserNum, UserId, UpdateRequest);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to update user profile!"));
	return true;
}

bool FOnlineUserAccelByte::QueryUserProfile(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserId Amount: %d"), LocalUserNum, UserIds.Num());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum, UserIds]()
			{
				UserInterface->TriggerOnQueryUserProfileCompleteDelegates(LocalUserNum, false, UserIds, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("query-user-profile-local-user-index-out-of-range")));
			});
		return false;
	}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserProfile>(AccelByteSubsystemPtr.Get(), LocalUserNum, UserIds, OnQueryUserProfileCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to query user information for %d IDs!"), UserIds.Num());
	return true;
}

bool FOnlineUserAccelByte::GetMyUserProfile(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetMyUserProfileCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileInfo{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("get-my-user-profile-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetMyUserProfile>(AccelByteSubsystemPtr.Get(), LocalUserNum, OnGetMyUserProfileCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get own user profile!"));
	return true;
}

bool FOnlineUserAccelByte::GetPublicUserProfileByPublicId(int32 LocalUserNum, const FString& PublicId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; PublicId: %s"), LocalUserNum, *PublicId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (PublicId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("PublicId is empty!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetPublicUserProfileByPublicIdCompleteDelegates(LocalUserNum, false, FAccelByteModelsPublicUserProfileInfo{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("get-public-user-profile-by-public-id-empty")));
			});
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetPublicUserProfileByPublicIdCompleteDelegates(LocalUserNum, false, FAccelByteModelsPublicUserProfileInfo{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("get-public-user-profile-by-public-id-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId>(AccelByteSubsystemPtr.Get(), LocalUserNum, PublicId, OnGetPublicUserProfileByPublicIdCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get public user profile by PublicId!"));
	return true;
}

bool FOnlineUserAccelByte::GetPublicUserProfileInfo(int32 LocalUserNum, const FString& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserId: %s"), LocalUserNum, *UserId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (UserId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("UserId is empty!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetPublicUserProfileInfoCompleteDelegates(LocalUserNum, false, FAccelByteModelsPublicUserProfileInfo{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("get-public-user-profile-info-empty")));
			});
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetPublicUserProfileInfoCompleteDelegates(LocalUserNum, false, FAccelByteModelsPublicUserProfileInfo{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("get-public-user-profile-info-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo>(AccelByteSubsystemPtr.Get(), LocalUserNum, UserId, OnGetPublicUserProfileInfoCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get public user profile info!"));
	return true;
}

bool FOnlineUserAccelByte::GetCustomAttributes(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetCustomAttributesCompleteDelegates(LocalUserNum, false, FJsonObjectWrapper{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("get-custom-attributes-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetCustomAttributes>(AccelByteSubsystemPtr.Get(), LocalUserNum, OnGetCustomAttributesCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get custom attributes!"));
	return true;
}

/*
 * [DEPRECATED] GetPublicCustomAttributes has been removed due to security issues.
 * Please use 'GetPublicUserProfileInfo(UserId)' instead, which includes
 * CustomAttributes in the returned FAccelByteModelsPublicUserProfileInfo.
 *
 * @deprecated SDK function deprecated with error code 14901
 */

bool FOnlineUserAccelByte::UpdateCustomAttributes(int32 LocalUserNum, const FJsonObject& CustomAttributes)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnUpdateCustomAttributesCompleteDelegates(LocalUserNum, false, FJsonObjectWrapper{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("update-custom-attributes-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdateCustomAttributes>(AccelByteSubsystemPtr.Get(), LocalUserNum, MakeShared<FJsonObject>(CustomAttributes), OnUpdateCustomAttributesCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to update custom attributes!"));
	return true;
}

bool FOnlineUserAccelByte::GetPrivateCustomAttributes(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetPrivateCustomAttributesCompleteDelegates(LocalUserNum, false, FJsonObjectWrapper{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("get-private-custom-attributes-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetPrivateCustomAttributes>(AccelByteSubsystemPtr.Get(), LocalUserNum, OnGetPrivateCustomAttributesCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get private custom attributes!"));
	return true;
}

bool FOnlineUserAccelByte::UpdatePrivateCustomAttributes(int32 LocalUserNum, const FJsonObject& PrivateAttributes)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnUpdatePrivateCustomAttributesCompleteDelegates(LocalUserNum, false, FJsonObjectWrapper{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("update-private-custom-attributes-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes>(AccelByteSubsystemPtr.Get(), LocalUserNum, MakeShared<FJsonObject>(PrivateAttributes), OnUpdatePrivateCustomAttributesCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to update private custom attributes!"));
	return true;
}

bool FOnlineUserAccelByte::GenerateUploadURL(int32 LocalUserNum, const FString& Folder, EAccelByteFileType FileType)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; Folder: %s; FileType: %d"), LocalUserNum, *Folder, static_cast<int32>(FileType));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (Folder.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Folder is empty!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGenerateUploadURLCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("generate-upload-url-folder-empty")));
			});
		return false;
	}

	if (FileType == EAccelByteFileType::NONE)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("FileType is NONE!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGenerateUploadURLCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("generate-upload-url-file-type-none")));
			});
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGenerateUploadURLCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("generate-upload-url-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateUploadURL>(AccelByteSubsystemPtr.Get(), LocalUserNum, Folder, FileType, OnGenerateUploadURLCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to generate upload URL!"));
	return true;
}

bool FOnlineUserAccelByte::GenerateUploadURLForUserContent(int32 LocalUserNum, const FString& UserId, EAccelByteFileType FileType, EAccelByteUploadCategory Category)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserId: %s; FileType: %d; Category: %d"), LocalUserNum, *UserId, static_cast<int32>(FileType), static_cast<int32>(Category));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (UserId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("UserId is empty!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGenerateUploadURLForUserContentCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("generate-upload-url-for-user-content-userid-empty")));
			});
		return false;
	}

	if (FileType == EAccelByteFileType::NONE)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("FileType is NONE!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGenerateUploadURLForUserContentCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("generate-upload-url-for-user-content-file-type-none")));
			});
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGenerateUploadURLForUserContentCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("generate-upload-url-for-user-content-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent>(AccelByteSubsystemPtr.Get(), LocalUserNum, UserId, FileType, Category, OnGenerateUploadURLForUserContentCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to generate upload URL for user content!"));
	return true;
}

bool FOnlineUserAccelByte::GetUserPlatformLinks(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
if (!AccelByteSubsystemPtr.IsValid())
{
    AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
    return false;
}
	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetUserPlatformLinksCompleteDelegates(LocalUserNum, false, TArray<FPlatformLink>{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("get-user-3rd-party-platform-information-index-out-of-range")));
			});
		return false;
	}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserPlatformLinks>(AccelByteSubsystemPtr.Get(), LocalUserNum);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Get user 3rd party platform information for LocalUserNum: %d"), LocalUserNum);
	return true;
}

bool FOnlineUserAccelByte::QueryUserInfo(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserId Amount: %d"), LocalUserNum, UserIds.Num());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
if (!AccelByteSubsystemPtr.IsValid())
{
    AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
    return false;
}
	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum, UserIds]() {
			UserInterface->TriggerOnQueryUserInfoCompleteDelegates(LocalUserNum, false, UserIds, TEXT("query-user-local-user-index-out-of-range"));
		});
		return false;
	}

	FOnlineAsyncTaskInfo TaskInfo;
	TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
	TaskInfo.bCreateEpicForThis = true;
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTask<FOnlineAsyncTaskAccelByteQueryUserInfo>(TaskInfo, AccelByteSubsystemPtr.Get(), LocalUserNum, UserIds, OnQueryUserInfoCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to query user information for %d IDs!"), UserIds.Num());
	return true;
}

bool FOnlineUserAccelByte::GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<FOnlineUser>>& OutUsers)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	OutUsers.Empty(IDToUserInfoMap.Num());
	for (const TPair<TSharedRef<const FUniqueNetId>, TSharedRef<FUserOnlineAccountAccelByte>>& KeyValue : IDToUserInfoMap)
	{
		OutUsers.Add(KeyValue.Value);
	}

	// supposed to return true if user data was found
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Returning array with %d users"), OutUsers.Num());
	return OutUsers.Num() > 0;
}

TSharedPtr<FOnlineUser> FOnlineUserAccelByte::GetUserInfo(int32 LocalUserNum, const FUniqueNetId& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserId: %s"), LocalUserNum, *UserId.ToDebugString());

	const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteID = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const TSharedRef<FUserOnlineAccountAccelByte>* UserInfo = IDToUserInfoMap.Find(AccelByteID);
	if (UserInfo != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found user info for user with ID of '%s'"), *UserId.ToDebugString());
		return (*UserInfo);
	}

	// As a fallback, attempt to find a matching user in the cache with the same AccelByte Id
	for (const TPair<TSharedRef<const FUniqueNetId>, TSharedRef<FUserOnlineAccountAccelByte>>& KeyValue : IDToUserInfoMap)
	{
		const TSharedRef<const FUniqueNetIdAccelByteUser> UserAccelByteID = FUniqueNetIdAccelByteUser::CastChecked(KeyValue.Key);
		if (UserAccelByteID->GetAccelByteId() == AccelByteID->GetAccelByteId())
		{
			AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found user info by comparing AccelByteId for user with ID of '%s'"), *UserId.ToDebugString());
			return KeyValue.Value;
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to find user info for user with ID of '%s'"), *UserId.ToDebugString());
	return nullptr;
}

bool FOnlineUserAccelByte::QueryUserIdMapping(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FOnQueryUserMappingComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; Display Name or Email to Query: %s"), *UserId.ToDebugString(), *DisplayNameOrEmail);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
if (!AccelByteSubsystemPtr.IsValid())
{
    AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
    return false;
}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserIdMapping>(AccelByteSubsystemPtr.Get(), UserId, DisplayNameOrEmail, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to query user ID for display name or email '%s'!"), *DisplayNameOrEmail);
	return true;
}

bool FOnlineUserAccelByte::QueryUserIdMappingWithPlatform(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, EAccelBytePlatformType PlatformType, const FOnQueryUserMappingComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; Display Name or Email to Query: %s"), *UserId.ToDebugString(), *DisplayNameOrEmail);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
if (!AccelByteSubsystemPtr.IsValid())
{
    AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
    return false;
}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserIdMappingWithPlatform>(AccelByteSubsystemPtr.Get(), UserId, DisplayNameOrEmail, PlatformType, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to query user ID for display name or email '%s'!"), *DisplayNameOrEmail);
	return true;
}

bool FOnlineUserAccelByte::QueryUserIdMappingWithPlatformId(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FString& PlatformId, const FOnQueryUserMappingComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; Display Name or Email to Query: %s"), *UserId.ToDebugString(), *DisplayNameOrEmail);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
if (!AccelByteSubsystemPtr.IsValid())
{
    AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
    return false;
}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserIdMappingWithPlatformId>(AccelByteSubsystemPtr.Get(), UserId, DisplayNameOrEmail, PlatformId, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to query user ID for display name or email '%s'!"), *DisplayNameOrEmail);
	return true;
}

bool FOnlineUserAccelByte::QueryExternalIdMappings(const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const FOnQueryExternalIdMappingsComplete& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; Platform Type: %s, External Ids Amount: %d"), *UserId.ToDebugString(), *QueryOptions.AuthType, ExternalIds.Num());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
if (!AccelByteSubsystemPtr.IsValid())
{
    AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
    return false;
}

	// Don't see a use case where we would need this, return that it isn't supported
	if (QueryOptions.bLookupByDisplayName)
	{
		const FString ErrorStr = TEXT("AccelByte OSS does not support calling this method with FExternalIdQueryOptions::bLookupByDisplayName set to true. Contact your account manager if you have a use case for this.");

		// Need to cast UserId to be an AccelByte ID for the delegate to copy it
		AccelByteSubsystemPtr->ExecuteNextTick([Delegate, AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UserId), QueryOptions, ErrorStr]() {
			Delegate.ExecuteIfBound(false, AccelByteId.Get(), QueryOptions, TArray<FString>(), ErrorStr);
		});

		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("%s"), *ErrorStr);
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryExternalIdMappings>(AccelByteSubsystemPtr.Get(), UserId, QueryOptions, ExternalIds, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get %d external ID mappings!"), ExternalIds.Num());
	return true;
}

void FOnlineUserAccelByte::GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<FUniqueNetIdPtr>& OutIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("External ID Count: %d"), ExternalIds.Num());

	// Don't see a use case where we would need this, return that it isn't supported
	if (QueryOptions.bLookupByDisplayName)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("AccelByte OSS does not support calling this method with FExternalIdQueryOptions::bLookupByDisplayName set to true. Contact your account manager if you have a use case for this."));
		return;
	}

	int32 ValidResults = 0;
	for (const FString& ExternalId : ExternalIds)
	{
		// NOTE(Maxwell, 5/24/2021): May seem odd that we want to explicitly return nullptr if we haven't found a result,
		// however this is something that the base interface method calls for, I assume so that you can check if an ID you
		// need to query isn't valid
		const TSharedRef<const FUniqueNetId>* Result = ExternalIDToAccelByteIDMap.Find(ExternalId);
		if (Result != nullptr)
		{
			OutIds.Add(*Result);
			ValidResults++;
		}
		else
		{
			OutIds.Add(nullptr);
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Received %d valid external ID mappings!"), ValidResults);
}

FUniqueNetIdPtr FOnlineUserAccelByte::GetExternalIdMapping(const FExternalIdQueryOptions& QueryOptions, const FString& ExternalId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("External ID: %s"), *ExternalId);

	// Don't see a use case where we would need this, return that it isn't supported
	if (QueryOptions.bLookupByDisplayName)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Error, TEXT("AccelByte OSS does not support calling this method with FExternalIdQueryOptions::bLookupByDisplayName set to true. Contact your account manager if you have a use case for this."));
		return nullptr;
	}

	const TSharedRef<const FUniqueNetId>* Result = ExternalIDToAccelByteIDMap.Find(ExternalId);
	if (Result != nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Found AccelByte ID '%s' for external ID '%s' in cache!"), *(*Result)->ToDebugString(), *ExternalId);
		return *Result;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("AccelByte ID not found in cache for external ID of '%s'"), *ExternalId);
	return nullptr;
}

void FOnlineUserAccelByte::PostLoginBulkGetUserProfileCompleted(int32 LocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FOnlineError& ErrorStr)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
	    AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
	    return;
	}

	const auto UserId = AccelByteSubsystemPtr->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
	if (UserIds.Num() == 0 && UserId.IsValid())
	{
		AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(LocalUserNum);
		if (ApiClient.IsValid() && ApiClient->CredentialsRef->IsComply())
		{
			CreateUserProfile(*UserId.Get());
		}
	}

	ClearOnQueryUserProfileCompleteDelegates(LocalUserNum, this);
}

#if WITH_DEV_AUTOMATION_TESTS
bool FOnlineUserAccelByte::TestExec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	bool bWasHandled = false;

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (FParse::Command(&Cmd, TEXT("EXTERNAL")))
	{
		// Full command to test external ID query is ONLINE TEST USER EXTERNAL <AuthType> <Space-separated external IDs>
		const FString AuthType = FParse::Token(Cmd, false);

		// Parse each external ID from the console command line one by one, until we get an empty string as a result, meaning
		// that we have hit the end of the input for the external IDs
		TArray<FString> ExternalIds;
		FString Id;
		while (!(Id = FParse::Token(Cmd, false)).IsEmpty())
		{
			ExternalIds.Add(Id);
		}

		TSharedPtr<FExecTestQueryExternalIds> ExternalIdTest = MakeShared<FExecTestQueryExternalIds>(InWorld, ACCELBYTE_SUBSYSTEM, AuthType, ExternalIds);
		ExternalIdTest->Run();

		AccelByteSubsystemPtr->AddExecTest(ExternalIdTest);
		bWasHandled = true;
	}
	else if (FParse::Command(&Cmd, TEXT("MAP")))
	{
		// Full command to test querying a user ID mapping is ONLINE TEST USER MAP <LocalUserNum> <DisplayNameOrEmail>
		const FString DisplayNameOrEmail = FParse::Token(Cmd, false);

		TSharedPtr<FExecTestQueryUserIdMapping> QueryUserIdMappingTest = MakeShared<FExecTestQueryUserIdMapping>(InWorld, ACCELBYTE_SUBSYSTEM, DisplayNameOrEmail);
		QueryUserIdMappingTest->Run();

		AccelByteSubsystemPtr->AddExecTest(QueryUserIdMappingTest);
		bWasHandled = true;
	}

	return bWasHandled;
}
#endif

void FOnlineUserAccelByte::ListUserByUserId(const int32 LocalUserNum, const TArray<FString>& UserIds)
{
	UE_LOG_AB(Display, TEXT("FOnlineUserAccelByte::ListUserByUserId"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteListUserByUserId>
		(AccelByteSubsystemPtr.Get(), LocalUserNum, UserIds);
}


void FOnlineUserAccelByte::LinkOtherPlatform(const FUniqueNetId& UserId, EAccelBytePlatformType PlatformType, const FString& Ticket)
{
	UE_LOG_AB(Display, TEXT("FOnlineIdentityAccelByte::LinkOtherPlatform"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLinkOtherPlatform>
		(AccelByteSubsystemPtr.Get(), UserId, PlatformType, Ticket);
}

void FOnlineUserAccelByte::UnlinkOtherPlatform(const FUniqueNetId& UserId, EAccelBytePlatformType PlatformType)
{
	UE_LOG_AB(Display, TEXT("FOnlineIdentityAccelByte::UnlinkOtherPlatform"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
		
	}
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUnlinkOtherPlatform>
		(AccelByteSubsystemPtr.Get(), UserId, PlatformType);
}

void FOnlineUserAccelByte::LinkOtherPlatformId(const FUniqueNetId& UserId, const FString& PlatformId, const FString& Ticket)
{
	UE_LOG_AB(Display, TEXT("FOnlineIdentityAccelByte::LinkOtherPlatformId"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLinkOtherPlatformId>
		(AccelByteSubsystemPtr.Get(), UserId, PlatformId, Ticket);
}

void FOnlineUserAccelByte::UnlinkOtherPlatformId(const FUniqueNetId& UserId, const FString& PlatformId)
{
	UE_LOG_AB(Display, TEXT("FOnlineIdentityAccelByte::UnlinkOtherPlatformId"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId>
		(AccelByteSubsystemPtr.Get(), UserId, PlatformId);
}

void FOnlineUserAccelByte::ForcePlatformLinkV3(const FUniqueNetId& UserId, const FString& PlatformId, const FString& Ticket)
{
	UE_LOG_AB(Display, TEXT("FOnlineUserAccelByte::ForcePlatformLinkV3"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteForcePlatformLinkV3>
		(AccelByteSubsystemPtr.Get(), UserId, PlatformId, Ticket);
}

void FOnlineUserAccelByte::CheckUserAccountAvailability(const FUniqueNetId& UserId, const FString& DisplayName, bool bIsSearchUniqueDisplayName)
{
	UE_LOG_AB(Display, TEXT("FOnlineIdentityAccelByte::CheckUserAccountAvailability"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteCheckUserAccountAvailability>
		(AccelByteSubsystemPtr.Get(), UserId, DisplayName, bIsSearchUniqueDisplayName);
}

void FOnlineUserAccelByte::AddNewLinkedUserAccountToCache(const TSharedRef<const FUniqueNetId>& UserId, const TArray<FPlatformLink>& LinkedAccounts)
{
	TArray<FAccelByteUserPlatformLinkInformationRef> TempLinkedPlatformAccounts = {};

	for (const auto& PlatformInfo : LinkedAccounts)
	{

#if UE_BUILD_SHIPPING
		if (PlatformInfo.PlatformId.Equals("device", ESearchCase::IgnoreCase) && PlatformInfo.PlatformId.Equals("justice", ESearchCase::IgnoreCase))
		{
			continue;
		}
#endif

		TempLinkedPlatformAccounts.Add(MakeShareable(new FAccelByteUserPlatformLinkInformation(PlatformInfo)));
	}

	NetIdToLinkedOnlineAccountMap.Add(UserId, TempLinkedPlatformAccounts);
}

bool FOnlineUserAccelByte::RemoveLinkedUserAccountFromCache(const TSharedRef<const FUniqueNetId>& UserId)
{
	bool bResult = false; 

	if (NetIdToLinkedOnlineAccountMap.Remove(UserId))
	{
		bResult = true;
	}

	return bResult;
}

void FOnlineUserAccelByte::GetLinkedUserAccountFromCache(const TSharedRef<const FUniqueNetId>& UserId, TArray<FAccelByteUserPlatformLinkInformationRef>& OutLinkedAccounts)
{
	TArray<FAccelByteUserPlatformLinkInformationRef> TempLinkedPlatformAccounts = {};

	if (const TArray<FAccelByteUserPlatformLinkInformationRef>* FoundAccounts = NetIdToLinkedOnlineAccountMap.Find(UserId))
	{
		TempLinkedPlatformAccounts = *FoundAccounts;
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("The given user id doesn't have their linked account cached"));

		return;
	}

	OutLinkedAccounts = TempLinkedPlatformAccounts;
}

void FOnlineUserAccelByte::ValidateUserInput(int32 LocalUserNum, const FUserInputValidationRequest& UserInputValidationRequest)
{
	UE_LOG_AB(Display, TEXT("FOnlineUserAccelByte::ValidateUserInput"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteValidateUserInput>
		(AccelByteSubsystemPtr.Get(), LocalUserNum, UserInputValidationRequest);
}

void FOnlineUserAccelByte::GetInputValidations(int32 LocalUserNum, const FString& LanguageCode, bool bDefaultOnEmpty)
{
	UE_LOG_AB(Display, TEXT("FOnlineUserAccelByte::GetInputValidations"));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetInputValidations>
		(AccelByteSubsystemPtr.Get(), LocalUserNum, LanguageCode, bDefaultOnEmpty);
}

bool FOnlineUserAccelByte::QueryUserIdsMapping(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FOnQueryUserIdsMappingComplete& Delegate, int32 Offset, int32 Limit)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; Display Name or Email to Query: %s"), *UserId.ToDebugString(), *DisplayNameOrEmail);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserIdsMapping>(AccelByteSubsystemPtr.Get(), UserId, DisplayNameOrEmail, Delegate, Offset, Limit);

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Created and dispatched async task to query user ID for display name or email '%s'!"), *DisplayNameOrEmail);
	return true;
}

bool FOnlineUserAccelByte::BulkGetPublicUserProfileInfosV2(int32 LocalUserNum, const TArray<FString>& UserIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserIds count: %d"), LocalUserNum, UserIds.Num());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (UserIds.Num() == 0)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("UserIds array is empty!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnBulkGetPublicUserProfileInfosV2CompleteDelegates(LocalUserNum, false, FAccelByteModelsPublicUserProfileInfoV2{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("bulk-get-public-user-profile-infos-v2-empty-userids")));
			});
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnBulkGetPublicUserProfileInfosV2CompleteDelegates(LocalUserNum, false, FAccelByteModelsPublicUserProfileInfoV2{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("bulk-get-public-user-profile-infos-v2-local-user-index-out-of-range")));
			});
		return false;
	}

	// Use dedicated async task for BulkGetPublicUserProfileInfosV2
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2>(AccelByteSubsystemPtr.Get(), LocalUserNum, UserIds, OnBulkGetPublicUserProfileInfosV2CompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to bulk get public user profile infos V2!"));
	return true;
}

bool FOnlineUserAccelByte::CreateUserProfile(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, identity interface invalid"));
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, user ID not valid for LocalUserNum %d"), LocalUserNum);
		return false;
	}

	// Delegate to existing CreateUserProfile(const FUniqueNetId& UserId) implementation
	return CreateUserProfile(*UserId);
}

bool FOnlineUserAccelByte::UpdateUserProfile(int32 LocalUserNum, const FAccelByteModelsUserProfileUpdateRequest& UpdateRequest)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, identity interface invalid"));
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, user ID not valid for LocalUserNum %d"), LocalUserNum);
		return false;
	}

	// Delegate to existing UpdateUserProfile(int32 LocalUserNum, const FUniqueNetId& UserId, const FAccelByteModelsUserProfileUpdateRequest& UpdateRequest) implementation
	return UpdateUserProfile(LocalUserNum, *UserId, UpdateRequest);
}

bool FOnlineUserAccelByte::GetUserProfile(int32 LocalUserNum, const FString& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d; UserId: %s"), LocalUserNum, *UserId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}

	if (UserId.IsEmpty())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("UserId is empty!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetUserProfileCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileInfo{}, ONLINE_ERROR(EOnlineErrorResult::InvalidParams, TEXT("get-user-profile-empty-userid")));
			});
		return false;
	}

	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("LocalUserNum passed was out of range!"));
		AccelByteSubsystemPtr->ExecuteNextTick([UserInterface = SharedThis(this), LocalUserNum]()
			{
				UserInterface->TriggerOnGetUserProfileCompleteDelegates(LocalUserNum, false, FAccelByteModelsUserProfileInfo{}, ONLINE_ERROR(EOnlineErrorResult::InvalidUser, TEXT("get-user-profile-local-user-index-out-of-range")));
			});
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserProfile>(AccelByteSubsystemPtr.Get(), LocalUserNum, UserId, OnGetUserProfileCompleteDelegates[LocalUserNum]);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Created and dispatched async task to get user profile!"));
	return true;
}

// ========================================
// Thread-Safe Delegate Access Helper Functions (Phase 1: Critical delegates)
// ========================================


#undef ONLINE_ERROR_NAMESPACE
