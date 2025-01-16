// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatUpdateSystemMessages.h"

#include "OnlineChatInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteChatDeleteSystemMessages"

FOnlineAsyncTaskAccelByteChatUpdateSystemMessages::FOnlineAsyncTaskAccelByteChatUpdateSystemMessages(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const TArray<FAccelByteModelsActionUpdateSystemMessage>& InActionUpdateSystemMessages)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, ActionUpdateSystemMessages(InActionUpdateSystemMessages)
	, OnlineError(FOnlineError())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatUpdateSystemMessages::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnUpdateSystemMessagesSuccessDelegate = TDelegateUtils<Api::Chat::FUpdateSystemMessagesResponse>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatUpdateSystemMessages::OnUpdateSystemMessagesSuccess);
	OnUpdateSystemMessagesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteChatUpdateSystemMessages::OnUpdateSystemMessagesError);
	API_CLIENT_CHECK_GUARD(OnlineError);
	ApiClient->Chat.UpdateSystemMessages(ActionUpdateSystemMessages, OnUpdateSystemMessagesSuccessDelegate, OnUpdateSystemMessagesErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatUpdateSystemMessages::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineChatAccelBytePtr ChatInterface;
	if (!ensure(FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface)))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for update system message complete as our chat interface invalid!"));
		return;
	}

	ChatInterface->TriggerOnUpdateSystemMessagesCompleteDelegates(UserId.ToSharedRef().Get(), OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatUpdateSystemMessages::OnUpdateSystemMessagesSuccess(
	const FAccelByteModelsUpdateSystemMessagesResponse& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %d"), *UserId->GetAccelByteId());
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatUpdateSystemMessages::OnUpdateSystemMessagesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %d"), *UserId->GetAccelByteId());
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to update system messages, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
