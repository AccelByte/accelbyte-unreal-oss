// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
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
class FOnlineAsyncTaskAccelByteReplaceUserRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteReplaceUserRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteReplaceUserRecord(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InKey, const FJsonObject& InUserRecordObj, bool IsPublic, int32 InLocalUserNum, const FString& InTargetUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReplaceUserRecord");
	}

private:

	/**
	 * Delegate handler for when replace user record succeed
	 */
	void OnReplaceUserRecordsSuccess();
	FVoidHandler OnReplaceUserRecordSuccessDelegate;

	/**
	 * Delegate handler for when replace user record fails
	 */
	void OnReplaceUserRecordsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnReplaceUserRecordErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr{};
	FString ErrorCode{};

	/**
	 * String representing the record key to delete
	 */
	FString Key{};

	/**
	 * the Record can only be set by either CLIENT or SERVER
	 */
	FString SetBy{};

	FJsonObject UserRecordObj{};

	bool IsPublicRecord{false};

	/**
	 * Target AccelByte User Id for server to call the request
	 */
	FString TargetUserId{};

	/**
	 * Current Local User Number
	 */
	int32 LocalUserNum = 0;
};