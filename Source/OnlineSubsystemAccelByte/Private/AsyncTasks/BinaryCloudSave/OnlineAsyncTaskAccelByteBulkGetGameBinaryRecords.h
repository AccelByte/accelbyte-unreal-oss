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
class FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkGetGameBinaryRecords(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, TArray<FString> const &InKeys);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("OnlineAsyncTaskAccelByteBulkGetGameBinaryRecords");
	}

private:

	/**
	 * Delegate handler for when request succed
	 */
	void OnSuccess(FAccelByteModelsListGameBinaryRecords const& InResult);
	THandler<FAccelByteModelsListGameBinaryRecords> OnSuccessDelegate;

	/**
	 * Delegate handler for when request fail
	 */
	void OnError(int32 Code, FString const& ErrorMessage);
	FErrorHandler OnErrorDelegate;


	TArray<FString> Keys;

	FAccelByteModelsListGameBinaryRecords Result;
};