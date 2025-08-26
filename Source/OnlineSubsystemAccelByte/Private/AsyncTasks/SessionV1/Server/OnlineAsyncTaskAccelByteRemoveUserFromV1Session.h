// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

class FOnlineAsyncTaskAccelByteRemoveUserFromV1Session
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRemoveUserFromV1Session, ESPMode::ThreadSafe>
{
public:

	/** Task to remove user from session in the channel */
	FOnlineAsyncTaskAccelByteRemoveUserFromV1Session(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InChannelName, const FString& InMatchId, const FOnRemoveUserFromSessionComplete& InDelegate = FOnRemoveUserFromSessionComplete());

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRegisterDedicatedSession");
	}

private:

	FString ChannelName;
	FString MatchId;
	FOnRemoveUserFromSessionComplete Delegate;
	FVoidHandler OnRemoveUserFromSessionSuccessDelegate;
	FErrorHandler OnRemoveUserFromSessionErrorDelegate;
	/**
	 * Delegate handler when remove user from session succeed
	 */
	void OnRemoveUserFromSessionSuccess();

	/**
	 * Delegate handler when remove user from session fails
	 */
	void OnRemoveUserFromSessionError(int32 ErrorCode, const FString& ErrorMessage);

	/**
	 * Attempt to authenticate the dedicated server with client credentials
	 */
	void OnAuthenticateServerComplete(bool bAuthenticationSuccessful);

};
