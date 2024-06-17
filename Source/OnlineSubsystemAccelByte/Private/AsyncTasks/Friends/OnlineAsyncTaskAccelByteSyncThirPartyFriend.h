// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineFriendsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncThirPartyFriend
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncThirPartyFriend, ESPMode::ThreadSafe>
{
public:
	/**	Task to sync third party platform friend, it takes third party user ids and send bulk friend request if the user ids is linked to AccelByte */
	FOnlineAsyncTaskAccelByteSyncThirPartyFriend(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InNativeFriendListName, const FString& InAccelByteFriendListName);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncThirPartyFriend");
	}

private:
	IOnlineSubsystem* NativeSubSystem = nullptr;

	/** Name of friend list on third party platform to sync*/
	FString NativeFriendListName;

	/** Name of friend list on third party platform to sync*/
	FString AccelByteFriendListName;

	/**
	 * Array of shared pointer to the friends instance that we are synced to,
	 * only gets passed to the friend interface if the sync was successful,
	 * otherwise this just gets cleaned up with the task
	 */
	TArray<TSharedPtr<FOnlineFriend>> SyncedFriends;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Function to query native friend list based on current platform. */
	bool QueryNativeFriendList();

	/** Handler for completed native friend query */
	void OnReadNativeFriendListComplete(int32 InLocalUserNum, bool bInWasSuccessful, const FString& InListName, const FString& ErrorStr);

	/** Handler for successful bulk query of AccelByte user id based on 3rd party user id. */
	void OnBulkGetUserByOtherPlatformUserIdsSuccess(const FBulkPlatformUserIdResponse& Result);

	/** Handler for failed bulk query of AccelByte user id based on 3rd party user id. */
	void OnBulkGetUserByOtherPlatformUserIdsError(int32 ErrorCode, const FString& ErrorMessage);

	/** Handler for completed a query for joined party member information */
	void OnQuerySyncedFriendComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried);

	/** Handler for successful adding 3rd party platform friend to current user. */
	void OnBulkFriendRequestSuccess();

	/** Handler for failed adding 3rd party platform friend to current user. */
	void OnBulkFriendRequestError(int32 ErrorCode, const FString& ErrorMessage);
};
