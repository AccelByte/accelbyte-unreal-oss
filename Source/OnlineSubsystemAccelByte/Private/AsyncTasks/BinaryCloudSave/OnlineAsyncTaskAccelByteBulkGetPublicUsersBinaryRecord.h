// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineBinaryCloudSaveInterfaceAccelByte.h"

/**
 * Task for saving binary record
 */
class FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, FString const &InKey
		, TArray<FString> const& InUserIds);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("OnlineAsyncTaskAccelByteBulkGetPublicUsersBinaryRecord");
	}

private:

	/**
	 * Delegate handler for when saving binary record succed
	 */
	void OnSuccess(FAccelByteModelsListUserBinaryRecords const& result);
	THandler<FAccelByteModelsListUserBinaryRecords> OnSuccessDelegate;

	/**
	 * Delegate handler for when saving binary record fails
	 */
	void OnError(int32 Code, FString const& ErrorMessage);
	FErrorHandler OnErrorDelegate;


	FString Key;
	TArray<FString> TargetUserIds;

	FAccelByteModelsListUserBinaryRecords UserRecords;
};