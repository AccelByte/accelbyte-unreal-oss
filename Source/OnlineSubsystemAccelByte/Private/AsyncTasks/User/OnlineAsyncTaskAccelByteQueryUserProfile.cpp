// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserProfile.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Core/AccelByteError.h"
#include "OnlineError.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

FOnlineAsyncTaskAccelByteQueryUserProfile::FOnlineAsyncTaskAccelByteQueryUserProfile
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const TArray<TSharedRef<const FUniqueNetId>>& InUserIds 
	, const FOnQueryUserProfileComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, InitialUserIds(InUserIds)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;

	// First, remove any invalid IDs from the initial user ID array
	InitialUserIds.RemoveAll([](const TSharedRef<const FUniqueNetId>& FoundUserId) { return !FoundUserId->IsValid(); });

	// Set up arrays and counters to be the same length as the amount of user IDs we want to query. These expected counters
	// will be used for synchronization later, the task knows that it is finished when these expected values match their
	// corresponding maps.
	UserIdsToQuery.Empty(InitialUserIds.Num());

	// Iterate through each user ID that we have been requested to query for, convert them to string IDs, and then fire
	// off a request to get account user profile for each user
	for (const TSharedRef<const FUniqueNetId>& NetId : InitialUserIds)
	{
		if (NetId->GetType() != ACCELBYTE_SUBSYSTEM)
		{
			UE_LOG_AB(Warning, TEXT("NetId passed to FOnlineUserAccelByte::QueryUserProfile (%s) was an invalid type (%s). Query the external mapping first to convert to an AccelByte ID. Skipping this ID!"), *(Subsystem->GetInstanceName().ToString()), *(NetId->GetType().ToString()));
			continue;
		}

		const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(NetId);
		UserIdsToQuery.Add(AccelByteId->GetAccelByteId());
	}
}

FOnlineAsyncTaskAccelByteQueryUserProfile::FOnlineAsyncTaskAccelByteQueryUserProfile
	(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const TArray<FString>& InUserIds
	, const FOnQueryUserProfileComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, UserIdsToQuery(InUserIds)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

FOnlineAsyncTaskAccelByteQueryUserProfile::FOnlineAsyncTaskAccelByteQueryUserProfile
	(FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId
	, const TArray<TSharedRef<const FUniqueNetId>>& InUserIds
	, const FOnQueryUserProfileComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, InitialUserIds(InUserIds)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);

	// First, remove any invalid IDs from the initial user ID array
	InitialUserIds.RemoveAll([](const TSharedRef<const FUniqueNetId>& FoundUserId) { return !FoundUserId->IsValid(); });

	// Set up arrays and counters to be the same length as the amount of user IDs we want to query. These expected counters
	// will be used for synchronization later, the task knows that it is finished when these expected values match their
	// corresponding maps.
	UserIdsToQuery.Empty(InitialUserIds.Num());

	// Iterate through each user ID that we have been requested to query for, convert them to string IDs, and then fire
	// off a request to get account user profile for each user
	for (const TSharedRef<const FUniqueNetId>& NetId : InitialUserIds)
	{
		if (NetId->GetType() != ACCELBYTE_SUBSYSTEM)
		{
			UE_LOG_AB(Warning, TEXT("NetId passed to FOnlineUserAccelByte::QueryUserProfile (%s) was an invalid type (%s). Query the external mapping first to convert to an AccelByte ID. Skipping this ID!"), *(Subsystem->GetInstanceName().ToString()), *(NetId->GetType().ToString()));
			continue;
		}

		const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(NetId);
		UserIdsToQuery.Add(AccelByteId->GetAccelByteId());
	}
}

