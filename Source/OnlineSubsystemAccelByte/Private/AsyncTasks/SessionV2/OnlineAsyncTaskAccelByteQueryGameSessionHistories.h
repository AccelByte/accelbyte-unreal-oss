// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineError.h"

/**
 * Task to send a request to backend to query the gane session associated with current user
 */
class FOnlineAsyncTaskAccelByteQueryGameSessionHistories
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryGameSessionHistories, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteQueryGameSessionHistories(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, EAccelByteGeneralSortBy const& InSortBy
		, FPagedQuery const& InPage);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryGameSessionHistories");
	}

private:

	/** Enum used to sort the history result */
	EAccelByteGeneralSortBy SortBy = EAccelByteGeneralSortBy::DESC;

	/** Page Query used to configure how many game session histories the player needs*/
	FPagedQuery PagedQuery;

	/* Success or Failure status code */
	int32 HttpStatus = 0;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};

	/* Variable to receive the game session history result from backend */
	TArray<FAccelByteModelsGameSessionHistoriesData> GameSessionHistoriesResult{};

	/* Variable to store delegate handle when query game session history returns success */
	THandler<FAccelByteModelsGameSessionHistoriesResult> OnQueryGameSessionHistorySuccessDelegate;

	/* Variable to store delegate handle when query game session history returns error/fail */
	FErrorHandler OnQueryGameSessionHistoryErrorDelegate;

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(QueryGameSessionHistory, FAccelByteModelsGameSessionHistoriesResult);

	void QueryGameSessionHistory(const int32& Offset, const int32& Limit);
};

