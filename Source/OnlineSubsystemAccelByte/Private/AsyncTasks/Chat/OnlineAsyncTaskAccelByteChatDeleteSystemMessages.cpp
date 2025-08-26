// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatDeleteSystemMessages.h"

#include "OnlineChatInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteChatDeleteSystemMessages"

FOnlineAsyncTaskAccelByteChatDeleteSystemMessages::FOnlineAsyncTaskAccelByteChatDeleteSystemMessages(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const TSet<FString>& InMessageIds)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, MessageIds(InMessageIds)
	, OnlineError(FOnlineError())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatDeleteSystemMessages::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnDeleteSystemMessagesSuccessDelegate = TDelegateUtils<Api::Chat::FDeleteSystemMessagesResponse>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatDeleteSystemMessages::OnDeleteSystemMessagesSuccess);
	OnDeleteSystemMessagesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatDeleteSystemMessages::OnDeleteSystemMessagesError);
	API_FULL_CHECK_GUARD(Chat, OnlineError);
	Chat->DeleteSystemMessages(MessageIds, OnDeleteSystemMessagesSuccessDelegate, OnDeleteSystemMessagesErrorDelegate);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatDeleteSystemMessages::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for delete system message complete as our chat interface invalid!"));
		return;
	}

	ChatInterface->TriggerOnDeleteSystemMessagesCompleteDelegates(UserId.ToSharedRef().Get(), OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatDeleteSystemMessages::OnDeleteSystemMessagesSuccess(const FAccelByteModelsDeleteSystemMessagesResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatDeleteSystemMessages::OnDeleteSystemMessagesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(TEXT("delete-system-messages-failed")));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to delete system messages, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
