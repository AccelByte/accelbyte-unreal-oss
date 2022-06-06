// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUsersByIds.h"
#include "Api/AccelByteUserApi.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystem.h"
#include "Dom/JsonObject.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Interfaces/OnlineUserInterface.h"

#define ACCELBYTE_QUERY_TYPE TEXT("ACCELBYTE")

FOnlineAsyncTaskAccelByteQueryUsersByIds::FOnlineAsyncTaskAccelByteQueryUsersByIds
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const TArray<FString>& AccelByteIds
	, bool InBIsImportant
	, const FOnQueryUsersComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, PlatformType(ACCELBYTE_QUERY_TYPE)
	, UserIds(AccelByteIds)
	, bIsImportant(InBIsImportant)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

FOnlineAsyncTaskAccelByteQueryUsersByIds::FOnlineAsyncTaskAccelByteQueryUsersByIds
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FString InPlatformType
	, const TArray<FString>& PlatformIds
	, bool InBIsImportant
	, const FOnQueryUsersComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, PlatformType(InPlatformType)
	, UserIds(PlatformIds)
	, bIsImportant(InBIsImportant)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

FOnlineAsyncTaskAccelByteQueryUsersByIds::FOnlineAsyncTaskAccelByteQueryUsersByIds
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId
	, const TArray<FString>& AccelByteIds
	, bool InBIsImportant
	, const FOnQueryUsersComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, PlatformType(ACCELBYTE_QUERY_TYPE)
	, UserIds(AccelByteIds)
	, bIsImportant(InBIsImportant)
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
}

