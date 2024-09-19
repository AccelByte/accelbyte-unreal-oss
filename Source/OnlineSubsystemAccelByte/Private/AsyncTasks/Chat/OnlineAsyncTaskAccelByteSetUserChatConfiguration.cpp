// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSetUserChatConfiguration.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSetUserChatConfiguration"

FOnlineAsyncTaskAccelByteSetUserChatConfiguration::FOnlineAsyncTaskAccelByteSetUserChatConfiguration(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId
	, const FAccelByteModelsSetUserChatConfigurationRequest& InRequest)
	: FOnlineAsyncTaskAccelByte(InABInterface)
		, Request(InRequest)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteSetUserChatConfiguration::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	API_CLIENT_CHECK_GUARD(OnlineError);
	const auto OnSuccessDelegate = TDelegateUtils<Api::Chat::FSetUserChatConfigurationResponse>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteSetUserChatConfiguration::OnSetUserChatConfigurationSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteSetUserChatConfiguration::OnSetUserChatConfigurationFailed);
	ApiClient->Chat.SetUserChatConfiguration(Request, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteSetUserChatConfiguration::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid() && bWasSuccessful)
	{
		FAccelByteModelsChatSetUserConfigPayload Payload{};
		Payload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
		Payload.ProfanityDisabled = Request.Config.ProfanityDisabled;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatSetUserConfigPayload>(Payload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSetUserChatConfiguration::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, bWasSuccessful: %s, ErrorMessage: %s")
		, *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineChatAccelBytePtr ChatInterface;
	FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(), ChatInterface);
	if (!ChatInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for set user chat configuration as our chat interface invalid!"));
		return;
	}

	ChatInterface->TriggerOnSetUserChatConfigurationCompleteDelegates(LocalUserNum, SetUserChatConfigurationResponse, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSetUserChatConfiguration::OnTaskTimedOut()
{
	Super::OnTaskTimedOut();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(EOnlineErrorResult::RequestFailure))
		, FText::FromString(TEXT("Set user chat configuration task timeout while waiting for response from backend.")));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSetUserChatConfiguration::OnSetUserChatConfigurationSuccess(FAccelByteSetUserChatConfigurationResponse const& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	SetUserChatConfigurationResponse = Response;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSetUserChatConfiguration::OnSetUserChatConfigurationFailed(const int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, ErrorCode: %d, ErrorMessage: %s")
		, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(ErrorCode)
		, FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
