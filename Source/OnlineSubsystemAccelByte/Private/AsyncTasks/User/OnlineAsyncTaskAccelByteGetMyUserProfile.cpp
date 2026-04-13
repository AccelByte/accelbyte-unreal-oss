// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetMyUserProfile.h"

#include "Core/AccelByteError.h"
#include "Core/AccelByteUtilities.h"
#include "Api/AccelByteUserProfileApi.h"
#include "OnlineError.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

FOnlineAsyncTaskAccelByteGetMyUserProfile::FOnlineAsyncTaskAccelByteGetMyUserProfile
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FOnGetMyUserProfileComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteGetMyUserProfile::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Check if this is a dedicated server - user profile operations are not intended for DS
	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (IsDS.IsSet() && IsDS.GetValue())
	{
		ErrorString = TEXT("request-failed-get-own-userprofile-ds-not-allowed");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get own userProfile, this request is not intended for DS!"));
		return;
	}

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-get-own-userprofile-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get own userProfile, identity interface is invalid!"));
		return;
	}

	UserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));

	if (!UserId.IsValid())
	{
		ErrorString = TEXT("request-failed-get-own-userprofile-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get own userProfile, user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-get-own-userprofile-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get own userProfile, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	Account = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(IdentityInterface->GetUserAccount(*UserId.Get()));
	if (!Account.IsValid())
	{
		Account = MakeShared<FUserOnlineAccountAccelByte>(UserId.ToSharedRef());
		Account->SetDisplayName(ApiClient->CredentialsRef->GetUserDisplayName());
		Account->SetCredentialsRef(ApiClient->CredentialsRef);
		Account->SetUniqueDisplayName(ApiClient->CredentialsRef->GetUniqueDisplayName());

		IdentityInterface->AddNewAuthenticatedUser(LocalUserNum, UserId.ToSharedRef(), Account.ToSharedRef());
	}

	GetMyUserProfile();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMyUserProfile::GetMyUserProfile()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Create success and error delegates
	OnGetMyUserProfileSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserProfileInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMyUserProfile::OnGetMyUserProfileSuccess);
	OnGetMyUserProfileErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetMyUserProfile::OnGetMyUserProfileError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->GetUserProfile(OnGetMyUserProfileSuccessDelegate, OnGetMyUserProfileErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to get own user profile for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteGetMyUserProfile::OnGetMyUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UserProfileInfo = Result;

	// Cache user attributes in Account object - include all fields since this is complete profile with private data
	Account->SetUserAttribute(TEXT("AvatarURL"), UserProfileInfo.AvatarUrl);
	Account->SetUserAttribute(TEXT("AvatarSmallURL"), UserProfileInfo.AvatarSmallUrl);
	Account->SetUserAttribute(TEXT("AvatarLargeURL"), UserProfileInfo.AvatarLargeUrl);
	Account->SetUserAttribute(TEXT("UserLanguage"), UserProfileInfo.Language);
	Account->SetUserAttribute(TEXT("FirstName"), UserProfileInfo.FirstName);
	Account->SetUserAttribute(TEXT("LastName"), UserProfileInfo.LastName);
	Account->SetUserAttribute(TEXT("Timezone"), UserProfileInfo.Timezone);
	Account->SetUserAttribute(TEXT("DateOfBirth"), UserProfileInfo.DateOfBirth);
	Account->SetUserAttribute(TEXT("ZipCode"), UserProfileInfo.ZipCode);
	Account->SetPublicCode(UserProfileInfo.PublicId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMyUserProfile::OnGetMyUserProfileError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-get-own-userprofile-error");
	UE_LOG_AB(Warning, TEXT("Failed to get own user profile! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetMyUserProfile::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// Cache user in the interface
		FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
		if (UserInterface.IsValid() && Account.IsValid())
		{
			UserInterface->AddUserInfo(UserId.ToSharedRef(), Account.ToSharedRef());
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetMyUserProfile::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnGetMyUserProfileCompleteDelegates(LocalUserNum, bWasSuccessful, UserProfileInfo, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnGetMyUserProfileCompleteDelegates(LocalUserNum, bWasSuccessful, FAccelByteModelsUserProfileInfo{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE