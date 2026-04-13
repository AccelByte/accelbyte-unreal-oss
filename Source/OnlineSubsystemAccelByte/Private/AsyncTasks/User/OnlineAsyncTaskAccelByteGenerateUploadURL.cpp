// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGenerateUploadURL.h"

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

FOnlineAsyncTaskAccelByteGenerateUploadURL::FOnlineAsyncTaskAccelByteGenerateUploadURL
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FString& InFolder
	, EAccelByteFileType InFileType
	, const FOnGenerateUploadURLComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, Folder(InFolder)
	, FileType(InFileType)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteGenerateUploadURL::Initialize()
{
	TRY_PIN_SUBSYSTEM();
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Folder: %s; FileType: %d"), *Folder, static_cast<int32>(FileType));

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-generate-upload-url-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate upload URL, identity interface is invalid!"));
		return;
	}

	UserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));
	if (!UserId.IsValid())
	{
		ErrorString = TEXT("request-failed-generate-upload-url-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate upload URL, user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-generate-upload-url-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to generate upload URL, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	GenerateUploadURL();
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateUploadURL::GenerateUploadURL()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Folder: %s; FileType: %d"), *Folder, static_cast<int32>(FileType));

	OnGenerateUploadURLSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserProfileUploadURLResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateUploadURL::OnGenerateUploadURLSuccess);
	OnGenerateUploadURLErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGenerateUploadURL::OnGenerateUploadURLError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->GenerateUploadURL(Folder, FileType, OnGenerateUploadURLSuccessDelegate, OnGenerateUploadURLErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to generate upload URL for folder '%s' and file type %d!"), *Folder, static_cast<int32>(FileType));
}

void FOnlineAsyncTaskAccelByteGenerateUploadURL::OnGenerateUploadURLSuccess(const FAccelByteModelsUserProfileUploadURLResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UploadURLResult = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateUploadURL::OnGenerateUploadURLError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-generate-upload-url-error");
	UE_LOG_AB(Warning, TEXT("Failed to generate upload URL! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGenerateUploadURL::Finalize()
{
	TRY_PIN_SUBSYSTEM();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// No caching needed for upload URL results - they are temporary and expire in 10 minutes

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateUploadURL::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnGenerateUploadURLCompleteDelegates(LocalUserNum, bWasSuccessful, UploadURLResult, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnGenerateUploadURLCompleteDelegates(LocalUserNum, bWasSuccessful, FAccelByteModelsUserProfileUploadURLResult{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE