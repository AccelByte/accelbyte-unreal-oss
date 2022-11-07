// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for getting user record
 */
class FOnlineAsyncTaskAccelByteGetUserRecord : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteGetUserRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, bool IsPublic);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetUserRecords");
	}

private:

	/**
	 * Delegate handler for when get user record succeed
	 */
	void OnGetUserRecordsSuccess(const FAccelByteModelsUserRecord& Result);
	THandler<FAccelByteModelsUserRecord> OnGetUserRecordsSuccessDelegate;

	/**
	 * Delegate handler for when get user record fails
	 */
	void OnGetUserRecordsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetUserRecordsErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	/**
	 * String representing the record key to get
	 */
	FString Key;

	FAccelByteModelsUserRecord UserRecord;

	bool IsPublicRecord;
};