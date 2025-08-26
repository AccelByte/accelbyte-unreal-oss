// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteChatQuerySystemMessages
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteChatQuerySystemMessages, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatQuerySystemMessages(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FQuerySystemMessageOptions& InQueryParams);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatQuerySystemMessages");
	}

private:
	/** List of actions to be performed on system messages.*/
	FQuerySystemMessageOptions QueryParams;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** The result of query system messages. */
	TArray<FSystemMessageNotifMessage> QueryResult;

	Api::Chat::FQuerySystemMessageResponse OnQuerySystemMessagesSuccessDelegate;
	void OnQuerySystemMessagesSuccess(const FAccelByteModelsQuerySystemMessagesResponse& Response);

	FErrorHandler OnQuerySystemMessagesErrorDelegate;
	void OnQuerySystemMessagesError(int32 ErrorCode, const FString& ErrorMessage);
};
