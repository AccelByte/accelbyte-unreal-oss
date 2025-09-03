// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteConnectChat.h"

#include "OnlineChatInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteConnectChat::FOnlineAsyncTaskAccelByteConnectChat(
	FOnlineSubsystemAccelByte* const InABInterface,
	const FUniqueNetId& InLocalUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	ErrorStr = TEXT("");
}

void FOnlineAsyncTaskAccelByteConnectChat::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	// Create delegates for successfully as well as unsuccessfully connecting to the AccelByte lobby websocket
	OnChatConnectSuccessDelegate = TDelegateUtils<AccelByte::Api::Chat::FChatConnectSuccess>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteConnectChat::OnChatConnectSuccess);
	OnChatConnectErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteConnectChat::OnChatConnectError);

	OnChatDisconnectedNotifDelegate = TDelegateUtils<AccelByte::Api::Chat::FChatDisconnectNotif>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteConnectChat::OnChatDisconnectedNotif);
	API_FULL_CHECK_GUARD(Chat, ErrorStr);
	Chat->SetDisconnectNotifDelegate(OnChatDisconnectedNotifDelegate);

	// Send off a request to connect to the lobby websocket, as well as connect our delegates for doing so
	Chat->SetConnectSuccessDelegate(OnChatConnectSuccessDelegate);
	Chat->SetConnectFailedDelegate(OnChatConnectErrorDelegate);
	Chat->Connect();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConnectChat::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid() && bWasSuccessful)
	{
		FAccelByteModelsChatV2ConnectedPayload ConnectedPayload{};
		ConnectedPayload.UserId = UserId.IsValid() ? UserId->GetAccelByteId() : TEXT("");
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2ConnectedPayload>(ConnectedPayload));
	}

	UnbindDelegates();
}

void FOnlineAsyncTaskAccelByteConnectChat::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineChatAccelBytePtr ChatInterface = StaticCastSharedPtr<FOnlineChatAccelByte>(SubsystemPin->GetChatInterface());
	if (ChatInterface.IsValid() && !bWasSuccessful)
	{
		ChatInterface->TriggerOnConnectChatCompleteDelegates(LocalUserNum, bWasSuccessful, *UserId.Get(), ErrorStr);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConnectChat::OnChatConnectSuccess()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Get identity interface and set connected to chat
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("AsyncTask terminated because IdentityInterface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("AsyncTask terminated because user '%s' is not logged in!"), *UserId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(LocalUserNum);

	if (!UserAccount.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("AsyncTask terminated because user account for user '%s' is invalid!"), *UserId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
	UserAccountAccelByte->SetConnectedToChat(true);

	// set chat delegates to interface events AFTER complete the QueryRoom
	const FOnlineChatAccelBytePtr ChatInterface = StaticCastSharedPtr<FOnlineChatAccelByte>(SubsystemPin->GetChatInterface());
	if (ChatInterface.IsValid())
	{
		ChatInterface->QueryRoomAfterChatConnectEstablished(UserId.ToSharedRef().Get());
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConnectChat::OnChatConnectError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("login-failed-chat-connect-error");
	UE_LOG_AB(Warning, TEXT("Failed to connect to the AccelByte chat websocket! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteConnectChat::OnChatDisconnectedNotif(const FAccelByteModelsChatDisconnectNotif& Result)
{
	TRY_PIN_SUBSYSTEM();

	// Update identity interface chat flag
	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (IdentityInterface.IsValid())
	{
		TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(LocalUserNum);

		if (UserAccount.IsValid())
		{
			const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
			UserAccountAccelByte->SetConnectedToChat(false);
		}
	}

	ErrorStr = TEXT("login-failed-chat-connect-error");

	UE_LOG_AB(Warning, TEXT("Chat disconnected. Reason '%s'"), *Result.Message);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteConnectChat::UnbindDelegates()
{
	OnChatConnectSuccessDelegate.Unbind();
	OnChatConnectErrorDelegate.Unbind();
	OnChatDisconnectedNotifDelegate.Unbind();
}
