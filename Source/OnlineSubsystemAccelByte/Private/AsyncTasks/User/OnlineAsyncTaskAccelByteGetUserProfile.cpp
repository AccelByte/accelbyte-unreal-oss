// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetUserProfile.h"

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

FOnlineAsyncTaskAccelByteGetUserProfile::FOnlineAsyncTaskAccelByteGetUserProfile
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FString& InUserId
	, const FOnGetUserProfileComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, UserId(InUserId)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteGetUserProfile::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId);

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-get-user-profile-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user profile, identity interface is invalid!"));
		return;
	}

	FUniqueNetIdPtr CurrentUserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));

	if (!CurrentUserId.IsValid())
	{
		ErrorString = TEXT("request-failed-get-user-profile-current-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user profile, current user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*CurrentUserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-get-user-profile-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user profile, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	GetUserProfile();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserProfile::GetUserProfile()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId);

	// Create success and error delegates
	OnGetUserProfileSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserProfileInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserProfile::OnGetUserProfileSuccess);
	OnGetUserProfileErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserProfile::OnGetUserProfileError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->GetUserProfile(UserId, OnGetUserProfileSuccessDelegate, OnGetUserProfileErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to get user profile for UserId '%s'!"), *UserId);
}

void FOnlineAsyncTaskAccelByteGetUserProfile::OnGetUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UserProfileInfo = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserProfile::OnGetUserProfileError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-get-user-profile-error");
	UE_LOG_AB(Warning, TEXT("Failed to get user profile! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetUserProfile::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Note: We don't cache this data as it's for accessing other users' complete profiles
	// The result is passed directly to delegates

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserProfile::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnGetUserProfileCompleteDelegates(LocalUserNum, bWasSuccessful, UserProfileInfo, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnGetUserProfileCompleteDelegates(LocalUserNum, bWasSuccessful, FAccelByteModelsUserProfileInfo{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE