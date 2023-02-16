// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Async task to attempt to delete a friend from the user's friends list on the backend.
 */
class FOnlineAsyncTaskAccelByteDeleteFriend : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteDeleteFriend, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteDeleteFriend(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDeleteFriend");
	}

private:

	/** Id of the friend that we are attempting to delete */
	TSharedRef<const FUniqueNetIdAccelByteUser> FriendId;

	/** Name of the friends list that we are operating on, ignored for now */
	FString ListName;

	/** String representing errors that may have occurred while trying to delete a friend, used by delegate */
	FString ErrorStr;

	/** Delegate handler for when we get a response back for removing a friend */
	void OnUnfriendResponse(const FAccelByteModelsUnfriendResponse& Result);

	/** Delegate handler for when we get a response back for canceling a friend request */
	void OnCancelFriendRequestResponse(const FAccelByteModelsCancelFriendsResponse& Result);

};

