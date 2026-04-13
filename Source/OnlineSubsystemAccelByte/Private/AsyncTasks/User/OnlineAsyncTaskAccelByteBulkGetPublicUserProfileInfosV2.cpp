// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2.h"

#include "Core/AccelByteError.h"
#include "Core/AccelByteUtilities.h"
#include "Api/AccelByteUserProfileApi.h"
#include "OnlineError.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const TArray<FString>& InUserIds
	, const FOnBulkGetPublicUserProfileInfosV2Complete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, UserIds(InUserIds)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserIds count: %d"), UserIds.Num());

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-bulk-get-public-user-profile-infos-v2-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk get public user profile infos V2, identity interface is invalid!"));
		return;
	}

	FUniqueNetIdPtr CurrentUserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));

	if (!CurrentUserId.IsValid())
	{
		ErrorString = TEXT("request-failed-bulk-get-public-user-profile-infos-v2-current-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk get public user profile infos V2, current user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*CurrentUserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-bulk-get-public-user-profile-infos-v2-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk get public user profile infos V2, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	BulkGetPublicUserProfileInfosV2();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::BulkGetPublicUserProfileInfosV2()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserIds count: %d"), UserIds.Num());

	// Create success and error delegates
	OnBulkGetPublicUserProfileInfosV2SuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsPublicUserProfileInfoV2>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::OnBulkGetPublicUserProfileInfosV2Success);
	OnBulkGetPublicUserProfileInfosV2ErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::OnBulkGetPublicUserProfileInfosV2Error);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->BulkGetPublicUserProfileInfosV2(UserIds, OnBulkGetPublicUserProfileInfosV2SuccessDelegate, OnBulkGetPublicUserProfileInfosV2ErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to bulk get public user profile infos V2!"));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::OnBulkGetPublicUserProfileInfosV2Success(const FAccelByteModelsPublicUserProfileInfoV2& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	PublicUserProfileInfoV2 = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::OnBulkGetPublicUserProfileInfosV2Error(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-bulk-get-public-user-profile-infos-v2-error");
	UE_LOG_AB(Warning, TEXT("Failed to bulk get public user profile infos V2! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Note: We could cache this data but bulk profile data is typically used for one-time operations
	// The result is passed directly to delegates

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnBulkGetPublicUserProfileInfosV2CompleteDelegates(LocalUserNum, bWasSuccessful, PublicUserProfileInfoV2, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnBulkGetPublicUserProfileInfosV2CompleteDelegates(LocalUserNum, bWasSuccessful, FAccelByteModelsPublicUserProfileInfoV2{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE