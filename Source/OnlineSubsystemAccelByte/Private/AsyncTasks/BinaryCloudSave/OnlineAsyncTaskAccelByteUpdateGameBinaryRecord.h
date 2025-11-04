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
class FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUpdateGameBinaryRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, FString const& InKey
		, EAccelByteFileType InContentType
		, FString const& InFileLocation);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("OnlineAsyncTaskAccelByteUpdateGameBinaryRecord");
	}

private:

	/**
	 * Delegate handler for when saving binary record succed
	 */
	void OnSuccess(FAccelByteModelsGameBinaryRecord const& InResult);
	THandler<FAccelByteModelsGameBinaryRecord> OnSuccessDelegate;

	/**
	 * Delegate handler for when saving binary record fails
	 */
	void OnError(int32 Code, FString const& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	FString Key;
	EAccelByteFileType ContentType;
	FString FileLocation;

	FAccelByteModelsGameBinaryRecord Result;
};