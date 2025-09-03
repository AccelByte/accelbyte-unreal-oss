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
class FOnlineAsyncTaskAccelByteGetUserRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetUserRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, FString const& InKey
		, bool IsPublic
		, FString const& InRecordUserId = TEXT(""));

	virtual void Initialize() override;
	virtual void Finalize() override;
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
	void OnGetUserRecordsError(int32 ErrorCode, FString const& ErrorMessage);
	FErrorHandler OnGetUserRecordsErrorDelegate;

	/**
	 * String representing the record key to get
	 */
	FString Key;

	/*
	 * The UserId of the record owner
	 */
	FString RecordUserId;

	FAccelByteModelsUserRecord UserRecord;

	bool IsPublicRecord;
};