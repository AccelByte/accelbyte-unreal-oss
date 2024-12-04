// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
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
class FOnlineAsyncTaskAccelByteBulkGetUserRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkGetUserRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FString& InKey, const TArray<FUniqueNetIdAccelByteUserRef>& InUniqueNetIds);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBulkGetUserRecord");
	}

private:

	/**
	 * Delegate handler for when get user record succeed
	 */
	void OnGetUserRecordsSuccess(const TArray<FAccelByteModelsUserRecord>& Result);
	THandler<TArray<FAccelByteModelsUserRecord>> OnGetUserRecordsSuccessDelegate;

	/**
	 * Delegate handler for when get user record fails
	 */
	void OnGetUserRecordsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetUserRecordsErrorDelegate;

	TArray<FString> ConvertUniqueNetIdsToAccelByteIds();
	FBulkGetUserRecordMap ConstructBulkGetRecordResponseModel(const TArray<FAccelByteModelsUserRecord>& Result);

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr{};

	/**
	 * String representing the error message that occurred
	 */
	FString ErrorCode{};

	/**
	 * String representing the record key to get
	 */
	FString Key{};

	/*
	 * The UserIds of the record owner
	 */
	TArray<FUniqueNetIdAccelByteUserRef> UniqueNetIds{};

	/*
	 * The map to store temporary user records before saving them locally
	 */
	FBulkGetUserRecordMap UserRecords{};

	/*
	 * Counter to manage the iteration based on total users
	 */
	FThreadSafeCounter RequestCount{0};

	/*
	 * Integer to store the maximum total of users in one request
	 */
	static constexpr int32 MaxUserIdsLimit = 50;
};