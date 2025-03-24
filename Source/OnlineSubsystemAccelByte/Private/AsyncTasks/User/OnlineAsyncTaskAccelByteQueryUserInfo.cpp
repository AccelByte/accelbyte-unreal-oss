// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserInfo.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Core/AccelByteError.h"

using namespace AccelByte;

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
	TRY_PIN_SUBSYSTEM();

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
	for (FUniqueNetIdRef const& NetId : InitialUserIds)
	{
		if (NetId->GetType() != ACCELBYTE_USER_ID_TYPE)
		{
			UE_LOG_AB(Warning, TEXT("NetId passed to FOnlineUserAccelByte::QueryUserInfo (%s) was an invalid type (%s). Query the external mapping first to convert to an AccelByte ID. Skipping this ID!"), *(SubsystemPin->GetInstanceName().ToString()), *(NetId->GetType().ToString()));
			continue;
		}

		const TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(NetId);
		UserIdsToQuery.Add(AccelByteId->GetAccelByteId());
	}

	// Try and query all of the users and cache them in the user store
	const FOnlineUserCacheAccelBytePtr UserCache = SubsystemPin->GetUserCache();
	if (!UserCache.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query information on users as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, UserCache]()
		{
			const FOnQueryUsersComplete OnQueryUsersCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUserInfo::OnQueryUsersComplete);
			UserCache->QueryUsersByAccelByteIds(LocalUserNum, UserIdsToQuery, OnQueryUsersCompleteDelegate);
		}));
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserInfo::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Iterate through each account that we have received from the backend and add to our user interface
	for (const FAccelByteUserInfoRef& QueriedUser : UsersQueried)
	{
		// Additionally, get an instance of the identity interface and check if this queried user has an account in it. If so
		// update their information accordingly.
		FOnlineIdentityAccelBytePtr IdentityInterface = nullptr;
		if (!FOnlineIdentityAccelByte::GetFromSubsystem(SubsystemPin.Get(), IdentityInterface))
		{
			continue;
		}
		
		// Check from identity user account cache first and correspond to update information from it.
		TSharedPtr<FUserOnlineAccountAccelByte> IdentityAccount = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(IdentityInterface->GetUserAccount(QueriedUser->Id.ToSharedRef().Get()));
		if (!IdentityAccount.IsValid())
		{
			// Create a new account instance if one does not already exist
			IdentityAccount = MakeShared<FUserOnlineAccountAccelByte>(QueriedUser->Id.ToSharedRef(), QueriedUser->DisplayName);
		}
		
		// Set user display name using desired source
		FString DisplayName{};
		const EAccelBytePlatformType DisplayNameSource = SubsystemPin->GetDisplayNameSource();
		if (DisplayNameSource == EAccelBytePlatformType::None)
		{
			// If display name source is explicitly default, or the provided source is invalid
			UE_LOG_AB(Verbose, TEXT("Using default display name source for queried user"));
			DisplayName = QueriedUser->DisplayName;
		}
		else
		{
			// Otherwise, attempt to retrieve platform display name using evaluated platform type
			const FAccelByteLinkedUserInfo* FoundLinkedPlatform = QueriedUser->LinkedPlatformInfo.FindByPredicate([&DisplayNameSource](const FAccelByteLinkedUserInfo& Info) {
				return DisplayNameSource == Info.PlatformType;
			});

			if (FoundLinkedPlatform != nullptr)
			{
				DisplayName = FoundLinkedPlatform->DisplayName;
			}
			else
			{
				UE_LOG_AB(Warning, TEXT("Configured display name source '%s' was not found, reverting to default"), *FAccelByteUtilities::GetUEnumValueAsString(DisplayNameSource));
				DisplayName = QueriedUser->DisplayName;
			}
		}

		IdentityAccount->SetDisplayName(DisplayName);
		IdentityAccount->SetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, QueriedUser->GameAvatarUrl);
		IdentityAccount->SetUserAttribute(ACCELBYTE_ACCOUNT_PUBLISHER_AVATAR_URL, QueriedUser->PublisherAvatarUrl);
		IdentityAccount->SetPublicCode(QueriedUser->PublicCode);
		IdentityAccount->SetUniqueDisplayName(QueriedUser->UniqueDisplayName);

		// Add the new user information instance to the user interface's cache so the developer can retrieve it in the delegate handler
		TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
		if (UserInterface != nullptr && IdentityAccount.IsValid())
		{
			TSharedRef<const FUniqueNetId> FoundUserId = QueriedUser->Id.ToSharedRef();
			
			UserInterface->AddUserInfo(FoundUserId, IdentityAccount.ToSharedRef());
			
			// Add the ID of this user to the list of IDs that we have queried. This will be passed to the delegate to inform
			// consumer of which users we successfully have cached info for.
			QueriedUserIds.Add(FoundUserId);
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

void FOnlineAsyncTaskAccelByteQueryUserInfo::OnQueryUsersComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> InUsersQueried)
{
	if (bIsSuccessful)
	{
		UsersQueried = InUsersQueried;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorStr = TEXT("query-users-error-response");
		UE_LOG_AB(Warning, TEXT("Failed to query users from the backend!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}
