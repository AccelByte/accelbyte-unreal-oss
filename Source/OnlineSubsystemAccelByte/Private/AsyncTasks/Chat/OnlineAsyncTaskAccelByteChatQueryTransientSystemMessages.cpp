// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatQueryTransientSystemMessages.h"

#include "OnlineChatInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages"

FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages::FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FQuerySystemMessageOptions& InQueryParams)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, QueryParams(InQueryParams)
	, OnlineError(FOnlineError())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnQuerySystemMessagesSuccessDelegate = TDelegateUtils<Api::Chat::FQuerySystemMessageResponse>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages::OnQuerySystemMessagesSuccess);
	OnQuerySystemMessagesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages::OnQuerySystemMessagesError);
	API_FULL_CHECK_GUARD(Chat, OnlineError);
	Chat->QuerySystemMessage(OnQuerySystemMessagesSuccessDelegate, OnQuerySystemMessagesErrorDelegate, QueryParams);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for query system message complete as our chat interface invalid!"));
		return;
	}

	ChatInterface->TriggerOnQueryTransientSystemMessageCompleteDelegates(UserId.ToSharedRef().Get(), QueryResult, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages::OnQuerySystemMessagesSuccess(const FAccelByteModelsQuerySystemMessagesResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());

	for (const auto& MessagesResponse : Response.Data)
	{
		if (MessagesResponse.IsTransientSystemMessage())
		{
			QueryResult.Add(MessagesResponse);
		}
	}

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages::OnQuerySystemMessagesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to query system messages, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
