// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteChatUpdateSystemMessages
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteChatUpdateSystemMessages, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatUpdateSystemMessages(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const TArray<FAccelByteModelsActionUpdateSystemMessage>& InActionUpdateSystemMessages);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatUpdateSystemMessages");
	}

private:
	/** List of actions to be performed on system messages.*/
	TArray<FAccelByteModelsActionUpdateSystemMessage> ActionUpdateSystemMessages;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	Api::Chat::FUpdateSystemMessagesResponse OnUpdateSystemMessagesSuccessDelegate;
	void OnUpdateSystemMessagesSuccess(const FAccelByteModelsUpdateSystemMessagesResponse& Response);

	FErrorHandler OnUpdateSystemMessagesErrorDelegate;
	void OnUpdateSystemMessagesError(int32 ErrorCode, const FString& ErrorMessage);
};
