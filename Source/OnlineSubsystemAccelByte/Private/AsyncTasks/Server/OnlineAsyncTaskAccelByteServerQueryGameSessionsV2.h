// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2  : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteServerQueryGameSessionsV2(FOnlineSubsystemAccelByte* const InABInterface,
		const FAccelByteModelsV2ServerQueryGameSessionsRequest& InRequest,
		int64 InOffset, int64 InLimit);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteServerQueryGameSessionV2");
	}

private:
	/** Variables for storing request data. */
	FAccelByteModelsV2ServerQueryGameSessionsRequest Request;
	int64 Offset;
	int64 Limit;

	/** Variable for storing query result. */
	FAccelByteModelsV2PaginatedGameSessionQueryResult QueryResult;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handler when query game sessions success. */
	void OnQueryGameSessionsSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result);

	/** Handler when query game sessions failed. */
	void OnQueryGameSessionsFailed(int32 ErrorCode, const FString& ErrorMessage);
};
