// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendFreeFormNotification.h"
#include "OnlineUserInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSendFreeFormNotification::FOnlineAsyncTaskAccelByteSendFreeFormNotification(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InReceiver, const FString& InTopic, const FString& InData)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Receiver(InReceiver)
	, Topic(InTopic)
	, Payload(InData)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteSendFreeFormNotification::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	SendFreeFormNotification();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendFreeFormNotification::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendFreeFormNotification::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// Trigger delegates here

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendFreeFormNotification::SendFreeFormNotification()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Sending notification!"));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	if (UserInterface.IsValid())
	{
		FAccelByteModelsFreeFormNotificationRequest Request;
		Request.Topic = Topic;
		Request.Message = Payload;

		FVoidHandler OnSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendFreeFormNotification::OnSendFreeFormNotificationSuccess);
		FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSendFreeFormNotification::OnSendFreeFormNotificationError);

		API_CLIENT_CHECK_GUARD();
		ApiClient->Lobby.SendNotificationToUser(Receiver, Request, true, OnSuccess, OnError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off the notification!"));
}

void FOnlineAsyncTaskAccelByteSendFreeFormNotification::OnSendFreeFormNotificationSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully notified the user"));
}

void FOnlineAsyncTaskAccelByteSendFreeFormNotification::OnSendFreeFormNotificationError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to send notification to party member(s) as the request to the backend failed! Error Code : %i Error Message : %s"), ErrorCode, *ErrorMessage);
}