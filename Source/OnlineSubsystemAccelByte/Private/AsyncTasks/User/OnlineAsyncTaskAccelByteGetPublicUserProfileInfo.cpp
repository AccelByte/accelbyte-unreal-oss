// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetPublicUserProfileInfo.h"

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

FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FString& InTargetUserId
	, const FOnGetPublicUserProfileInfoComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, UserId(InTargetUserId)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TargetUserId: %s"), *UserId);

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-get-public-userprofile-info-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public userProfile info, identity interface is invalid!"));
		return;
	}

	FUniqueNetIdPtr CurrentUserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));

	if (!CurrentUserId.IsValid())
	{
		ErrorString = TEXT("request-failed-get-public-userprofile-info-current-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public userProfile info, current user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*CurrentUserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-get-public-userprofile-info-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public userProfile info, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	GetPublicUserProfileInfo();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::GetPublicUserProfileInfo()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TargetUserId: %s"), *UserId);

	// Create success and error delegates
	OnGetPublicUserProfileInfoSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsPublicUserProfileInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::OnGetPublicUserProfileInfoSuccess);
	OnGetPublicUserProfileInfoErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::OnGetPublicUserProfileInfoError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->GetPublicUserProfileInfo(UserId, OnGetPublicUserProfileInfoSuccessDelegate, OnGetPublicUserProfileInfoErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to get public user profile info for TargetUserId '%s'!"), *UserId);
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::OnGetPublicUserProfileInfoSuccess(const FAccelByteModelsPublicUserProfileInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	PublicUserProfileInfo = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::OnGetPublicUserProfileInfoError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-get-public-userprofile-info-error");
	UE_LOG_AB(Warning, TEXT("Failed to get public user profile info! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Note: We don't cache this data in user accounts as it's for a different user
	// The result is passed directly to delegates

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnGetPublicUserProfileInfoCompleteDelegates(LocalUserNum, bWasSuccessful, PublicUserProfileInfo, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnGetPublicUserProfileInfoCompleteDelegates(LocalUserNum, bWasSuccessful, FAccelByteModelsPublicUserProfileInfo{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE