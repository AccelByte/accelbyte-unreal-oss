// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Models/AccelByteLobbyModels.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemTypes.h"

/**
 * Async task to try and read the user's friends list from the backend through the Lobby websocket.
 */
class FOnlineAsyncTaskAccelByteReadFriendsList
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteReadFriendsList, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteReadFriendsList(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, const FString& InListName
		, const EInviteStatus::Type& InInviteStatus
		, int32 InOffset
		, int32 InLimit
		, const FOnReadFriendsListComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReadFriendsList");
	}

private:

	/**
	 * Name of the friends list that we wish to query, corresponds to EFriendsLists
	 */
	FString ListName;

	/** String representing an error message that is passed to the delegate, can be blank */
	FString ErrorString;

	/** Delegate that will be fired once we have finished our attempt to get the user's friends list */
	FOnReadFriendsListComplete Delegate;

	/** Whether we have gotten a response back from the backend for querying incoming friend requests */
	FThreadSafeBool bHasReceivedResponseForIncomingFriends;

	/** Whether we have gotten a response back from the backend for querying outgoing friend requests */
	FThreadSafeBool bHasReceivedResponseForOutgoingFriends;

	/** Whether we have gotten a response back from the backend for querying our current friends */
	FThreadSafeBool bHasReceivedResponseForCurrentFriends;

	/** Whether we have already sent a request to get information on all of our friends */
	FThreadSafeBool bHasSentRequestForFriendInformation;

	/** Whether we have gotten a response back from the backend for querying information on all of our friends */
	FThreadSafeBool bHasRecievedAllFriendInformation;
	
	FThreadSafeBool bHasSentRequestForUserStatus;

	FThreadSafeBool bHasReceivedAllUserStatus;

	/** Array of AccelByte IDs that we need to query from backend */
	TArray<FString> FriendIdsToQuery;

	/** Resulting array of friend instances from each query */
	TArray<TSharedPtr<FOnlineFriend>> FoundFriends;

	/** Map of AccelByte IDs to invite status, used to make final friend instance */
	TMap<FString, EInviteStatus::Type> AccelByteIdToFriendStatus;
	
	TMap<FString, FAccelByteModelsUserStatusNotif> AccelByteIdToPresence;

	/** Friend status to query */
	EInviteStatus::Type InviteStatus;

	/** Convenience method for checking in tick whether the task is still waiting on async work from the backend */
	bool HasTaskFinishedAsyncWork();

	/** Delegate handler for when the friends list load has completed */
	int32 QueryFriendListOffset {0};
	int32 QueryFriendListLimit{ 0 };
	void QueryFriendList();
	THandler<FAccelByteModelsQueryFriendListResponse> OnQueryFriendListSuccessDelegate;
	void OnQueryFriendListSuccess(const FAccelByteModelsQueryFriendListResponse& Result);
	FErrorHandler OnQueryFriendListFailedDelegate;
	void OnQueryFriendListFailed(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when the incoming friend request load has completed */
	int32 QueryIncomingFriendReqOffset {0};
	int32 QueryIncomingFriendReqLimit{ 0 };
	void QueryIncomingFriendRequest();
	THandler<FAccelByteModelsIncomingFriendRequests> OnQueryIncomingFriendRequestSuccessDelegate;
	void OnQueryIncomingFriendRequestSuccess(const FAccelByteModelsIncomingFriendRequests& Result);
	FErrorHandler OnQueryIncomingFriendRequestFailedDelegate;
	void OnQueryIncomingFriendRequestFailed(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when the outgoing friend request load has completed */
	int32 QueryOutgoingFriendReqOffset {0};
	int32 QueryOutgoingFriendReqLimit{ 0 };
	void QueryOutgoingFriendRequest();
	THandler<FAccelByteModelsOutgoingFriendRequests> OnQueryOutgoingFriendRequestSuccessDelegate;
	void OnQueryOutgoingFriendRequestSuccess(const FAccelByteModelsOutgoingFriendRequests& Result);
	FErrorHandler OnQueryOutgoingFriendRequestFailedDelegate;
	void OnQueryOutgoingFriendRequestFailed(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when we successfully get all information for each user in our friends list */
	void OnQueryFriendInformationComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried);
	
	void OnGetUserPresenceComplete(const FAccelByteModelsBulkUserStatusNotif& Statuses);
};

