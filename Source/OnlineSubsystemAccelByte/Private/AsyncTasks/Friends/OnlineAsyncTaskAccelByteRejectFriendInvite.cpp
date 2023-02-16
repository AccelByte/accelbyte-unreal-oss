// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRejectFriendInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

FOnlineAsyncTaskAccelByteRejectFriendInvite::FOnlineAsyncTaskAccelByteRejectFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendId(FUniqueNetIdAccelByteUser::CastChecked(InFriendId))
	, ListName(InListName)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());

	// Attempt to get a pointer to the friend in the friends list to make sure that we have their invite in our list
	// otherwise, the request will fail
	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	const TSharedPtr<FOnlineFriend> InviteeFriend = FriendsInterface->GetFriend(LocalUserNum, FriendId.Get(), ListName);

	// Check if we have the friend in our list currently, as well as whether the friend in our list is an inbound friend,
	// rather than accepted or any other state. If it is pending inbound, then send off the request to reject their invite.
	// If they are not pending inbound, then we want to fail the async task entirely, as we shouldn't be rejecting an invite
	// from them in the first place. We also want to fail the request if we don't have the user in our list at all, any friend
	// invite should be read through ReadFriendsList, or propagated through the delegates on the friends interface.
	if (InviteeFriend.IsValid())
	{
		const EInviteStatus::Type InviteStatus = InviteeFriend->GetInviteStatus();
		if (InviteStatus == EInviteStatus::PendingInbound)
		{
			// Since this friend is a valid pointer and is a pending inbound invite, then we want to send a request to reject their invite
			AccelByte::Api::Lobby::FRejectFriendsResponse OnRejectFriendResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FRejectFriendsResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRejectFriendInvite::OnRejectFriendResponse);
			ApiClient->Lobby.SetRejectFriendsResponseDelegate(OnRejectFriendResponseDelegate);
			ApiClient->Lobby.RejectFriend(FriendId->GetAccelByteId());
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request through lobby websocket to reject a friend request."));
		}
		else
		{
			ErrorStr = TEXT("friend-reject-failed-not-pending-friend");
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reject friend request for %s as the invite status was not PendingInbound. Current invite status: %s"), *FriendId->ToDebugString(), EInviteStatus::ToString(InviteStatus));
		}
	}
	else
	{
		ErrorStr = TEXT("friend-reject-failed-not-valid-friend");
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to reject friend request for %s as the friend invite does not exist in the local friends list, maybe need to call ReadFriendsList?"), *FriendId->ToDebugString());
	}
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// If the request was successful, then we want to get rid of this user from our friends list
	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendInterface->RemoveFriendFromList(LocalUserNum, FriendId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	FriendsInterface->TriggerOnRejectInviteCompleteDelegates(LocalUserNum, bWasSuccessful, FriendId.Get(), ListName, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::OnRejectFriendResponse(const FAccelByteModelsRejectFriendsResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorStr = TEXT("friend-reject-failed-request-error");
		UE_LOG_AB(Warning, TEXT("Failed to reject friend invite for user %s as the request failed on the backend. Error code: %s"), *FriendId->ToDebugString(), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}
