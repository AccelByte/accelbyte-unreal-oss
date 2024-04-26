// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineFriendsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList
	: public FOnlineAsyncTaskAccelByte
, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList(FOnlineSubsystemAccelByte* InABInterfaceS
		, int32 InLocalUserNum
		, const FAccelByteModelsSyncThirdPartyBlockListRequest& Request);
	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncThirdPartyBlockList");
	}

private:
	/**
	 * Request model sent to the backend to configure the block list sync request.
	 */
	FAccelByteModelsSyncThirdPartyBlockListRequest SyncRequest;
	
	/**
	 * Response model from the backend once the block list sync has successfully completed. Contains information on
	 * players that were successfully synced from the platform block list.
	 */
	TArray<FAccelByteModelsSyncThirdPartyBlockListResponse> SyncPlatformResponse;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(SyncThirdPartyBlockList, TArray<FAccelByteModelsSyncThirdPartyBlockListResponse>);

};
