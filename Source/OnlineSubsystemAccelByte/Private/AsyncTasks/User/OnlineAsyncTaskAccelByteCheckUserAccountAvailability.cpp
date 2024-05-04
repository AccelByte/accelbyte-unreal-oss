// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCheckUserAccountAvailability.h" 

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteCheckUserAccountAvailability::FOnlineAsyncTaskAccelByteCheckUserAccountAvailability(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InDisplayName, bool InSearchUniqueDisplayName)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, DisplayName(InDisplayName)
	, bIsSearchUniqueDisplayName(InSearchUniqueDisplayName)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));
	
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	bUserExisted = false; 
	ErrorCode = 0;
    ErrorMessage = TEXT("");
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteCheckUserAccountAvailability::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	const FVoidHandler& OnSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteCheckUserAccountAvailability::HandleSuccess);
	const FErrorHandler& OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteCheckUserAccountAvailability::HandleError);
	API_CLIENT_CHECK_GUARD(ErrorMessage);
	ApiClient->User.CheckUserAccountAvailability(DisplayName, OnSuccess, OnError, bIsSearchUniqueDisplayName);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteCheckUserAccountAvailability::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	
	Super::TriggerDelegates();
	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	if (UserInterface.IsValid())
	{
		const FOnlineErrorAccelByte OnlineError = bWasSuccessful ? ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success) :
			ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorMessage));
		UserInterface->TriggerOnCheckUserAccountAvailabilityCompleteDelegates(bWasSuccessful, bUserExisted, OnlineError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckUserAccountAvailability::HandleSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	bUserExisted = true;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCheckUserAccountAvailability::HandleError(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	// We want the expected error (ErrorCode 404) to be set as success error response
	// while user availability checking is identified by variable bUserExisted, that is set false
	if (Code == EHttpResponseCodes::NotFound)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		ErrorCode = Code;
		ErrorMessage = Message; 
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE