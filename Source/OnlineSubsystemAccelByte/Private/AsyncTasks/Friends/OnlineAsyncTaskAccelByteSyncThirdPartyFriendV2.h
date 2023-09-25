// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineFriendsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2, ESPMode::ThreadSafe>
{
public:
	/**	Task to sync third party platform friend, it takes third party user ids and send bulk friend request if the user ids is linked to AccelByte */
	FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2(FOnlineSubsystemAccelByte* InABInterface, int32 InLocalUserNum, const FAccelByteModelsSyncThirdPartyFriendsRequest& Request);
	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncThirdPartyFriendV2");
	}

private:
	/**
	 * Our sync request to each platform we want to sync friends.
	 */
	FAccelByteModelsSyncThirdPartyFriendsRequest SyncRequest;
	
	/**
	 * Response of our sync platform operation.
	 * contains success state of each platform we want to sync.
	 */
	TArray<FAccelByteModelsSyncThirdPartyFriendsResponse> SyncPlatformResponse;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(SyncThirdPartyFriends, TArray<FAccelByteModelsSyncThirdPartyFriendsResponse>);

};
