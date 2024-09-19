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

void FOnlineAsyncTaskAccelByteQueryUsersByIds::BulkGetUserByOtherPlatformUserIds(const TArray<FString>& InUserIds)
{
	TRY_PIN_SUBSYSTEM()

	EAccelBytePlatformType ABPlatformType;
	if (SubsystemPin->GetAccelBytePlatformTypeFromAuthType(PlatformType, ABPlatformType))
	{
		const THandler<FBulkPlatformUserIdResponse> OnBulkGetUserSuccess = TDelegateUtils<THandler<FBulkPlatformUserIdResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsSuccess);
		const FErrorHandler OnBulkGetUserError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsError);
		API_CLIENT_CHECK_GUARD();
		ApiClient->User.BulkGetUserByOtherPlatformUserIdsV4(ABPlatformType, InUserIds, OnBulkGetUserSuccess, OnBulkGetUserError);
	}
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

	FAccelByteUtilities::SplitArraysToNum(UserIds, MaximumQueryLimit, SplitUserIds);

	// If these are already AccelByte IDs, then we just want to run a bulk query for the users
	if (PlatformType == ACCELBYTE_QUERY_TYPE)
	{
		GetBasicUserInfo(UserIds);
	}
	else
	{
		BulkGetUserByOtherPlatformUserIds(SplitUserIds[LastSplitQueryIndex.GetValue()]);
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
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (UsersQueried.Num() > 0)
	{
		FOnlineUserCacheAccelBytePtr UserCache = SubsystemPin->GetUserCache();
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
	TArray<FAccelByteUserInfoRef> ReturnUsers;
	
	// Only append queried and cached arrays on success
	if(UsersQueried.Num() > 0)
	{
		ReturnUsers.Append(UsersQueried);
	}

	if(UsersCached.Num() > 0)
	{
		ReturnUsers.Append(UsersCached);
	}

	// Fire off the delegate
	Delegate.ExecuteIfBound(bWasSuccessful, ReturnUsers);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnBulkQueryPlatformIdMappingsSuccess(const FBulkPlatformUserIdResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Mappings found: %d"), Result.UserIdPlatforms.Num());

	QueriedUserMapByPlatformUserIds.Append(Result.UserIdPlatforms);
	LastSplitQueryIndex.Increment();
	SetLastUpdateTimeToCurrentTime();
	
	if(LastSplitQueryIndex.GetValue() < SplitUserIds.Num())
	{
		BulkGetUserByOtherPlatformUserIds(SplitUserIds[LastSplitQueryIndex.GetValue()]);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Querying next platform user Ids"));
		return;
	}

	if (QueriedUserMapByPlatformUserIds.Num() > 0)
	{
		TArray<FString> AccelByteIds;
		for (const FPlatformUserIdMap& UserIdMapping : QueriedUserMapByPlatformUserIds)
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
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("IDs to query: %d"), AccelByteIds.Num());

	if (AccelByteIds.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as our array of user IDs is blank!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const FOnlineUserCacheAccelBytePtr UserCache = SubsystemPin->GetUserCache();
	if (!UserCache.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Get users that we already have cached and users that we need to query, filters from the AccelByteIds array
	TArray<FString> UserIdsToQueryArray;
	UserCache->GetQueryAndCacheArrays(AccelByteIds, UserIdsToQueryArray, UsersCached);

	// Add userId that is not in the cache to the queue
	for (const auto& AccelbyteUserId : UserIdsToQueryArray)
	{
		UsersToQuery.Enqueue(AccelbyteUserId);
	}

	// This means these users are already in the cache, so we can just skip the query and successfully complete
	if (UsersToQuery.IsEmpty())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	GetUserOtherPlatformBasicPublicInfo();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::GetUserOtherPlatformBasicPublicInfo()
{
	FPlatformAccountInfoRequest Request;

	// Add user id to query until queue empty or limit reached
	while (!UsersToQuery.IsEmpty() && Request.UserIds.Num() < MaximumQueryLimit)
	{
		FString AccelbyteUserId;
		UsersToQuery.Dequeue(AccelbyteUserId);
		Request.UserIds.Emplace(AccelbyteUserId);
	}

	const THandler<FAccountUserPlatformInfosResponse> OnGetUserPlatformInfoSuccessDelegate = TDelegateUtils<THandler<FAccountUserPlatformInfosResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetUserOtherPlatformBasicPublicInfoSuccess);
	const FErrorHandler OnGetUserPlatformInfoErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetUserOtherPlatformBasicPublicInfoError);
	API_CLIENT_CHECK_GUARD();
	ApiClient->User.GetUserOtherPlatformBasicPublicInfo(Request, OnGetUserPlatformInfoSuccessDelegate, OnGetUserPlatformInfoErrorDelegate);
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetUserOtherPlatformBasicPublicInfoSuccess(const FAccountUserPlatformInfosResponse& Result)
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("User information received: %d"), Result.Data.Num());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as our Identity Interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as Local User is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::Create(*LocalUserId.Get());
	if (!AccelByteUserId->IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as AccelByte Local User is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnlineUserCacheAccelBytePtr UserCache = SubsystemPin->GetUserCache();
	if (!UserCache.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query users as AccelByte user cache interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	for (const FAccountUserPlatformData& BasicInfo : Result.Data)
	{
		// Construct a composite ID for this user
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = BasicInfo.UserId;

		ExtractPlatformDataFromBasicUserInfo(BasicInfo, CompositeId);

		// Set up our user info struct with basic data, get existing userInfo if not exist, create new
		FAccelByteUserInfoPtr User = ConstCastSharedPtr<FAccelByteUserInfo>(UserCache->GetUser(CompositeId));
		if (!User.IsValid())
		{
			User = MakeShared<FAccelByteUserInfo, ESPMode::ThreadSafe>();
			User->Id = FUniqueNetIdAccelByteUser::Create(CompositeId);
		}
		User->bIsImportant = bIsImportant;
		User->LastAccessedTimeInSeconds = FPlatformTime::Seconds();
		User->PublisherAvatarUrl = BasicInfo.AvatarUrl;
		User->UniqueDisplayName = BasicInfo.UniqueDisplayName;
		User->LastUpdatedTime = FDateTime::UtcNow();

		// By default, we would provide the initial value using the basic platform information
		User->DisplayName = BasicInfo.DisplayName;
		User->GameAvatarUrl = BasicInfo.AvatarUrl;

		// Override based on the platform information
		User->LinkedPlatformInfo.Empty();
		if (BasicInfo.PlatformInfos.Num() != 0)
		{
			for (const FAccountUserPlatformInfo& UserPlatform : BasicInfo.PlatformInfos)
			{
				FAccelByteLinkedUserInfo PlatformUserInfo;
				PlatformUserInfo.Id =  FUniqueNetIdAccelByteUser::Create(FAccelByteUniqueIdComposite(BasicInfo.UserId, UserPlatform.PlatformId, UserPlatform.PlatformUserId));
				PlatformUserInfo.PlatformId = UserPlatform.PlatformUserId;
				PlatformUserInfo.AvatarUrl = UserPlatform.PlatformAvatarUrl;
				PlatformUserInfo.DisplayName = UserPlatform.PlatformDisplayName;
				User->LinkedPlatformInfo.Add(PlatformUserInfo);
			}
		}

		// Add the user to our successful queries
		UsersQueried.Add(User.ToSharedRef());

		// Also query the user on the native platform, if we have their platform information
		FUniqueNetIdPtr PlatformUniqueId = User->Id->GetPlatformUniqueId();
		if (PlatformUniqueId.IsValid() && PlatformUniqueId->IsValid())
		{
			PlatformIdsToQuery.Add(PlatformUniqueId.ToSharedRef());
		}
	}

	if (UsersToQuery.IsEmpty())
	{
		bHasQueriedBasicUserInfo = true;

		if (PlatformIdsToQuery.Num() > 0)
		{
			QueryUsersOnNativePlatform(PlatformIdsToQuery);
		}
		else
		{
			// Just set this flag to true so that we aren't waiting on it
			bHasQueriedUserPlatformInfo = true;
		}
	}
	else
	{
		GetUserOtherPlatformBasicPublicInfo();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::OnGetUserOtherPlatformBasicPublicInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get basic user information from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryUsersByIds::QueryUsersOnNativePlatform(const TArray<TSharedRef<const FUniqueNetId>>& PlatformUniqueIds)
{
	TRY_PIN_SUBSYSTEM()

	const IOnlineSubsystem* NativeSubsystem = SubsystemPin->GetNativePlatformSubsystem();
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
	TRY_PIN_SUBSYSTEM()

	// Grab our own user account from the identity interface to determine which platform we should grab for composite IDs
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		return;
	}

	TSharedPtr<FUserOnlineAccountAccelByte> LocalUserAccount = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(IdentityInterface->GetUserAccount(UserId.ToSharedRef().Get()));
	if (!LocalUserAccount.IsValid())
	{
		return;
	}

	FString FoundPlatformType;
	FString FoundPlatformId;
	for (const auto& KV : BasicInfo.PlatformInfos)
	{
		if (!KV.PlatformUserId.IsEmpty() && KV.PlatformUserId.Equals(LocalUserAccount->GetPlatformUserId()))
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
