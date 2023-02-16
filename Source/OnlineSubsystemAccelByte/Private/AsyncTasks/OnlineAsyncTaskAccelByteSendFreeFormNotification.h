// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

/**
 * Async task to send free form notification.
 */
class FOnlineAsyncTaskAccelByteSendFreeFormNotification : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteSendFreeFormNotification, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteSendFreeFormNotification(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InReceiver, const FString& InTopic, const FString& InData);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendFreeFormNotification");
	}

private:

	/**
	 * Method that sends off the notification to user in lobby.
	 */
	void SendFreeFormNotification();

	/**
	 * Delegate success handler on sending notification.
	 */
	void OnSendFreeFormNotificationSuccess();

	/**
	 * Delegate error handler on sending notification.
	 */
	void OnSendFreeFormNotificationError(int32 ErrorCode, const FString& ErrorMessage);

	FString Receiver;

	FString Topic;

	FString Payload;
};
