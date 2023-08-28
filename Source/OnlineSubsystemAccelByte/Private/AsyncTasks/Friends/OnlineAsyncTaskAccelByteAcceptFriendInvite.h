// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Async task to attempt to accept a friend request from a user on the backend.
 */
class FOnlineAsyncTaskAccelByteAcceptFriendInvite
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteAcceptFriendInvite, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteAcceptFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnAcceptInviteComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteAcceptFriendInvite");
	}

private:

	/** Id of the user that we want to accept an invite from */
	TSharedRef<const FUniqueNetIdAccelByteUser> FriendId;

	/** Name of the friends list that we are operating on, not used currently */
	FString ListName;

	/** Friend object that we are operating on for this invite */
	TSharedPtr<FOnlineFriend> InviteeFriend;

	/** Delegate fired after the request to accept the invite completes */
	FOnAcceptInviteComplete Delegate;

	/** String representing an error if one occurs, passed to delegate when complete */
	FString ErrorStr;
	
	/** Delegate handler for when the call to accept a friend request completes */
	void OnAcceptFriendResponseDelegate(const FAccelByteModelsAcceptFriendsResponse& Result);

	void OnGetUserPresenceComplete(const FUniqueNetId& TargetUserId, const bool bGetPresenceSuccess);
};

