// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for getting admin game record
 */
class FOnlineAsyncTaskAccelByteGetAdminGameRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetAdminGameRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetAdminGameRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, const FString& InKey
		, bool bInAlwaysRequestToService);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetAdminGameRecord");
	}

private:

	/**
	 * Delegate handler for when get game record succeed
	 */
	void OnGetAdminGameRecordSuccess(const FAccelByteModelsAdminGameRecord& Result);
	THandler<FAccelByteModelsAdminGameRecord> OnGetAdminGameRecordSuccessDelegate;

	/**
	 * Delegate handler for when get game record fails
	 */
	void OnGetAdminGameRecordError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetAdminGameRecordErrorDelegate;

	/**
	 * String representing the record key to get
	 */
	FString Key;

	FAccelByteModelsAdminGameRecord GameRecord;
	bool bAlwaysRequestToService;
};