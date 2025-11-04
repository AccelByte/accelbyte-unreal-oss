// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

/**
 * Declares two methods for SDK delegate handlers. Use in a header file to define these for the task, and then use
 * AB_ASYNC_TASK_DEFINE_SDK_DELEGATES to define the bindings to these delegates in the async task logic.
 *
 * @param Verb Action that this delegate is handling
 */
#define AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(Verb) void On##Verb##Success(); \
	void On##Verb##Error(int32 ErrorCode, const FString& ErrorMessage);

/**
 * Declares two methods for SDK delegate handlers. Use in a header file to define these for the task, and then use
 * AB_ASYNC_TASK_DEFINE_SDK_DELEGATES to define the bindings to these delegates in the async task logic.
 * 
 * @param Verb Action that this delegate is handling
 * @param SuccessType Type that the success delegate will recieve
 */
#define AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(Verb, SuccessType) void On##Verb##Success(const SuccessType& Result); \
	void On##Verb##Error(int32 ErrorCode, const FString& ErrorMessage);

/**
 * Defines two delegates for use in an SDK call wrapped in an async task. Success delegate will have the name
 * On{Verb}SuccessDelegate, and be bound to the On{Verb}Success method of the class. Error delegate will have
 * the name On{Verb}ErrorDelegate, and be bound to the On{Verb}Error method of the class.
 * 
 * @param AsyncTaskClass Name of the class that we are binding delegate methods to
 * @param Verb Name of the action that is being handled by the two delegates, effects the name of the final delegates
 * @param SuccessType Delegate type for the success delegate
 */
#define AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(AsyncTaskClass, Verb, SuccessType) \
	const SuccessType On##Verb##SuccessDelegate = SuccessType::CreateRaw(this, &AsyncTaskClass::On##Verb##Success); \
	const FErrorHandler On##Verb##ErrorDelegate = FErrorHandler::CreateRaw(this, &AsyncTaskClass::On##Verb##Error);

/**
 * Convenience macro for async tasks to validate the expression evaluates to true, otherwise throwing an InvalidState error in the task.
 * This will also stop execution of the task by returning out.
 * 
 * @param Expression Expression that you want to evaluate to true, or fail the task
 * @param Message Message to log in a trace if the expression is false. This will automatically be wrapped in TEXT macro, so no need to do that manually.
 */
#define AB_ASYNC_TASK_VALIDATE(Expression, Message, ...) if (!(Expression))   \
{                                                                                 \
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT(Message), ##__VA_ARGS__); \
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);                 \
	return;                                                                       \
}

/**
 * Convenience macro to log a request error and fail a task.
 * 
 * @param Message Message to log for the request that failed
 * @param ErrorCode Error code for the request that failed
 * @param ErrorMessage Error message for the request that failed
 */
#define AB_ASYNC_TASK_REQUEST_FAILED(Message, ErrorCode, ErrorMessage, ...) const FString LogString = FString::Printf(TEXT(Message), ##__VA_ARGS__); \
	UE_LOG_AB(Warning, TEXT("%s. Error code: %d; Error message: %s"), *LogString, ErrorCode, *ErrorMessage); \
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

#define ASYNC_TASK_FLAG_BIT(Flag) (1 << static_cast<uint8>(Flag))

#define API_CLIENT_CHECK_GUARD(...) \
if (!IsApiClientValid())\
{\
	RaiseGenericError(__VA_ARGS__);\
	return;\
}\
auto ApiClient = GetApiClientInternal();\

#define API_CHECK_GUARD(ApiName, ...) \
auto ApiName = ApiClient->Get##ApiName##Api().Pin(); \
if(!ApiName.IsValid()) \
{ \
	RaiseGenericError(__VA_ARGS__);\
	return;\
} \

#define API_FULL_CHECK_GUARD(ApiName, ...) \
API_CLIENT_CHECK_GUARD(__VA_ARGS__) \
API_CHECK_GUARD(ApiName, __VA_ARGS__) \

#define SERVER_API_CLIENT_CHECK_GUARD(...) \
FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin(); \
if(!AccelByteInstance.IsValid()) \
{ \
	RaiseGenericServerError(__VA_ARGS__); \
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("AccelByteInstance is invalid")); \
	return;\
} \
\
const AccelByte::FServerApiClientPtr ServerApiClient = AccelByteInstance->GetServerApiClient(); \
if(!ServerApiClient.IsValid()) \
{ \
	RaiseGenericServerError(__VA_ARGS__);\
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("ServerApiClient is invalid")); \
	return;\
} \

#define TRY_PIN_ACCELBYTEINSTANCE() \
FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin(); \
if(!AccelByteInstance.IsValid()) \
{ \
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("AccelByteInstance is invalid")); \
	return; \
} \

#define TRY_PIN_SUBSYSTEM_RAW(bCompleteTaskIfInvalid, .../*returned variable if the SubsystemPin invalid*/) \
auto SubsystemPin = AccelByteSubsystem.Pin(); \
if (!SubsystemPin.IsValid()) \
{ \
	if (!bIsComplete && bCompleteTaskIfInvalid) \
	{ \
		TaskOnlineError = EOnlineErrorResult::MissingInterface; \
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState); \
	} \
	return __VA_ARGS__; \
} \

#define TRY_PIN_SUBSYSTEM(.../*returned variable if the SubsystemPin invalid*/) TRY_PIN_SUBSYSTEM_RAW(true, __VA_ARGS__)

#define TRY_PIN_SUBSYSTEM_CONSTRUCTOR() TRY_PIN_SUBSYSTEM_RAW(false)
