// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetUserPlatformLinks.h"
#include "OnlineUserInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetUserPlatformLinks"

FOnlineAsyncTaskAccelByteGetUserPlatformLinks::FOnlineAsyncTaskAccelByteGetUserPlatformLinks(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	const int32 InLocalUserNum)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteGetUserPlatformLinks::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);


	OnSuccessDelegate = TDelegateUtils<THandler<FPagedPlatformLinks>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserPlatformLinks::OnGetUserPlatformLinksSuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserPlatformLinks::OnGetUserPlatformLinksError);

	API_FULL_CHECK_GUARD(User, ErrorString);
	User->GetPlatformLinks(OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteGetUserPlatformLinks::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (!UserInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("[Error] Failed to process the request! User interface is not valid!"));

		return;
	}

	auto Error = bWasSuccessful ?
		ONLINE_ERROR(EOnlineErrorResult::Success) :
		ONLINE_ERROR(EOnlineErrorResult::RequestFailure
			, FString::Printf(TEXT("%d"), HttpStatus)
			, FText::FromString(ErrorString));

	UserInterface->TriggerOnGetUserPlatformLinksCompleteDelegates(LocalUserNum
		, bWasSuccessful
		, AccelByteModelsUserPlatformLinks
		, Error);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserPlatformLinks::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (bWasSuccessful == false)
	{
		UE_LOG_AB(Warning, TEXT("[Error] Failed to cache the user 3rd party platform information because the task return false!"));
		return;
	}

	TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (!UserInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("[Error] Failed to cache the user 3rd party platform information because the user interface is not valid!"));
		return;
	}

	UserInterface->AddNewLinkedUserAccountToCache(UserId.ToSharedRef(),  AccelByteModelsUserPlatformLinks);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserPlatformLinks::OnGetUserPlatformLinksSuccess(const FPagedPlatformLinks& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	HttpStatus = static_cast<int32>(ErrorCodes::StatusOk);

	if (Result.Data.Num() > 0)
	{
		AccelByteModelsUserPlatformLinks = Result.Data;
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteGetUserPlatformLinks::OnGetUserPlatformLinksError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	HttpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE