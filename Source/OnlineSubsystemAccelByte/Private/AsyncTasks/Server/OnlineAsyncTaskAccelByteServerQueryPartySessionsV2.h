// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2(FOnlineSubsystemAccelByte* const InABInterface,
		const FAccelByteModelsV2QueryPartiesRequest& InRequest,
		int64 InOffset, int64 InLimit);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteServerQueryPartySessionsV2");
	}

private:
	/** Variables for storing request data. */
	FAccelByteModelsV2QueryPartiesRequest Request;
	int64 Offset;
	int64 Limit;

	/** Variable for storing query result. */
	FAccelByteModelsV2PaginatedPartyQueryResult QueryResult;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handler when query party sessions success. */
	THandler<FAccelByteModelsV2PaginatedPartyQueryResult> OnQuerySuccess;
	void OnQueryPartySessionsSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result);

	/** Handler when query party sessions failed. */
	FErrorHandler OnQueryFailed;
	void OnQueryPartySessionsFailed(int32 ErrorCode, const FString& ErrorMessage);
};
