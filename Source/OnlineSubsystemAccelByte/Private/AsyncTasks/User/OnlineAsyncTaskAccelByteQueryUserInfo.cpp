// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserInfo.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Core/AccelByteError.h"

FOnlineAsyncTaskAccelByteQueryUserInfo::FOnlineAsyncTaskAccelByteQueryUserInfo
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const TArray<TSharedRef<const FUniqueNetId>>& InUserIds
	, const FOnQueryUserInfoComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, InitialUserIds(InUserIds)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteQueryUserInfo::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initiating User Index: %d; Initial UserId Amount: %d"), LocalUserNum, InitialUserIds.Num());

	// First, remove any invalid IDs from the initial user ID array
	InitialUserIds.RemoveAll([](const TSharedRef<const FUniqueNetId>& FoundUserId) { return !FoundUserId->IsValid(); });

	// Then, check if we have an empty array for user IDs, and bail out if we do
	if (InitialUserIds.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("No user IDs passed to query user info! Unable to complete!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Set up arrays and counters to be the same length as the amount of user IDs we want to query. These expected counters
	// will be used for synchronization later, the task knows that it is finished when these expected values match their
	// corresponding maps.
	UserIdsToQuery.Empty(InitialUserIds.Num());

	// Iterate through each user ID that we have been requested to query for, convert them to string IDs, and then fire
	// off a request to get account data (to get display name) as well as public profile attributes for each user
	for (const TSharedRef<const FUniqueNetId>& NetId : InitialUserIds)
	{
		if (NetId->GetType() != ACCELBYTE_SUBSYSTEM)
		{
			UE_LOG_AB(Warning, TEXT("NetId passed to FOnlineUserAccelByte::QueryUserInfo (%s) was an invalid type (%s). Query the external mapping first to convert to an AccelByte ID. Skipping this ID!"), *(Subsystem->GetInstanceName().ToString()), *(NetId->GetType().ToString()));
			continue;
		}

		const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(NetId);
		UserIdsToQuery.Add(AccelByteId->GetAccelByteId());
	}

	GetBasicUserInfo();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserInfo::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Iterate through each account that we have received from the backend and add to our user interface
	for (const TSharedRef<FAccelByteUserInfo>& QueriedUser : UsersQueried)
	{

		// Construct the user information instance and add the ID of the user to our QueriedUserIds array that will be passed to the completion delegate
		TSharedRef<FUserOnlineAccountAccelByte> User = MakeShared<FUserOnlineAccountAccelByte>(QueriedUser->Id.ToSharedRef(), QueriedUser->DisplayName);
		User->SetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, QueriedUser->GameAvatarUrl);
		User->SetUserAttribute(ACCELBYTE_ACCOUNT_PUBLISHER_AVATAR_URL, QueriedUser->PublisherAvatarUrl);

		// Add the ID of this user to the list of IDs that we have queried. This will be passed to the delegate to inform
		// consumer of which users we successfully have cached info for.
		TSharedRef<const FUniqueNetId> FoundUserId = User->GetUserId();
		QueriedUserIds.Add(FoundUserId);

		// Add the new user information instance to the user interface's cache so the developer can retrieve it in the delegate handler
		TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
		if (UserInterface != nullptr)
		{
			UserInterface->AddUserInfo(FoundUserId, User);
		}

		// Additionally, get an instance of the identity interface and check if this queried user has an account in it. If so
		// update their information accordingly.
		FOnlineIdentityAccelBytePtr IdentityInterface = nullptr;
		if (!FOnlineIdentityAccelByte::GetFromSubsystem(Subsystem, IdentityInterface))
		{
			continue;
		}

		TSharedPtr<FUserOnlineAccountAccelByte> IdentityAccount = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(IdentityInterface->GetUserAccount(QueriedUser->Id.ToSharedRef().Get()));
		if (IdentityAccount.IsValid())
		{
			IdentityAccount->SetDisplayName(QueriedUser->DisplayName);
			IdentityAccount->SetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, QueriedUser->GameAvatarUrl);
			IdentityAccount->SetUserAttribute(ACCELBYTE_ACCOUNT_PUBLISHER_AVATAR_URL, QueriedUser->PublisherAvatarUrl);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserInfo::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Delegate.Broadcast(LocalUserNum, bWasSuccessful, QueriedUserIds, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserInfo::GetBasicUserInfo()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("IDs to query: %d"), UserIdsToQuery.Num());

	if (UserIdsToQuery.Num() <= 0)
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

	TArray<FString> UsersToQuery;
	// Get users that we already have cached and users that we need to query, filters from the AccelByteIds array
	UserCache->GetQueryAndCacheArrays(UserIdsToQuery, UsersToQuery, UsersCached);

	// This means these users are already in the cache, so we can just skip the query and successfully complete
	if (UsersToQuery.Num() <= 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	const THandler<FListBulkUserInfo> OnBulkGetBasicUserInfoSuccessDelegate = THandler<FListBulkUserInfo>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUserInfo::OnGetBasicUserInfoSuccess);
	const FErrorHandler OnBulkGetBasicUserInfoErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUserInfo::OnGetBasicUserInfoError);
	ApiClient->User.BulkGetUserInfo(UsersToQuery, OnBulkGetBasicUserInfoSuccessDelegate, OnBulkGetBasicUserInfoErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserInfo::OnGetBasicUserInfoSuccess(const FListBulkUserInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("User information received: %d"), Result.Data.Num());

	TArray<TSharedRef<const FUniqueNetId>> PlatformIdsToQuery;
	for (const FBaseUserInfo& BasicInfo : Result.Data)
	{
		// Set up our user info struct with basic data
		TSharedRef<FAccelByteUserInfo> User = MakeShared<FAccelByteUserInfo>();
		User->DisplayName = BasicInfo.DisplayName;
		User->GameAvatarUrl = BasicInfo.AvatarUrl;
		User->PublisherAvatarUrl = BasicInfo.PublisherAvatarUrl;

		// Add the user to our successful queries
		UsersQueried.Add(User);
	}

	UsersQueried.Append(UsersCached);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserInfo::OnGetBasicUserInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get basic user information from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
