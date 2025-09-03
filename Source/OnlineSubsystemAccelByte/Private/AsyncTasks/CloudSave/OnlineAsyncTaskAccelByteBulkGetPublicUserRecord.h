// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"

/**
 * Task for getting another user record
 */
class FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkGetPublicUserRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, FString const& InKey
		, TArray<FString> const& InUserIds);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBulkGetPublicUserRecords");
	}

private:

	/**
	 * Delegate handler for when get another user record succeed
	 */
	void OnBulkGetPublicUserRecordSuccess(const FListAccelByteModelsUserRecord& Result);
	THandler<FListAccelByteModelsUserRecord> OnBulkGetPublicUserRecordSuccessDelegate;

	/**
	 * Delegate handler for when get another user record fails
	 */
	void OnBulkGetPublicUserRecordError(int32 Code, FString const& ErrorMessage);
	FErrorHandler OnBulkGetPublicUserRecordErrorDelegate;

	/**
	 * String representing the record key to get
	 */
	FString Key;

	/**
	 * String representing another user id
	 */
	TArray<FString> UserIds;

	FListAccelByteModelsUserRecord ListUserRecord;
};