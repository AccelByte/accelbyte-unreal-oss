// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUsersByIds.h"
#include "Api/AccelByteUserApi.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystem.h"
#include "Dom/JsonObject.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Interfaces/OnlineUserInterface.h"

using namespace AccelByte;

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
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
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
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

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
			const THandler<FBulkPlatformUserIdResponse> OnBulkGetUserSuccess = TDelegateUtils<THandler<FBulkPlatformUserIdResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsSuccess);
			const FErrorHandler OnBulkGetUserError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsError);
			ApiClient->User.BulkGetUserByOtherPlatformUserIds(ABPlatformType, UserIds, OnBulkGetUserSuccess, OnBulkGetUserError);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::Tick()
{
	Super::Tick();

	if (bHasQueriedBasicUserInfo && bHasQueriedUserPlatformInfo)
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

	// Assign the array of user ids into the platform accounts information request
	PlatformAccountsInfoRequest.UserIds = UsersToQuery;

	const THandler<FAccountUserPlatformInfosResponse> OnGetUserPlatformInfoSuccessDelegate = TDelegateUtils<THandler<FAccountUserPlatformInfosResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoSuccess);
	const FErrorHandler OnGetUserPlatformInfoErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoError);
	ApiClient->User.GetUserOtherPlatformBasicPublicInfo(PlatformAccountsInfoRequest, OnGetUserPlatformInfoSuccessDelegate, OnGetUserPlatformInfoErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoSuccess(const FAccountUserPlatformInfosResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("User information received: %d"), Result.Data.Num());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		TSharedRef<const FUniqueNetIdAccelByteUser> CompositeCurrentUserId = FUniqueNetIdAccelByteUser::Create(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef().Get());

		TArray<TSharedRef<const FUniqueNetId>> PlatformIdsToQuery;
		for (const FAccountUserPlatformData& BasicInfo : Result.Data)
		{
			// Set up our user info struct with basic data
			TSharedRef<FAccelByteUserInfo> User = MakeShared<FAccelByteUserInfo>();
			User->bIsImportant = bIsImportant;
			User->LastAccessedTimeInSeconds = FPlatformTime::Seconds();
			User->PublisherAvatarUrl = BasicInfo.AvatarUrl;
			User->UniqueDisplayName = BasicInfo.DisplayName;

			if (BasicInfo.PlatformInfos.Num() != 0)
			{
				for (const FAccountUserPlatformInfo& UserPlatform : BasicInfo.PlatformInfos)
				{
					if (UserPlatform.PlatformId == CompositeCurrentUserId->GetPlatformId())
					{
						User->GameAvatarUrl = UserPlatform.PlatformAvatarUrl;
						User->DisplayName = UserPlatform.PlatformDisplayName;
					}
				}
			}

			// Construct a composite ID for this user
			FAccelByteUniqueIdComposite CompositeId;
			CompositeId.Id = BasicInfo.UserId;

			ExtractPlatformDataFromBasicUserInfo(BasicInfo, CompositeId);

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
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetBasicUserInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get basic user information from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::QueryUsersOnNativePlatform(const TArray<TSharedRef<const FUniqueNetId>>& PlatformUniqueIds)
{
	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		UE_LOG_AB(Warning, TEXT("Unable to retrieve the native online subsystem! Skipping native platform query."));
		bHasQueriedUserPlatformInfo = true;
		return;
	}

	const IOnlineUserPtr NativeUserInterface = NativeSubsystem->GetUserInterface();
	if (!NativeUserInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("The native platform either does not have UserInterface implemented or is not supported! Skipping native platform query."));
		bHasQueriedUserPlatformInfo = true;
		return;
	}

	// Make a request to the native platform to query all of these IDs that we have retrieved, no need to get the results
	// of these so this can just be a fire and forget
	NativeUserInterface->QueryUserInfo(LocalUserNum, PlatformUniqueIds);
	bHasQueriedUserPlatformInfo = true;
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::ExtractPlatformDataFromBasicUserInfo(const FAccountUserPlatformData& BasicInfo, FAccelByteUniqueIdComposite& CompositeId)
{
	FString FoundPlatformType;
	FString FoundPlatformId;
	for (const auto& KV : BasicInfo.PlatformInfos)
	{
		if (!KV.PlatformUserId.IsEmpty())
		{
			FoundPlatformId = KV.PlatformUserId;
			FoundPlatformType = KV.PlatformId;
			break;
		}
	}

	// Just return if either are empty, otherwise we may have an ID with no type or vice versa
	if (FoundPlatformType.IsEmpty() || FoundPlatformId.IsEmpty())
	{
		return;
	}

	CompositeId.PlatformType = FoundPlatformType;
	CompositeId.PlatformId = FoundPlatformId;
}
