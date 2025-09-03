// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for replacing user record
 */
class FOnlineAsyncTaskAccelByteBulkReplaceUserRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkReplaceUserRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkReplaceUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FString& InKey, const TMap <FUniqueNetIdAccelByteUserRef, FJsonObject>& InRequest);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBulkReplaceUserRecord");
	}

private:

	/**
	 * Delegate handler for when replace user record succeed
	 */
	void OnReplaceUserRecordsSuccess(const TArray<FAccelByteModelsBulkReplaceUserRecordResponse>& InResponse);
	THandler<TArray<FAccelByteModelsBulkReplaceUserRecordResponse>> OnReplaceUserRecordSuccessDelegate;

	/**
	 * Delegate handler for when replace user record fails
	 */
	void OnReplaceUserRecordsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnReplaceUserRecordErrorDelegate;

	FAccelByteModelsBulkReplaceUserRecordRequest ConstructBulkReplaceRequestModel();
	FBulkReplaceUserRecordMap ConstructBulkReplaceResponseModel(const TArray<FAccelByteModelsBulkReplaceUserRecordResponse>& Response);

	/**
	 * String representing the record key to delete
	 */
	FString Key{};

	/**
	 * The replace request information
	 */
	TMap <FUniqueNetIdAccelByteUserRef, FJsonObject> Request{};

	/*
	 * The map to store temporary user records before saving them locally
	 */
	FBulkReplaceUserRecordMap Result{};

	/*
	 * Counter to manage the iteration based on total users
	 */
	FThreadSafeCounter RequestCount{0};

	/*
	 * Integer to store the maximum total of users in one request
	 */
	static constexpr int32 MaxUserIdsLimit = 50;
};