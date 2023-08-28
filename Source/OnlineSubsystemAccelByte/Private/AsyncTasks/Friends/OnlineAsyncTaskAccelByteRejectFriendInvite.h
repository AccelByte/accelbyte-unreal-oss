// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Async task to attempt to reject a friend request from the backend.
 */
class FOnlineAsyncTaskAccelByteRejectFriendInvite
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRejectFriendInvite, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteRejectFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRejectFriendInvite");
	}

protected:
	
	/** Id of the user that we are rejecting the invite of */
	TSharedRef<const FUniqueNetIdAccelByteUser> FriendId;

	/** Name of the friends list that we are operating on, ignored for now */
	FString ListName;

	/** String representing any error that occurs during this task, passed to delegate */
	FString ErrorStr;

	/** Delegate handler for when the request to reject a friend invite succeeds */
	void OnRejectFriendResponse(const FAccelByteModelsRejectFriendsResponse& Result);

};

