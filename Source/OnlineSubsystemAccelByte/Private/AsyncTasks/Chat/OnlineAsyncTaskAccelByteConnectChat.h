// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteConnectChat
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteConnectChat, ESPMode::ThreadSafe>
{
	public:

	FOnlineAsyncTaskAccelByteConnectChat(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteConnectChat");
	}

private:

	/**
	 * Delegate handler fired when the chat websocket successfully connects.
	 */
	void OnChatConnectSuccess();
	AccelByte::Api::Chat::FChatConnectSuccess OnChatConnectSuccessDelegate;

	/**
	 * Delegate handler fired when we fail to connect to the chat websocket.
	 */
	void OnChatConnectError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnChatConnectErrorDelegate;

	/** Delegate handler for when receive a chat disconnected notification. */
	void OnChatDisconnectedNotif(const FAccelByteModelsChatDisconnectNotif&);
	AccelByte::Api::Chat::FChatDisconnectNotif OnChatDisconnectedNotifDelegate;

	/** Delegate handler for when a chat connection is disconnected. */
	static void OnChatConnectionClosed(int32 StatusCode, const FString& Reason, bool WasClean, int32 InLocalUserNum, const FOnlineIdentityAccelBytePtr IdentityInterface, const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface);

	void UnbindDelegates();

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

};
