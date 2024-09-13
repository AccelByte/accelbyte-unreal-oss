// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteValidateUserInput.h"
#include "OnlineUserInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserSystemAccelByte"

FOnlineAsyncTaskAccelByteValidateUserInput::FOnlineAsyncTaskAccelByteValidateUserInput(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FUserInputValidationRequest& InUserInputValidationRequest)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	UserInputValidationRequest(InUserInputValidationRequest),
	LocalUserNum(InLocalUserNum)
{}

void FOnlineAsyncTaskAccelByteValidateUserInput::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize Validate User Input"));
 
	const THandler<FUserInputValidationResponse>& OnLinkOtherPlatformSuccess = TDelegateUtils<THandler<FUserInputValidationResponse>>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteValidateUserInput::HandleSuccess);
	const FErrorHandler& OnLinkOtherPlatformError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteValidateUserInput::HandleError);

	// Ensure to always get api client since the endpoint is able to call without authorization
	if (Subsystem->GetApiClient(LocalUserNum) == nullptr)
	{
		SetApiClient(FMultiRegistry::GetApiClient(FString::Printf(TEXT("%s/%d"), *Subsystem->GetInstanceName().ToString(), LocalUserNum)));
	}
	API_CLIENT_CHECK_GUARD(OnlineError)
	
	ApiClient->User.ValidateUserInput(UserInputValidationRequest, OnLinkOtherPlatformSuccess, OnLinkOtherPlatformError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteValidateUserInput::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TriggerDelegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());  
	if (UserInterface.IsValid())
	{
		UserInterface->TriggerOnValidateUserInputCompleteDelegates(UserInputValidationResponse, bWasSuccessful, OnlineError);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteValidateUserInput::HandleSuccess(const FUserInputValidationResponse& InUserInputValidationResponse)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess")); 

	UserInputValidationResponse = InUserInputValidationResponse;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteValidateUserInput::HandleError(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));
	
	const FString ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest);
	OnlineError =  ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(Message)); 
	const FString ErrorMessage = FString::Printf(TEXT("Error Code: %d; Error Message: %s"), Code, *Message);
	UE_LOG_AB(Warning, TEXT("Failed to validate user input! %s), "), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE