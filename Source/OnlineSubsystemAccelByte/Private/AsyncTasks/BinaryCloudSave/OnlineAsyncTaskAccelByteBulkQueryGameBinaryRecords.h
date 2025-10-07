// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"

/**
 * Task for saving binary record
 */
class FOnlineAsyncTaskAccelByteBulkQueryGameBinaryRecords
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkQueryGameBinaryRecords, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkQueryGameBinaryRecords(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, FString const &InQuery
		, int32 const& InOffset
		, int32 const& InLimit);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("OnlineAsyncTaskAccelByteBulkQueryGameBinaryRecords");
	}

private:

	/**
	 * Delegate handler for when request succed
	 */
	void OnSuccess(FAccelByteModelsPaginatedGameBinaryRecords const& InResult);
	THandler<FAccelByteModelsPaginatedGameBinaryRecords> OnSuccessDelegate;

	/**
	 * Delegate handler for when request fail
	 */
	void OnError(int32 Code, FString const& ErrorMessage);
	FErrorHandler OnErrorDelegate;


	FString Query;
	int32 Offset;
	int32 Limit;

	FAccelByteModelsPaginatedGameBinaryRecords Result;
};