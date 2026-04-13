// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId.h"

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

FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FString& InPublicId
	, const FOnGetPublicUserProfileByPublicIdComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, PublicId(InPublicId)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PublicId: %s"), *PublicId);

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-get-public-userprofile-by-public-id-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public userProfile by PublicId, identity interface is invalid!"));
		return;
	}

	UserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));

	if (!UserId.IsValid())
	{
		ErrorString = TEXT("request-failed-get-public-userprofile-by-public-id-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public userProfile by PublicId, user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-get-public-userprofile-by-public-id-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get public userProfile by PublicId, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	GetPublicUserProfileByPublicId();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::GetPublicUserProfileByPublicId()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PublicId: %s"), *PublicId);

	// Create success and error delegates
	OnGetPublicUserProfileByPublicIdSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsPublicUserProfileInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::OnGetPublicUserProfileByPublicIdSuccess);
	OnGetPublicUserProfileByPublicIdErrorDelegate = TDelegateUtils<FCustomErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::OnGetPublicUserProfileByPublicIdError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->GetUserProfilePublicInfoByPublicId(PublicId, OnGetPublicUserProfileByPublicIdSuccessDelegate, OnGetPublicUserProfileByPublicIdErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to get public user profile by PublicId '%s'!"), *PublicId);
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::OnGetPublicUserProfileByPublicIdSuccess(const FAccelByteModelsPublicUserProfileInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	PublicUserProfileInfo = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::OnGetPublicUserProfileByPublicIdError(int32 ErrorCode, const FString& ErrorMessage, const FJsonObject& JsonObject)
{
	ErrorString = TEXT("request-failed-get-public-userprofile-by-public-id-error");
	UE_LOG_AB(Warning, TEXT("Failed to get public user profile by PublicId! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Note: We don't cache this data in user accounts as it's for a different user and we only have PublicId
	// The result is passed directly to delegates

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnGetPublicUserProfileByPublicIdCompleteDelegates(LocalUserNum, bWasSuccessful, PublicUserProfileInfo, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnGetPublicUserProfileByPublicIdCompleteDelegates(LocalUserNum, bWasSuccessful, FAccelByteModelsPublicUserProfileInfo{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE