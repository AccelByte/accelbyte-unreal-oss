// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGenerateCodeForPublisherToken.h"
#include "OnlineIdentityInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskGenerateCodeForPublisherToken"


FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken::FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& InPublisherClientID, const FGenerateCodeForPublisherTokenComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum, true),
	PublisherClientID(InPublisherClientID),
	Delegate(InDelegate)
{}

void FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize()")); 

	THandler<FCodeForTokenExchangeResponse> OnSuccess = TDelegateUtils<THandler<FCodeForTokenExchangeResponse>>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken::HandleSuccess);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken::HandleError);

	API_FULL_CHECK_GUARD(User, OnlineError);
	User->GenerateCodeForPublisherTokenExchange(PublisherClientID, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TriggerDelegates"));
	Delegate.ExecuteIfBound(bWasSuccessful, Result);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken::HandleSuccess(const FCodeForTokenExchangeResponse& InResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	Result = InResult;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteGenerateCodeForPublisherToken::HandleError(int32 ErrorCode, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError")); 	 
	
	const FString ErrorCodeString = FString::Printf(TEXT("%d"), ErrorCode);
	OnlineError =  ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCodeString, FText::FromString(Message));
	const FString ErrorMessage = FString::Printf(TEXT("Error Code: %d; Error Message: %s"), ErrorCode, *Message);
	UE_LOG_AB(Warning, TEXT("Failed to generate code for publisher namespace exchange! %s), "), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE