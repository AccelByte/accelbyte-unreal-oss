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
	FOnlineAsyncTaskAccelByteBulkGetUserRecord(FOnlineSubsystemAccelByte* const InABInterface
		, FString const& InKey
		, TArray<FUniqueNetIdAccelByteUserRef> const& InUniqueNetIds);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBulkGetUserRecord");
	}

private:

	void BulkGetUserRecords(FString const& InKey);

	/**
	 * Delegate handler for when get user record succeed
	 */
	void OnGetUserRecordsSuccess(TArray<FAccelByteModelsUserRecord> const& Result);
	THandler<TArray<FAccelByteModelsUserRecord>> OnGetUserRecordsSuccessDelegate;

	/**
	 * Delegate handler for when get user record fails
	 */
	void OnGetUserRecordsError(int32 ErrorCode, FString const& ErrorMessage);
	FErrorHandler OnGetUserRecordsErrorDelegate;

	TArray<FString> ConvertUniqueNetIdsToAccelByteIds(int32 Limit);
	FBulkGetUserRecordMap ConstructBulkGetRecordResponseModel(TArray<FAccelByteModelsUserRecord> const& Result);

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

	/**
	 *  
	 */
	int32 Offset = 0;

	/*
	 * Integer to store the maximum total of users in one request
	 */
	static constexpr int32 MaxUserIdsLimit = 50;
};