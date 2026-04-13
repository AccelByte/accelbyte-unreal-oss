// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGenerateUploadURLForUserContent.h"

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

FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FString& InUserId
	, EAccelByteFileType InFileType
	, EAccelByteUploadCategory InCategory
	, const FOnGenerateUploadURLForUserContentComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, UserId(InUserId)
	, FileType(InFileType)
	, Category(InCategory)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileType: %d; Category: %d"), *UserId, static_cast<int32>(FileType), static_cast<int32>(Category));

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-generate-upload-url-for-user-content-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate upload URL for user content, identity interface is invalid!"));
		return;
	}

	FUniqueNetIdPtr CurrentUserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));

	if (!CurrentUserId.IsValid())
	{
		ErrorString = TEXT("request-failed-generate-upload-url-for-user-content-current-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate upload URL for user content, current user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*CurrentUserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-generate-upload-url-for-user-content-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate upload URL for user content, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	GenerateUploadURLForUserContent();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::GenerateUploadURLForUserContent()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileType: %d; Category: %d"), *UserId, static_cast<int32>(FileType), static_cast<int32>(Category));

	// Create success and error delegates
	OnGenerateUploadURLForUserContentSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserProfileUploadURLResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::OnGenerateUploadURLForUserContentSuccess);
	OnGenerateUploadURLForUserContentErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::OnGenerateUploadURLForUserContentError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->GenerateUploadURLForUserContent(UserId, FileType, OnGenerateUploadURLForUserContentSuccessDelegate, OnGenerateUploadURLForUserContentErrorDelegate, Category);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to generate upload URL for user content for UserId '%s'!"), *UserId);
}

void FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::OnGenerateUploadURLForUserContentSuccess(const FAccelByteModelsUserProfileUploadURLResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	UploadURLResult = Result;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::OnGenerateUploadURLForUserContentError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-generate-upload-url-for-user-content-error");
	UE_LOG_AB(Warning, TEXT("Failed to generate upload URL for user content! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Note: We don't cache this data as upload URLs are temporary and category-specific
	// The result is passed directly to delegates

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnGenerateUploadURLForUserContentCompleteDelegates(LocalUserNum, bWasSuccessful, UploadURLResult, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnGenerateUploadURLForUserContentCompleteDelegates(LocalUserNum, bWasSuccessful, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE