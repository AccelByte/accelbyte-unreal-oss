// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatGetSystemMessagesStats.h"

#include "OnlineChatInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats"

FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats::FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FAccelByteGetSystemMessageStatsRequest& InOptionalParams)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, OptionalParams(InOptionalParams)
	, OnlineError(FOnlineError())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnGetSystemMessagesStatsSuccessDelegate = TDelegateUtils<Api::Chat::FGetSystemMessageStatsResponse>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats::OnGetSystemMessagesStatsSuccess);
	OnGetSystemMessagesStatsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats::OnGetSystemMessagesStatsError);
	API_FULL_CHECK_GUARD(Chat, OnlineError);
	Chat->GetSystemMessageStats(OnGetSystemMessagesStatsSuccessDelegate, OnGetSystemMessagesStatsErrorDelegate, OptionalParams);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for get system message stats complete as our chat interface invalid!"));
		return;
	}

	ChatInterface->TriggerOnGetSystemMessageStatsCompleteDelegates(UserId.ToSharedRef().Get(), SystemMessageStats, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats::OnGetSystemMessagesStatsSuccess(
	const FAccelByteGetSystemMessageStatsResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %d"), *UserId->GetAccelByteId());

	SystemMessageStats = Response;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats::OnGetSystemMessagesStatsError(int32 ErrorCode,
	const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %d"), *UserId->GetAccelByteId());
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to query system messages, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
