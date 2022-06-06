// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserModels.h"
#include "Models/AccelByteLobbyModels.h"
#include "Interfaces/OnlineFriendsInterface.h"

/**
 * Task to cancel a friend request to a user
 * Note: This is a custom functionality
 */
class FOnlineAsyncTaskAccelByteRescindFriendInvite : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteRescindFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnSendInviteComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRescindFriendInvite");
	}

private:

	/** Id of the user that we wish to send a friend invite to */
	TSharedRef<const FUniqueNetIdAccelByteUser> FriendId;

	/** Name of the friends list that we wish to query, not used currently */
	FString ListName;

	/** Delegate fired after we finish the task to cancel an invite */
	FOnSendInviteComplete Delegate;

	/** String representing the error that occurred in the request, if one did */
	FString ErrorStr;

	/** Delegate handler for when the request to cancel a friend invite completes */
	void OnCancelFriendInviteResponse(const FAccelByteModelsCancelFriendsResponse& Result);

};

