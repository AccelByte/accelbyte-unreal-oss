// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteLobbyModels.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineUserCacheAccelByte.h"

/**
 * Task to send a friend request to a user
 */
class FOnlineAsyncTaskAccelByteSendFriendInvite
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSendFriendInvite, ESPMode::ThreadSafe>
{
public:

	/**
	 * Constructor to send a friend request using the ID of the user that you want to send the request to
	 */
	FOnlineAsyncTaskAccelByteSendFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnSendInviteComplete& InDelegate);
	
	/**
	 * Constructor to send a friend request using a friend code
	 */
	FOnlineAsyncTaskAccelByteSendFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InFriendCode, const FString& InListName, const FOnSendInviteComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendFriendInvite");
	}

private:

	/** Id of the user that we wish to send a friend invite to */
	TSharedRef<const FUniqueNetIdAccelByteUser> FriendId;

	/**
	 * Optional friend code that we can use instead of ID to send a friend request
	 */
	FString FriendCode;

	/** Name of the friends list that we wish to query, not used currently */
	FString ListName;

	/** Delegate fired after we finish the task to send an invite */
	FOnSendInviteComplete Delegate;

	/** String representing the error that occurred in the request, if one did */
	FString ErrorStr;

	/**
	 * Shared pointer to the friend instance that we are creating for the invite, only gets passed to the friend interface
	 * if the invite was successfully sent, otherwise this just gets cleaned up with the task
	 */
	TSharedPtr<FOnlineFriend> InvitedFriend;
	
	/** Delegate handler for when the call to GetPublicUserProfileByPublicId succeeds, used for friend code invite */
	void OnGetUserByFriendCodeSuccess(const FAccelByteModelsPublicUserProfileInfo& Result);

	/** Delegate handler for when the call to GetPublicUserProfileByPublicId errors out, used for friend code invite */
	void OnGetUserByFriendCodeError(int32 ErrorCode, const FString& ErrorMessage, const FJsonObject& ErrorObject);

	/**
	 * Method to kick off a query about the user we want to send a request to
	 */
	void QueryInvitedFriend(const FString& FriendId);

	/** Delegate handler for when we complete a query for joined party member information */
	void OnQueryInvitedFriendComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

	/** Delegate handler for when the request to send a friend invite success */
	FVoidHandler OnSendFriendRequestSuccessDelegate;
	void OnSendFriendRequestSuccess();

	/** Delegate handler for when the request to send a friend invite failed */
	FErrorHandler OnSendFriendRequestFailedDelegate;
	void OnSendFriendRequestError(int32 ErrorCode, const FString& ErrorMessage);

};

