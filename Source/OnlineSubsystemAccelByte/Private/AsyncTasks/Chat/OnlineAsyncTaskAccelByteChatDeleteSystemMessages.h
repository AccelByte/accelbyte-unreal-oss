// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteChatDeleteSystemMessages
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteChatDeleteSystemMessages, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatDeleteSystemMessages(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const TSet<FString>& InMessageIds);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatDeleteSystemMessages");
	}

private:
	/** Message ids to delete.*/
	TSet<FString> MessageIds;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	Api::Chat::FDeleteSystemMessagesResponse OnDeleteSystemMessagesSuccessDelegate;
	void OnDeleteSystemMessagesSuccess(const FAccelByteModelsDeleteSystemMessagesResponse& Response);

	FErrorHandler OnDeleteSystemMessagesErrorDelegate;
	void OnDeleteSystemMessagesError(int32 ErrorCode, const FString& ErrorMessage);
};
