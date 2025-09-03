// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for deleting user record
 */
class FOnlineAsyncTaskAccelByteDeleteUserRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteDeleteUserRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteDeleteUserRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 LocalUserNum
		, FString const& InKey
		, FString const& InTargetUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDeleteUserRecord");
	}

private:

	/**
	 * Delegate handler for when delete user record succeed
	 */
	void OnDeleteUserRecordSuccess();
	FVoidHandler OnDeleteUserRecordSuccessDelegate;

	/**
	 * Delegate handler for when delete user record fails
	 */
	void OnDeleteUserRecordError(int32 ErrorCode
		, FString const& ErrorMessage);

	FErrorHandler OnDeleteUserRecordErrorDelegate;

	/**
	 * String representing the record key to delete
	 */
	FString Key{};

	/**
	 * Target AccelByte User Id for server to call the request
	 */
	FString TargetUserId{};
};