// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for replacing game record
 */
class FOnlineAsyncTaskAccelByteReplaceGameRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteReplaceGameRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteReplaceGameRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, FString const& InKey
		, ESetByMetadataRecord SetBy
		, FJsonObject const& InGameRecordObj
		, FTTLConfig const& TTLConfig);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReplaceGameRecord");
	}

private:

	/**
	 * Delegate handler for when replace game record succeed
	 */
	void OnReplaceGameRecordSuccess();
	FVoidHandler OnReplaceGameRecordSuccessDelegate;

	/**
	 * Delegate handler for when get game record fails
	 */
	void OnReplaceGameRecordError(int32 ErrorCode
		, FString const& ErrorMessage);
	FErrorHandler OnReplaceGameRecordErrorDelegate;

	/**
	 * String representing the record key to get
	 */
	FString Key;

	ESetByMetadataRecord SetBy;

	FJsonObject GameRecordObj;

	FTTLConfig TTLConfig;
};