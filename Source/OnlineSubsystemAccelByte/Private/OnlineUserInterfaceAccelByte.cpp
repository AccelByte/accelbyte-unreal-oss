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
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteListUserByUserId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteLinkOtherPlatform.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteUnlinkOtherPlatform.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteLinkOtherPlatformId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteUnlinkOtherPlatformId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteCheckUserAccountAvailability.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserIdMappingWithPlatform.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserIdMappingWithPlatformId.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteGetUserPlatformLinks.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteValidateUserInput.h"
#include "AsyncTasks/User/OnlineAsyncTaskAccelByteQueryUserIdsMapping.h"
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
		CreateUserProfile(*UserId.Get());
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

#undef ONLINE_ERROR_NAMESPACE
