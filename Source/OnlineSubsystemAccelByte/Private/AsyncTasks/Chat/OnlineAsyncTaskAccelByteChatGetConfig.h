// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteChatGetConfig
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteChatGetConfig, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteChatGetConfig(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatGetConfig");
	}

private:

	/**
	 * Delegate handler fired when get chat config succeeded.
	 */
	void OnGetChatConfigSuccess(const FAccelByteModelsChatPublicConfigResponse& Response);

	/**
	 * Delegate handler fired when fail to get the chat config.
	 */
	void OnGetChatConfigError(int32 ErrorCode, const FString& ErrorMessage);

	/**
	 * Struct representing the response model from backend
	 */
	FAccelByteModelsChatPublicConfigResponse ChatPublicConfigResponse{};

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;
};
