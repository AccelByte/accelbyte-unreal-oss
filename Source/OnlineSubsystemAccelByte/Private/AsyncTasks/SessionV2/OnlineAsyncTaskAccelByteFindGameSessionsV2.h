// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSessionSettings.h"

/**
 * Task to query game sessions on backend.
 */
class FOnlineAsyncTaskAccelByteFindGameSessionsV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteFindGameSessionsV2, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteFindGameSessionsV2(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& InSearchSettings);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteFindGameSessionsV2");
	}

private:
	/** Search settings object for this find sessions call - will also contain the results */
	TSharedRef<FOnlineSessionSearch> SearchSettings;

	/** Structure used to query for sessions on the backend - essentially just a JSON object of the search settings */
	FAccelByteModelsV2GameSessionQuery QueryStruct{};

	/** Amount of session results we want per page */
	const int32 ResultsPerPage = 20;

	/** Amount of sessions that we are trying to fill in the search results */
	int32 ResultsRemaining = 0;

	/**
	 * Query a single page of results. On complete if we need more results and have not reached the end, we will query another page.
	 */
	void QueryResultsPage(int32 Offset);

	THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult> OnQueryGameSessionsSuccessDelegate;
	void OnQueryGameSessionsSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result, int32 LastOffset);

	FErrorHandler OnQueryGameSessionsErrorDelegate;
	void OnQueryGameSessionsError(int32 ErrorCode, const FString& ErrorMessage);

	bool AddVariantDataToQuery(FAccelByteModelsV2GameSessionQuery& Query, const FString& FieldName, const EAccelByteV2SessionQueryComparisonOp& Comparison, const FVariantData& Data) const;
	bool AddArrayParameterToQuery(FAccelByteModelsV2GameSessionQuery& Query, const FString& FieldName, const EAccelByteV2SessionQueryComparisonOp& Comparison, const FVariantData& Data) const;
};
