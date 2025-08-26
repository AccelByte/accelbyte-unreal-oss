// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FAccelByteGetSystemMessageStatsRequest& InOptionalParams);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatQuerySystemMessages");
	}

private:
	/** List of actions to be performed on system messages.*/
	FAccelByteGetSystemMessageStatsRequest OptionalParams;

	/** The system message stats from BE call.*/
	FAccelByteGetSystemMessageStatsResponse SystemMessageStats;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	Api::Chat::FGetSystemMessageStatsResponse OnGetSystemMessagesStatsSuccessDelegate;
	void OnGetSystemMessagesStatsSuccess(const FAccelByteGetSystemMessageStatsResponse& Response);

	FErrorHandler OnGetSystemMessagesStatsErrorDelegate;
	void OnGetSystemMessagesStatsError(int32 ErrorCode, const FString& ErrorMessage);
};
