// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Task for send ready message to AMS
 */
class FOnlineAsyncTaskAccelByteSendReadyToAMS : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteSendReadyToAMS, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteSendReadyToAMS(FOnlineSubsystemAccelByte* const InABInterface, const FOnRegisterServerComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendReadyToAMS");
	}

private:

	/** Delegate fired after we finish registering server */
	FOnRegisterServerComplete Delegate;

	/**
	 * Delegate handler fired when the AMS websocket successfully connects.
	 */
	void OnAMSConnectSuccess();
	AccelByte::GameServerApi::ServerAMS::FConnectSuccess OnAMSConnectSuccessDelegate;

	/**
	 * Delegate handler fired when we fail to connect to the AMS websocket.
	 */
	void OnAMSConnectError(const FString& ErrorMessage);
	AccelByte::GameServerApi::ServerAMS::FConnectError OnAMSConnectErrorDelegate;

	/** Delegate handler for when receive a lobby disconnected notification. */
	void OnAMSConnectionClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	AccelByte::GameServerApi::ServerAMS::FConnectionClosed OnAMSConnectionClosedDelegate;

	void UnbindDelegates();

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;
};
