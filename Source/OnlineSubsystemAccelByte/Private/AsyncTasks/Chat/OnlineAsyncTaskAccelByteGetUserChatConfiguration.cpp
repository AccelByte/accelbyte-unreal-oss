// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetUserChatConfiguration.h"

#include "OnlineChatInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetUserChatConfiguration"

FOnlineAsyncTaskAccelByteGetUserChatConfiguration::FOnlineAsyncTaskAccelByteGetUserChatConfiguration(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteGetUserChatConfiguration::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	TRY_PIN_SUBSYSTEM();

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (!IsDS.IsSet())
	{
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::InvalidAuth
			, FString(TEXT("get-user-chat-configuration-failed-user-not-logged-in"))
			, FText::FromString(TEXT("Failed to get user chat configuration, user is not logged in!")));
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user chat configuration, user is not logged in!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (IsDS.GetValue())
	{
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::NotImplemented
			, FString(TEXT("get-user-chat-configuration-not-supported-on-dedicated-server"))
			, FText::FromString(TEXT("Failed to get user chat configuration, operation not supported on dedicated server!")));
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user chat configuration, operation not supported on dedicated server!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	API_FULL_CHECK_GUARD(Chat, OnlineError);
	const auto OnSuccessDelegate = TDelegateUtils<Api::Chat::FGetUserChatConfigurationResponse>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteGetUserChatConfiguration::OnGetUserChatConfigurationSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteGetUserChatConfiguration::OnGetUserChatConfigurationFailed);
	Chat->GetUserChatConfiguration(OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteGetUserChatConfiguration::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid() && bWasSuccessful)
	{
		FAccelByteModelsChatGetUserConfigPayload Payload{};
		Payload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
		Payload.ProfanityDisabled = UserChatConfiguration.Config.ProfanityDisabled;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatGetUserConfigPayload>(Payload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserChatConfiguration::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, bWasSuccessful: %s, ErrorMessage: %s")
		, *UserId->ToDebugString(), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	FOnlineChatAccelBytePtr ChatInterface;
	FOnlineChatAccelByte::GetFromSubsystem(SubsystemPin.Get(),  ChatInterface);
	if (!ChatInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegate for get user chat configuration as our chat interface invalid!"));
		return;
	}

	ChatInterface->TriggerOnGetUserChatConfigurationCompleteDelegates(LocalUserNum, UserChatConfiguration, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserChatConfiguration::OnTaskTimedOut()
{
	Super::OnTaskTimedOut();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(EOnlineErrorResult::RequestFailure))
		, FText::FromString(TEXT("Get user chat configuration task timeout while waiting for response from backend.")));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserChatConfiguration::OnGetUserChatConfigurationSuccess(FAccelByteModelsGetUserChatConfigurationResponse const& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	UserChatConfiguration = Response;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserChatConfiguration::OnGetUserChatConfigurationFailed(const int32 ErrorCode, const FString& ErrorMessage)
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
