// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReadFriendsList.h"

#include "OnlinePresenceInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"
#include "Api/AccelByteUserApi.h"

FOnlineAsyncTaskAccelByteReadFriendsList::FOnlineAsyncTaskAccelByteReadFriendsList(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InListName, const FOnReadFriendsListComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, ListName(InListName)
	, Delegate(InDelegate)
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
	AccelByte::Api::Lobby::FLoadFriendListResponse OnLoadFriendsListResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FLoadFriendListResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnLoadFriendsListResponse);
	ApiClient->Lobby.SetLoadFriendListResponseDelegate(OnLoadFriendsListResponseDelegate);

	AccelByte::Api::Lobby::FListIncomingFriendsResponse OnListIncomingFriendsResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FListIncomingFriendsResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnListIncomingFriendsResponse);
	ApiClient->Lobby.SetListIncomingFriendsResponseDelegate(OnListIncomingFriendsResponseDelegate);

	AccelByte::Api::Lobby::FListOutgoingFriendsResponse OnListOutgoingFriendsResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FListOutgoingFriendsResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnListOutgoingFriendsResponse);
	ApiClient->Lobby.SetListOutgoingFriendsResponseDelegate(OnListOutgoingFriendsResponseDelegate);

	// Fire off all list requests for friends
	ApiClient->Lobby.ListIncomingFriends();
	ApiClient->Lobby.ListOutgoingFriends();
	ApiClient->Lobby.LoadFriendsList();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadFriendsList::Tick()
{
	Super::Tick();

	if (bHasReceivedResponseForCurrentFriends && bHasReceivedResponseForIncomingFriends && bHasReceivedResponseForOutgoingFriends && (!bHasSentRequestForUserStatus && !bHasReceivedAllUserStatus))
	{
		ApiClient->Lobby.BulkGetUserPresence(FriendIdsToQuery,
			TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnGetUserPresenceComplete),
			FErrorHandler::CreateLambda([this](int32 Code, FString const& ErrMsg)
			{
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query friends presence"));
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			})
		);
		bHasSentRequestForUserStatus = true;
	}
	// If we have received all friend IDs from each list, but we haven't tried to get information on those friends yet, send the request out
	if (bHasReceivedResponseForCurrentFriends && bHasReceivedResponseForIncomingFriends && bHasReceivedResponseForOutgoingFriends && bHasReceivedAllUserStatus && (!bHasSentRequestForFriendInformation && !bHasRecievedAllFriendInformation))
	{
		FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
		if (!UserStore.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query information about our friends as our user store instance is invalid!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			return;
		}

		FOnQueryUsersComplete OnQueryFriendInformationCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendInformationComplete);
		UserStore->QueryUsersByAccelByteIds(LocalUserNum, FriendIdsToQuery, OnQueryFriendInformationCompleteDelegate, true);

		bHasSentRequestForFriendInformation = true;
	}

	if (HasTaskFinishedAsyncWork())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteReadFriendsList::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
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

void FOnlineAsyncTaskAccelByteReadFriendsList::OnLoadFriendsListResponse(const FAccelByteModelsLoadFriendListResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorString = TEXT("query-friends-failed-load-current-friends");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to load friends list as response was non-zero! Response code: %s"), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Add mappings for each friend loaded to their current friend status
	for (const FString& AccelByteId : Result.friendsId)
	{
		AccelByteIdToFriendStatus.Add(AccelByteId, EInviteStatus::Accepted);
	}

	// Add all friend IDs to an array to query at the end
	FriendIdsToQuery.Append(Result.friendsId);
	bHasReceivedResponseForCurrentFriends = true;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnListIncomingFriendsResponse(const FAccelByteModelsListIncomingFriendsResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorString = TEXT("query-friends-failed-load-incoming-friends");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to load friends list as response was non-zero! Response code: %s"), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Add mappings for each friend loaded to their current friend status
	for (const FString& AccelByteId : Result.friendsId)
	{
		AccelByteIdToFriendStatus.Add(AccelByteId, EInviteStatus::PendingInbound);
	}

	// Add all friend IDs to an array to query at the end
	FriendIdsToQuery.Append(Result.friendsId);
	bHasReceivedResponseForIncomingFriends = true;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnListOutgoingFriendsResponse(const FAccelByteModelsListOutgoingFriendsResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorString = TEXT("query-friends-failed-load-outgoing-friends");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to load friends list as response was non-zero! Response code: %s"), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Add mappings for each friend loaded to their current friend status
	for (const FString& AccelByteId : Result.friendsId)
	{
		AccelByteIdToFriendStatus.Add(AccelByteId, EInviteStatus::PendingOutbound);
	}

	// Add all friend IDs to an array to query at the end
	FriendIdsToQuery.Append(Result.friendsId);
	bHasReceivedResponseForOutgoingFriends = true;
}

void FOnlineAsyncTaskAccelByteReadFriendsList::OnQueryFriendInformationComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	SetLastUpdateTimeToCurrentTime();
	if (bIsSuccessful)
	{
		for (const TSharedRef<FAccelByteUserInfo>& FriendInfo : UsersQueried)
		{
			EInviteStatus::Type* FoundInviteStatus = AccelByteIdToFriendStatus.Find(FriendInfo->Id->GetAccelByteId());
			if (FoundInviteStatus == nullptr)
			{
				continue;
			}

			TSharedPtr<FOnlineFriendAccelByte> Friend = MakeShared<FOnlineFriendAccelByte>(FriendInfo->DisplayName, FriendInfo->Id.ToSharedRef(), *FoundInviteStatus);
			Friend->SetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, FriendInfo->GameAvatarUrl);
			Friend->SetUserAttribute(ACCELBYTE_ACCOUNT_PUBLISHER_AVATAR_URL, FriendInfo->PublisherAvatarUrl);
			
			FAccelByteModelsUserStatusNotif* UserPresenceStatus = AccelByteIdToPresence.Find(FriendInfo->Id->GetAccelByteId());
			if (UserPresenceStatus != nullptr)
			{
				FOnlineUserPresence Presence;
				Presence.bIsOnline = UserPresenceStatus->Availability == EAvailability::Online;
				Presence.Status.StatusStr = UserPresenceStatus->Activity;
				Presence.Status.State = UserPresenceStatus->Availability == EAvailability::Online ? EOnlinePresenceState::Online : EOnlinePresenceState::Offline;
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
	bHasReceivedAllUserStatus = true;
}