FOnlineAsyncTaskAccelByteQueryUserProfile::FOnlineAsyncTaskAccelByteQueryUserProfile
(FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId
	, const TArray<FString>& InUserIds
	, const FOnQueryUserProfileComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, UserIdsToQuery(InUserIds)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryUserProfile::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initiating User Index: %d; Initial UserId Amount: %d"), LocalUserNum, InitialUserIds.Num());

	// Then, check if we have an empty array for user IDs, and bail out if we do
	if (UserIdsToQuery.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("No user IDs passed to query user profile! Unable to complete!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	auto OnQueryUsersProfileCompleteDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsPublicUserProfileInfo>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUserProfile::OnQueryUsersProfileComplete);
	auto OnQueryUserProfileErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUserProfile::OnQueryUserProfileError);
	ApiClient->UserProfile.BulkGetPublicUserProfileInfos(UserIdsToQuery, OnQueryUsersProfileCompleteDelegate, OnQueryUserProfileErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserProfile::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	const FOnlineUserCacheAccelBytePtr UserCache = Subsystem->GetUserCache();

	for (const auto& InUserQueried : UsersQueried)
	{
		FAccelByteUniqueIdComposite UserCompositeId;
		UserCompositeId.Id = InUserQueried.UserId;
		TSharedPtr<const FAccelByteUserInfo> CachedUserInfo = UserCache->GetUser(UserCompositeId);
		TSharedPtr<FUserOnlineAccountAccelByte> Account;
		if (CachedUserInfo.IsValid())
		{
			Account = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(IdentityInterface->GetUserAccount(*CachedUserInfo->Id.Get()));
		}
		else
		{
			TSharedRef<FAccelByteUserInfo> TempUserInfo = MakeShared<FAccelByteUserInfo>();
			TempUserInfo->Id = FUniqueNetIdAccelByteUser::Create(UserCompositeId);
			UserCache->AddUsersToCache({ TempUserInfo });
			CachedUserInfo = TempUserInfo;
		}

		if (!Account.IsValid())
		{
			Account = MakeShared<FUserOnlineAccountAccelByte>(CachedUserInfo->Id.ToSharedRef());
			Account->SetDisplayName(ApiClient->CredentialsRef->GetUserDisplayName());
			Account->SetAccessToken(ApiClient->CredentialsRef->GetAccessToken());

			if (UserId.Get() == CachedUserInfo->Id.Get())
			{
				IdentityInterface->AddNewAuthenticatedUser(LocalUserNum, CachedUserInfo->Id.ToSharedRef(), Account.ToSharedRef());
			}
		}

		// Set attributes for the user account based on profile data
		Account->SetUserAttribute(TEXT("AvatarURL"), InUserQueried.AvatarUrl);
		Account->SetUserAttribute(TEXT("AvatarSmallURL"), InUserQueried.AvatarSmallUrl);
		Account->SetUserAttribute(TEXT("AvatarLargeURL"), InUserQueried.AvatarLargeUrl);

		TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterfacePtr = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
		if (InUserQueried.PublicId.IsEmpty())
		{
			UE_LOG_AB(Warning, TEXT("Failed to set FriendCode text. Player FriendCode (PublicCode) is empty!"));
		}
		else
		{
			Account->SetPublicCode(InUserQueried.PublicId);
			UserCache->AddPublicCodeToCache(*CachedUserInfo->Id.ToSharedRef(), InUserQueried.PublicId);
		}

		// Add the ID of this user to the list of IDs that we have queried. This will be passed to the delegate to inform
		// consumer of which users we successfully have cached info for.
		TSharedRef<const FUniqueNetId> FoundUserId = Account->GetUserId();
		QueriedUserIds.Add(FoundUserId);

		// Add the new user information instance to the user interface's cache so the developer can retrieve it in the delegate handler
		TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
		if (UserInterface != nullptr)
		{
			UserInterface->AddUserInfo(FoundUserId, Account.ToSharedRef());
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserProfile::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	Delegate.Broadcast(LocalUserNum, bWasSuccessful, QueriedUserIds, ONLINE_ERROR(Result, ErrorStr));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserProfile::OnQueryUsersProfileComplete(const TArray<FAccelByteModelsPublicUserProfileInfo>& InUsersQueried)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UsersQueried = InUsersQueried;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully have a user profile!"));
}

void FOnlineAsyncTaskAccelByteQueryUserProfile::OnQueryUserProfileError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("query-user-profile-error-response");
	UE_LOG_AB(Warning, TEXT("Failed to query users from the backend!"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE
