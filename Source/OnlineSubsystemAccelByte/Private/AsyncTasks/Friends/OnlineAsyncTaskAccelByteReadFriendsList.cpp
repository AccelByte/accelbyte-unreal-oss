// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReadFriendsList.h"

#include "OnlinePresenceInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"
#include "Api/AccelByteUserApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteReadFriendsList::FOnlineAsyncTaskAccelByteReadFriendsList(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, const FString& InListName
	, const EInviteStatus::Type& InInviteStatus
	, int32 InOffset
	, int32 InLimit
	, const FOnReadFriendsListComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, ListName(InListName)
	, Delegate(InDelegate)
	, InviteStatus(InInviteStatus)
	, QueryFriendListOffset(InOffset)
	, QueryFriendListLimit(InLimit)
	, QueryIncomingFriendReqOffset(InOffset)
	, QueryIncomingFriendReqLimit(InLimit)
	, QueryOutgoingFriendReqOffset(InOffset)
	, QueryOutgoingFriendReqLimit(InLimit)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	// Since we will want to be able to operate on users that we have sent an invite to, as well as users that have
	// sent invites to us, then we need to not only query the current accepted friends list, but also the outgoing
	// and incoming friends lists. Set up all the delegates for these queries.
	if (InviteStatus == EInviteStatus::Accepted || InviteStatus == EInviteStatus::Unknown)
	{
		QueryFriendList();
	}
	else
	{
		bHasReceivedResponseForCurrentFriends = true;
	}
	if (InviteStatus == EInviteStatus::PendingInbound || InviteStatus == EInviteStatus::Unknown)
	{
		QueryIncomingFriendRequest();
	}
	else
	{
		bHasReceivedResponseForIncomingFriends = true;
	}
	if (InviteStatus == EInviteStatus::PendingOutbound || InviteStatus == EInviteStatus::Unknown)
	{
		QueryOutgoingFriendRequest();
	}
	else
	{
		bHasReceivedResponseForOutgoingFriends = true;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadFriendsList::Tick()
{
	Super::Tick();

	if (bHasReceivedResponseForCurrentFriends && bHasReceivedResponseForIncomingFriends && bHasReceivedResponseForOutgoingFriends && (!bHasSentRequestForUserStatus && !bHasReceivedAllUserStatus))
	{
		SetLastUpdateTimeToCurrentTime();//Increase this AsyncTask lifespan to do the initial BulkGetUserPresence operation

		if (FriendIdsToQuery.Num() > 0)
		{
			API_FULL_CHECK_GUARD(Lobby, ErrorString);
			Lobby->BulkGetUserPresenceV2(FriendIdsToQuery,
				TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnGetUserPresenceComplete),
				FErrorHandler::CreateLambda([this](int32 Code, FString const& ErrMsg)
					{
						AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query friends presence"));
						CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
					})
			);
		}
		else
		{
			OnGetUserPresenceComplete(FAccelByteModelsBulkUserStatusNotif{});
		}
		bHasSentRequestForUserStatus = true;
	}
	// If we have received all friend IDs from each list, but we haven't tried to get information on those friends yet, send the request out
	if (bHasReceivedResponseForCurrentFriends && bHasReceivedResponseForIncomingFriends && bHasReceivedResponseForOutgoingFriends && bHasReceivedAllUserStatus && (!bHasSentRequestForFriendInformation && !bHasRecievedAllFriendInformation))
	{
		TRY_PIN_SUBSYSTEM();

		SetLastUpdateTimeToCurrentTime();//Increase this AsyncTask lifespan to do the initial QueryUsersByAccelByteIds operation

		FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
		if (!UserStore.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query information about our friends as our user store instance is invalid!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}

		Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
		{
			FOnQueryUsersComplete OnQueryFriendInformationCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendInformationComplete);
			UserStore->QueryUsersByAccelByteIds(LocalUserNum, FriendIdsToQuery, OnQueryFriendInformationCompleteDelegate, true);
		}));

		bHasSentRequestForFriendInformation = true;
	}

	if (HasTaskFinishedAsyncWork())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteReadFriendsList::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());
		FriendInterface->AddFriendsToList(LocalUserNum, FoundFriends);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadFriendsList::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, ListName, ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

bool FOnlineAsyncTaskAccelByteReadFriendsList::HasTaskFinishedAsyncWork()
{
	// Check whether we have received responses for each friend type, invited or already friends
	if (bHasReceivedResponseForCurrentFriends && bHasReceivedResponseForIncomingFriends && bHasReceivedResponseForOutgoingFriends && bHasRecievedAllFriendInformation && bHasReceivedAllUserStatus)
	{
		return true;
	}

	// We either have not received responses for current, incoming, and/or outgoing friends, or we have not queried all user accounts yet
	return false;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::QueryFriendList()
{
	OnQueryFriendListSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsQueryFriendListResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendListSuccess);
	OnQueryFriendListFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendListFailed);

	API_FULL_CHECK_GUARD(Lobby, ErrorString);

	int32 Limit = MaximumQueryLimit;
	if (QueryFriendListLimit > MaximumQueryLimit)
	{
		QueryFriendListLimit -= MaximumQueryLimit;
	}
	else if(QueryFriendListLimit > 0)
	{
		Limit = QueryFriendListLimit;
		QueryFriendListLimit = 0;
	}
	Lobby->QueryFriendList(OnQueryFriendListSuccessDelegate, OnQueryFriendListFailedDelegate, QueryFriendListOffset, Limit);
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendListSuccess(const FAccelByteModelsQueryFriendListResponse& Result)
{
	// Add mappings for each friend loaded to their current friend status
	for (const FString& AccelByteId : Result.FriendIds)
	{
		AccelByteIdToFriendStatus.Add(AccelByteId, EInviteStatus::Accepted);
	}

	// Add all friend IDs to an array to query at the end
	FriendIdsToQuery.Append(Result.FriendIds);

	// check next page
	if(Result.Paging.Next.IsEmpty() || QueryFriendListLimit == 0)
	{
		bHasReceivedResponseForCurrentFriends = true;
	}
	else
	{
		SetLastUpdateTimeToCurrentTime();//Increase this AsyncTask lifespan if still continue to QueryFriendList()
		QueryFriendListOffset += MaximumQueryLimit;
		QueryFriendList();
	}
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendListFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("query-friends-failed-load-current-friends");
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to load friends list, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
	bHasReceivedResponseForCurrentFriends = true;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::QueryIncomingFriendRequest()
{
	OnQueryIncomingFriendRequestSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsIncomingFriendRequests>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryIncomingFriendRequestSuccess);
	OnQueryIncomingFriendRequestFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryIncomingFriendRequestFailed);
	API_FULL_CHECK_GUARD(Lobby, ErrorString);

	int32 Limit = MaximumQueryLimit;
	if (QueryIncomingFriendReqLimit > MaximumQueryLimit)
	{
		QueryIncomingFriendReqLimit -= MaximumQueryLimit;
	}
	else if(QueryIncomingFriendReqLimit > 0)
	{
		Limit = QueryIncomingFriendReqLimit;
		QueryIncomingFriendReqLimit = 0;
	}
	Lobby->QueryIncomingFriendRequest(OnQueryIncomingFriendRequestSuccessDelegate, OnQueryIncomingFriendRequestFailedDelegate, QueryIncomingFriendReqOffset,Limit);
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryIncomingFriendRequestSuccess(const FAccelByteModelsIncomingFriendRequests& Result)
{
	// Add mappings for each friend loaded to their current friend status
	for (const FAccelByteModelsFriendRequest& IncomingReq : Result.Data)
	{
		AccelByteIdToFriendStatus.Add(IncomingReq.FriendID, EInviteStatus::PendingInbound);

		// Add all friend IDs to an array to query at the end
		FriendIdsToQuery.Add(IncomingReq.FriendID);
	}

	// check next page
	if(Result.Paging.Next.IsEmpty() || QueryIncomingFriendReqLimit == 0)
	{
		bHasReceivedResponseForIncomingFriends = true;
	}
	else
	{
		SetLastUpdateTimeToCurrentTime();//Increase this AsyncTask lifespan if still continue to QueryIncomingFriendRequest()
		QueryIncomingFriendReqOffset += MaximumQueryLimit;
		QueryIncomingFriendRequest();
	}
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryIncomingFriendRequestFailed(int32 ErrorCode,	const FString& ErrorMessage)
{
	ErrorString = TEXT("query-friends-failed-load-incoming-friends");
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to load incoming friend request, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
	bHasReceivedResponseForIncomingFriends = true;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::QueryOutgoingFriendRequest()
{
	OnQueryOutgoingFriendRequestSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsOutgoingFriendRequests>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryOutgoingFriendRequestSuccess);
	OnQueryOutgoingFriendRequestFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryOutgoingFriendRequestFailed);
	API_FULL_CHECK_GUARD(Lobby, ErrorString);

	int32 Limit = MaximumQueryLimit;
	if (QueryOutgoingFriendReqLimit > MaximumQueryLimit)
	{
		QueryOutgoingFriendReqLimit -= MaximumQueryLimit;
	}
	else if(QueryOutgoingFriendReqLimit > 0)
	{
		Limit = QueryOutgoingFriendReqLimit;
		QueryOutgoingFriendReqLimit = 0;
	}
	Lobby->QueryOutgoingFriendRequest(OnQueryOutgoingFriendRequestSuccessDelegate, OnQueryOutgoingFriendRequestFailedDelegate, QueryOutgoingFriendReqOffset, Limit);
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryOutgoingFriendRequestSuccess(const FAccelByteModelsOutgoingFriendRequests& Result)
{
	// Add mappings for each friend loaded to their current friend status
	for (const FAccelByteModelsFriendRequest& OutgoingReq : Result.Data)
	{
		AccelByteIdToFriendStatus.Add(OutgoingReq.FriendID, EInviteStatus::PendingOutbound);

		// Add all friend IDs to an array to query at the end
		FriendIdsToQuery.Add(OutgoingReq.FriendID);
	}

	// check next page
	if (Result.Paging.Next.IsEmpty() || QueryOutgoingFriendReqLimit == 0)
	{
		bHasReceivedResponseForOutgoingFriends = true;
	}
	else
	{
		SetLastUpdateTimeToCurrentTime();//Increase this AsyncTask lifespan if still continue to QueryOutgoingFriendRequest()
		QueryOutgoingFriendReqOffset += MaximumQueryLimit;
		QueryOutgoingFriendRequest();
	}
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryOutgoingFriendRequestFailed(int32 ErrorCode,
	const FString& ErrorMessage)
{
	ErrorString = TEXT("query-friends-failed-load-outgoing-friends");
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to load outgoing friend request, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
	bHasReceivedResponseForOutgoingFriends = true;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendInformationComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	SetLastUpdateTimeToCurrentTime();
	if (bIsSuccessful)
	{
		for (const FAccelByteUserInfoRef& FriendInfo : UsersQueried)
		{
			EInviteStatus::Type* FoundInviteStatus = AccelByteIdToFriendStatus.Find(FriendInfo->Id->GetAccelByteId());
			if (FoundInviteStatus == nullptr)
			{
				continue;
			}

			TSharedPtr<FOnlineFriendAccelByte> Friend = MakeShared<FOnlineFriendAccelByte>(FriendInfo, *FoundInviteStatus);
			Friend->SetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, FriendInfo->GameAvatarUrl);
			Friend->SetUserAttribute(ACCELBYTE_ACCOUNT_PUBLISHER_AVATAR_URL, FriendInfo->PublisherAvatarUrl);
			
			FAccelByteModelsUserStatusNotif* UserPresenceStatus = AccelByteIdToPresence.Find(FriendInfo->Id->GetAccelByteId());
			if (UserPresenceStatus != nullptr)
			{
				FOnlineUserPresence Presence;
				Presence.bIsOnline = UserPresenceStatus->Availability == EAvailability::Online;
				Presence.Status.StatusStr = UserPresenceStatus->Activity;
				Presence.Status.State = UserPresenceStatus->Availability == EAvailability::Online ? EOnlinePresenceState::Online : EOnlinePresenceState::Offline;
				Presence.Status.Properties.Add(DefaultPlatformKey, UserPresenceStatus->Platform);
				Friend->SetPresence(Presence);
			}

			FoundFriends.Add(Friend);
		}
	}
	else
	{
		ErrorString = TEXT("query-friends-failed-load-friends-information");
		UE_LOG_AB(Warning, TEXT("Failed to get information about all friends in friends list!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}

	bHasRecievedAllFriendInformation = true;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnGetUserPresenceComplete(const FAccelByteModelsBulkUserStatusNotif& Statuses)
{
	for(FAccelByteModelsUserStatusNotif const& Status : Statuses.Data)
	{
		AccelByteIdToPresence.Add(Status.UserID, Status);
	}

	if (Statuses.NotProcessed.Num() > 0)
	{
		SetLastUpdateTimeToCurrentTime();//Increase this AsyncTask lifespan if still continue to process next UserStatus
		API_FULL_CHECK_GUARD(Lobby, ErrorString);
		Lobby->BulkGetUserPresenceV2(Statuses.NotProcessed,
			TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnGetUserPresenceComplete),
			FErrorHandler::CreateLambda([this](int32 Code, FString const& ErrMsg)
				{
					AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query friends presence"));
					CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				})
		);
	}
	else
	{
		bHasReceivedAllUserStatus = true;
	}
}
