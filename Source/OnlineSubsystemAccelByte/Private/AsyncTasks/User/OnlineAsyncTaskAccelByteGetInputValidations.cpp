// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetInputValidations.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserSystemAccelByte"

FOnlineAsyncTaskAccelByteGetInputValidations::FOnlineAsyncTaskAccelByteGetInputValidations(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& InLanguageCode, bool bInDefaultOnEmpty)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, LanguageCode(InLanguageCode)
	, bDefaultOnEmpty(bInDefaultOnEmpty)
{}

void FOnlineAsyncTaskAccelByteGetInputValidations::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize Get Input Validations"));

	const THandler<FInputValidation>& OnGetInputValidationsSuccess = TDelegateUtils<THandler<FInputValidation>>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGetInputValidations::HandleSuccess);
	const FErrorHandler& OnGetInputValidationsError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteGetInputValidations::HandleError);

	// Ensure to always get api client since the endpoint is able to call without authorization
	if (SubsystemPin->GetApiClient(LocalUserNum) == nullptr)
	{
		TRY_PIN_ACCELBYTEINSTANCE();
		SetApiClient(AccelByteInstance->GetApiClient(FString::Printf(TEXT("%d"), LocalUserNum)));
	}
	API_FULL_CHECK_GUARD(User, OnlineError)

	User->GetInputValidationsByNamespace(LanguageCode, OnGetInputValidationsSuccess, OnGetInputValidationsError, bDefaultOnEmpty);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetInputValidations::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TriggerDelegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		UserInterface->TriggerOnGetInputValidationsCompleteDelegates(InputValidation, bWasSuccessful, OnlineError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetInputValidations::HandleSuccess(const FInputValidation& InInputValidation)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	InputValidation = InInputValidation;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetInputValidations::HandleError(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	const FString ErrorCode = FString::Printf(TEXT("%d"), Code);
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(Message));
	const FString ErrorMessage = FString::Printf(TEXT("Error Code: %d; Error Message: %s"), Code, *Message);
	UE_LOG_AB(Warning, TEXT("Failed to get input validations! %s), "), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
