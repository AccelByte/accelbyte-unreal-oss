// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for getting game record
 */
class FOnlineAsyncTaskAccelByteGetGameRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetGameRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, bool bInAlwaysRequestToService);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetGameRecord");
	}

private:

	/**
	 * Delegate handler for when get game record succeed
	 */
	void OnGetGameRecordSuccess(const FAccelByteModelsGameRecord& Result);
	THandler<FAccelByteModelsGameRecord> OnGetGameRecordSuccessDelegate;

	/**
	 * Delegate handler for when get game record fails
	 */
	void OnGetGameRecordError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetGameRecordErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FText ErrorStr;
	FString ErrorCode;

	/**
	 * String representing the record key to get
	 */
	FString Key;

	FAccelByteModelsGameRecord GameRecord;
	bool bAlwaysRequestToService;
};