FOnlineAsyncTaskAccelByteQueryUsersByIds::FOnlineAsyncTaskAccelByteQueryUsersByIds
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId
	, const FString InPlatformType
	, const TArray<FString>& PlatformIds
	, bool InBIsImportant
	, const FOnQueryUsersComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, PlatformType(InPlatformType)
	, UserIds(PlatformIds)
	, bIsImportant(InBIsImportant)
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""), LOG_BOOL_FORMAT(bWasSuccessful));

	if (UserIds.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query users as the array of IDs to query was blank!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// If these are already AccelByte IDs, then we just want to run a bulk query for the users
	if (PlatformType == ACCELBYTE_QUERY_TYPE)
	{
		GetBasicUserInfo(UserIds);
	}
	else
	{
		EAccelBytePlatformType ABPlatformType;
		if (Subsystem->GetAccelBytePlatformTypeFromAuthType(PlatformType, ABPlatformType))
		{
			const THandler<FBulkPlatformUserIdResponse> OnBulkGetUserSuccess = THandler<FBulkPlatformUserIdResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsSuccess);
			const FErrorHandler OnBulkGetUserError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsError);
			ApiClient->User.BulkGetUserByOtherPlatformUserIds(ABPlatformType, UserIds, OnBulkGetUserSuccess, OnBulkGetUserError);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::Tick()
{
	Super::Tick();

	if (bHasQueriedBasicUserInfo && bHasQueriedPublicUserProfile && bHasQueriedUserPlatformInfo)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineUserCacheAccelBytePtr UserCache = Subsystem->GetUserCache();
		if (UserCache.IsValid())
		{
			UserCache->AddUsersToCache(UsersQueried);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Concatenate array of queried and cached users to return to the delegate
	TArray<TSharedRef<FAccelByteUserInfo>> ReturnUsers;
	
	// Only append queried and cached arrays on success
	if (bWasSuccessful)
	{
		ReturnUsers.Append(UsersQueried);
		ReturnUsers.Append(UsersCached);
	}

	// Fire off the delegate
	Delegate.ExecuteIfBound(bWasSuccessful, ReturnUsers);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsSuccess(const FBulkPlatformUserIdResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Mappings found: %d"), Result.UserIdPlatforms.Num());

	if (Result.UserIdPlatforms.Num() > 0)
	{
		TArray<FString> AccelByteIds;
		for (const FPlatformUserIdMap& UserIdMapping : Result.UserIdPlatforms)
		{
			AccelByteIds.Add(UserIdMapping.UserId);
		}

		GetBasicUserInfo(AccelByteIds);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Could not query for AccelByte IDs from %s platform IDs in bulk! Error code: %d; Error message: %s"), *PlatformType, ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::GetBasicUserInfo(const TArray<FString>& AccelByteIds)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("IDs to query: %d"), AccelByteIds.Num());

	if (AccelByteIds.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as our array of user IDs is blank!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const FOnlineUserCacheAccelBytePtr UserCache = Subsystem->GetUserCache();
	if (!UserCache.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Get users that we already have cached and users that we need to query, filters from the AccelByteIds array
	UserCache->GetQueryAndCacheArrays(AccelByteIds, UsersToQuery, UsersCached);

	// This means these users are already in the cache, so we can just skip the query and successfully complete
	if (UsersToQuery.Num() <= 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	const THandler<FListBulkUserInfo> OnBulkGetBasicUserInfoSuccessDelegate = THandler<FListBulkUserInfo>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoSuccess);
	const FErrorHandler OnBulkGetBasicUserInfoErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoError);
	ApiClient->User.BulkGetUserInfo(UsersToQuery, OnBulkGetBasicUserInfoSuccessDelegate, OnBulkGetBasicUserInfoErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoSuccess(const FListBulkUserInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("User information received: %d"), Result.Data.Num());

	TArray<TSharedRef<const FUniqueNetId>> PlatformIdsToQuery;
	for (const FBaseUserInfo& BasicInfo : Result.Data)
	{
		// Set up our user info struct with basic data
		TSharedRef<FAccelByteUserInfo> User = MakeShared<FAccelByteUserInfo>();
		User->DisplayName = BasicInfo.DisplayName;
		User->bIsImportant = bIsImportant;
		User->LastAccessedTimeInSeconds = FPlatformTime::Seconds();

		// Construct a composite ID for this user
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = BasicInfo.UserId;
		
		// Create the final user ID for the queried user
		User->Id = FUniqueNetIdAccelByteUser::Create(CompositeId);

		// Add the user to our successful queries
		UsersQueried.Add(User);

		// Also query the user on the native platform, if we have their platform information
		TSharedPtr<const FUniqueNetId> PlatformUniqueId = User->Id->GetPlatformUniqueId();
		if (PlatformUniqueId.IsValid() && PlatformUniqueId->IsValid())
		{
			PlatformIdsToQuery.Add(PlatformUniqueId.ToSharedRef());
		}
	}

	const auto OnGetPublicUserProfilesSuccessDelegate = THandler<TArray<FAccelByteModelsPublicUserProfileInfo>>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetPublicUserProfilesSuccess);
	const FErrorHandler OnGetPublicUserProfilesErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetPublicUserProfilesError);

	ApiClient->UserProfile.BatchGetPublicUserProfileInfos(FString::Join(UsersToQuery, TEXT(",")), OnGetPublicUserProfilesSuccessDelegate, OnGetPublicUserProfilesErrorDelegate);
	if (PlatformIdsToQuery.Num() > 0)
	{
		QueryUsersOnNativePlatform(PlatformIdsToQuery);
	}
	else
	{
		// Just set this flag to true so that we aren't waiting on it
		bHasQueriedUserPlatformInfo = true;
	}

	bHasQueriedBasicUserInfo = true;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get basic user information from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetPublicUserProfilesSuccess(const TArray<FAccelByteModelsPublicUserProfileInfo>& Results)
{
	// Go through each profile that we have retrieved and add public ID to their user information struct
	for (const FAccelByteModelsPublicUserProfileInfo& Profile : Results)
	{
		// First, find the corresponding user that we have already queried to add this information to
		TSharedRef<FAccelByteUserInfo>* User = UsersQueried.FindByPredicate([&Profile](const TSharedRef<FAccelByteUserInfo>& TestUser) {
			return TestUser->Id->GetAccelByteId() == Profile.UserId;
		});

		// Skip this profile if a corresponding user cannot be found
		if (User == nullptr)
		{
			continue;
		}

		(*User)->AvatarUrl = Profile.AvatarUrl;
		(*User)->AvatarSmallUrl = Profile.AvatarSmallUrl;
		(*User)->AvatarLargeUrl = Profile.AvatarLargeUrl;
	}

	bHasQueriedPublicUserProfile = true;
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetPublicUserProfilesError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get public user profile information from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::QueryUsersOnNativePlatform(const TArray<TSharedRef<const FUniqueNetId>>& PlatformUniqueIds)
{
	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		return;
	}

	const IOnlineUserPtr NativeUserInt = NativeSubsystem->GetUserInterface();
	if (!NativeUserInt.IsValid())
	{
		return;
	}

	// Make a request to the native platform to query all of these IDs that we have retrieved, no need to get the results
	// of these so this can just be a fire and forget
	NativeUserInt->QueryUserInfo(LocalUserNum, PlatformUniqueIds);
	bHasQueriedUserPlatformInfo = true;
}
