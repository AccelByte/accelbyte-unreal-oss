// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatGetConfig.h"

#include "OnlineChatInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteChatGetConfig::FOnlineAsyncTaskAccelByteChatGetConfig(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	ErrorStr = TEXT("");
}

void FOnlineAsyncTaskAccelByteChatGetConfig::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	const AccelByte::THandler<FAccelByteModelsChatPublicConfigResponse> OnGetChatConfigSuccess =
		TDelegateUtils<AccelByte::THandler<FAccelByteModelsChatPublicConfigResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatGetConfig::OnGetChatConfigSuccess);
	const AccelByte::FErrorHandler OnGetChatConfigFail =
		TDelegateUtils<AccelByte::FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteChatGetConfig::OnGetChatConfigError);
	API_CLIENT_CHECK_GUARD(ErrorStr);
	ApiClient->Chat.GetChatConfig(OnGetChatConfigSuccess, OnGetChatConfigFail);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatGetConfig::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid() && bWasSuccessful)
	{
		FAccelByteModelsChatGetConfigPayload GetConfigPayload{};
		GetConfigPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatGetConfigPayload>(GetConfigPayload));
	}
}

void FOnlineAsyncTaskAccelByteChatGetConfig::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	Super::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineChatAccelBytePtr ChatInterface = StaticCastSharedPtr<FOnlineChatAccelByte>(SubsystemPin->GetChatInterface());
	if (ChatInterface.IsValid())
	{
		ChatInterface->TriggerOnGetChatConfigCompleteDelegates(LocalUserNum, bWasSuccessful, ChatPublicConfigResponse, ErrorStr);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatGetConfig::OnGetChatConfigSuccess(
	const FAccelByteModelsChatPublicConfigResponse& Response)
{
	TRY_PIN_SUBSYSTEM()

	const FOnlineChatAccelBytePtr ChatInterface = StaticCastSharedPtr<FOnlineChatAccelByte>(SubsystemPin->GetChatInterface());
	if (ChatInterface.IsValid())
	{
		ChatPublicConfigResponse = Response;
		ChatInterface->SetMaxChatMessageLength(ChatPublicConfigResponse.MaxChatMessageLength);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorStr = TEXT("get-chat-config-error");
		UE_LOG_AB(Warning, TEXT("Failed to get chat cofnig! Message: Chat Interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}

void FOnlineAsyncTaskAccelByteChatGetConfig::OnGetChatConfigError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("get-chat-config-error");
	UE_LOG_AB(Warning, TEXT("Failed to get chat config!"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}