// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatQuerySystemMessages.h"

#include "OnlineChatInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteChatQuerySystemMessages"

FOnlineAsyncTaskAccelByteChatQuerySystemMessages::FOnlineAsyncTaskAccelByteChatQuerySystemMessages(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FQuerySystemMessageOptions& InQueryParams)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, QueryParams(InQueryParams)
	, OnlineError(FOnlineError())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatQuerySystemMessages::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnQuerySystemMessagesSuccessDelegate = TDelegateUtils<Api::Chat::FQuerySystemMessageResponse>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatQuerySystemMessages::OnQuerySystemMessagesSuccess);
	OnQuerySystemMessagesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatQuerySystemMessages::OnQuerySystemMessagesError);
	API_FULL_CHECK_GUARD(Chat, OnlineError);
	Chat->QuerySystemMessage(OnQuerySystemMessagesSuccessDelegate, OnQuerySystemMessagesErrorDelegate, QueryParams);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQuerySystemMessages::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for query system message complete as our chat interface invalid!"));
		return;
	}

	ChatInterface->TriggerOnQuerySystemMessageCompleteDelegates(UserId.ToSharedRef().Get(), QueryResult, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQuerySystemMessages::OnQuerySystemMessagesSuccess(
	const FAccelByteModelsQuerySystemMessagesResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %d"), *UserId->GetAccelByteId());

	for (const auto& MessagesResponse : Response.Data)
	{
		if (MessagesResponse.IsTransientSystemMessage())
		{
			continue;
		}

		FSystemMessageNotifMessage SystemMessage;
		MessagesResponse.GetSystemMessageData(SystemMessage);
		QueryResult.Add(SystemMessage);
	}

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQuerySystemMessages::OnQuerySystemMessagesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %d"), *UserId->GetAccelByteId());
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to query system messages, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
