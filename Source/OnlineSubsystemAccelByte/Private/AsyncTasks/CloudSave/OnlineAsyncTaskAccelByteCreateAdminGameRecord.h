// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for creating admin game record
 */
class FOnlineAsyncTaskAccelByteCreateAdminGameRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCreateAdminGameRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCreateAdminGameRecord(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InKey, const FJsonObject& InGameRecordObj, const TArray<FString> InTags, const FTTLConfig& TTLConfig);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateAdminGameRecord");
	}

private:

	/**
	 * Delegate handler for when create game record succeed
	 */
	void OnCreateAdminGameRecordSuccess(const FAccelByteModelsAdminGameRecord& Result);
	THandler<FAccelByteModelsAdminGameRecord> OnCreateAdminGameRecordSuccessDelegate;

	/**
	 * Delegate handler for when get game record fails
	 */
	void OnCreateAdminGameRecordError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnCreateAdminGameRecordErrorDelegate;

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

	TArray<FString> Tags;
	
	FTTLConfig TTLConfig;
};