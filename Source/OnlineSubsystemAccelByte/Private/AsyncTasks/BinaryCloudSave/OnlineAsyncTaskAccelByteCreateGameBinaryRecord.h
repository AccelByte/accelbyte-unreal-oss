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
class FOnlineAsyncTaskAccelByteCreateGameBinaryRecord
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCreateGameBinaryRecord, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCreateGameBinaryRecord(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, FString const& InKey
		, EAccelByteFileType const& InFileType
		, ESetByMetadataRecord const&  InSetBy
		, FTTLConfig const& InTTLConfig);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("OnlineAsyncTaskAccelByteCreateGameBinaryRecord");
	}

private:

	/**
	 * Delegate handler for when saving binary record succed
	 */
	void OnSuccess(FAccelByteModelsBinaryInfo const & InResult);
	THandler<FAccelByteModelsBinaryInfo> OnSuccessDelegate;

	/**
	 * Delegate handler for when saving binary record fails
	 */
	void OnError(int32 Code, FString const& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	FString Key;
	EAccelByteFileType FileType;
	ESetByMetadataRecord SetBy;
	FTTLConfig TTLConfig;

	FAccelByteModelsBinaryInfo Result;
};