// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Task for deleting ttl config of game record
 */
class FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig(FOnlineSubsystemAccelByte* const InABInterface
		, int32 LocalUserNum
		, FString const& InKey);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDeleteGameRecordTTLConfig");
	}

private:

	/**
	 * Delegate handler for when delete ttl config of game record succeed
	 */
	void OnDeleteGameRecordTTLConfigSuccess();
	FVoidHandler OnDeleteGameRecordTTLConfigSuccessDelegate;

	/**
	 * Delegate handler for when delete ttl config of game record fails
	 */
	void OnDeleteGameRecordTTLConfigError(int32 ErrorCode
		, FString const& ErrorMessage);

	FErrorHandler OnDeleteGameRecordTTLConfigErrorDelegate;

	/**
	 * String representing the record key to delete
	 */
	FString Key{};
};