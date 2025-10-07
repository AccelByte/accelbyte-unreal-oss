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
class FOnlineAsyncTaskAccelByteBulkGetCurentUserBinaryRecords
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkGetCurentUserBinaryRecords, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkGetCurentUserBinaryRecords(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, TArray<FString> const &InKeys);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("OnlineAsyncTaskAccelByteBulkGetCurentUserBinaryRecords");
	}

private:

	/**
	 * Delegate handler for when request succed
	 */
	void OnSuccess(FAccelByteModelsListUserBinaryRecords const& result);
	THandler<FAccelByteModelsListUserBinaryRecords> OnSuccessDelegate;

	/**
	 * Delegate handler for when request fail
	 */
	void OnError(int32 Code, FString const& ErrorMessage);
	FErrorHandler OnErrorDelegate;


	TArray<FString> Keys;

	FAccelByteModelsListUserBinaryRecords UserRecords;
};