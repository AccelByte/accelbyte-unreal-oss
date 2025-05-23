// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for replacing admin game record
 */
class FOnlineAsyncTaskAccelByteReplaceAdminGameRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteReplaceAdminGameRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteReplaceAdminGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, const FJsonObject& InGameRecordObj, const FTTLConfig& TTLConfig);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReplaceAdminGameRecord");
	}

private:

	/**
	 * Delegate handler for when replace game record succeed
	 */
	void OnReplaceAdminGameRecordSuccess(const FAccelByteModelsAdminGameRecord& Result);
	THandler<FAccelByteModelsAdminGameRecord> OnReplaceAdminGameRecordSuccessDelegate;

	/**
	 * Delegate handler for when get game record fails
	 */
	void OnReplaceAdminGameRecordError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnReplaceAdminGameRecordErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FText ErrorStr;
	FString ErrorCode;

	/**
	 * String representing the record key to get
	 */
	FString Key;

	FJsonObject GameRecordObj;

	FTTLConfig TTLConfig;
};