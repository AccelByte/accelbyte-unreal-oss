// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAcceptFriendInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"

#include "Api/AccelByteLobbyApi.h"
#include <OnlineIdentityInterfaceAccelByte.h>
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteAcceptFriendInvite::FOnlineAsyncTaskAccelByteAcceptFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnAcceptInviteComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendId(FUniqueNetIdAccelByteUser::CastChecked(InFriendId))
	, ListName(InListName)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteAcceptFriendInvite::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d; FriendId: %s"), LocalUserNum, *FriendId->ToDebugString());

	// Attempt to get a pointer to the friend in the friends list, that way we can just update the invite status
	// rather than creating a new friend and removing from the list
	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
	InviteeFriend = FriendsInterface->GetFriend(LocalUserNum, FriendId.Get(), ListName);

	// Check if we have the friend in our list currently, as well as whether the friend in our list is an inbound friend,
	// rather than accepted or any other state. If it is pending inbound, then send off the request to accept their invite.
	// If they are not pending inbound, then we want to fail the async task entirely, as we shouldn't be accepting an invite
	// from them in the first place. We also want to fail the request if we don't have the user in our list at all, any friend
	// invite should be read through ReadFriendsList, or propagated through the delegates on the friends interface.
	if (InviteeFriend.IsValid())
	{
		const EInviteStatus::Type InviteStatus = InviteeFriend->GetInviteStatus();
		if (InviteStatus == EInviteStatus::PendingInbound)
		{
			// Since this friend is a valid pointer and is a pending inbound invite, then we want to send a request to accept their invite
			OnAcceptFriendRequestSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAcceptFriendInvite::OnAcceptFriendRequestSuccess);
			OnAcceptFriendRequestFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAcceptFriendInvite::OnAcceptFriendRequestFailed);
			API_FULL_CHECK_GUARD(Lobby, ErrorStr);
			Lobby->AcceptFriendRequest(FriendId->GetAccelByteId(), OnAcceptFriendRequestSuccessDelegate, OnAcceptFriendRequestFailedDelegate);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request through lobby websocket to accept a friend request."));
		}
		else
		{
			ErrorStr = FString::Printf(TEXT("Failed to accept friend request for %s as the invite status was not PendingInbound. Current invite status: %s"), *FriendId->ToDebugString(), EInviteStatus::ToString(InviteStatus));
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorStr);
		}
	}
	else
	{
		ErrorStr = FString::Printf(TEXT("Failed to accept friend request for %s as the friend invite does not exist in the local friends list, maybe need to call ReadFriendsList?"), *FriendId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorStr);
	}
}

void FOnlineAsyncTaskAccelByteAcceptFriendInvite::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// Update the invite status of the friend to match that it is an accepted friend
		TSharedPtr<FOnlineFriendAccelByte> AccelByteFriend = StaticCastSharedPtr<FOnlineFriendAccelByte>(InviteeFriend);
		AccelByteFriend->SetInviteStatus(EInviteStatus::Accepted);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid() && FriendId->IsValid())
		{
			FAccelByteModelsFriendRequestAcceptedPayload FriendRequestAcceptedPayload{};
			FriendRequestAcceptedPayload.SenderId = FriendId->GetAccelByteId();
			FriendRequestAcceptedPayload.ReceiverId = UserId->GetAccelByteId();
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsFriendRequestAcceptedPayload>(FriendRequestAcceptedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptFriendInvite::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, FriendId.Get(), ListName, ErrorStr);

	// If the friend invite accept was successful, then we want to also notify that we changed the friends list to reflect
	if (bWasSuccessful)
	{
		const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
		FriendsInterface->TriggerOnFriendsChangeDelegates(LocalUserNum);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptFriendInvite::OnAcceptFriendRequestSuccess()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""))
	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
		{
			SubsystemPin->GetPresenceInterface()->QueryPresence(*FriendId, TDelegateUtils<IOnlinePresence::FOnPresenceTaskCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAcceptFriendInvite::OnGetUserPresenceComplete));
		}));
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteAcceptFriendInvite::OnAcceptFriendRequestFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Failed to accept friend invite for user %s as the request failed on the backend. Error code: %d. Error message: %s"), *FriendId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteAcceptFriendInvite::OnGetUserPresenceComplete(const FUniqueNetId& TargetUserId, const bool bGetPresenceSuccess)
{
	TRY_PIN_SUBSYSTEM();

	TSharedPtr<FOnlineUserPresence> Presence;
	SubsystemPin->GetPresenceInterface()->GetCachedPresence(TargetUserId, Presence);
	if(Presence.IsValid())
	{
		TSharedPtr<FOnlineFriendAccelByte> AccelByteFriend = StaticCastSharedPtr<FOnlineFriendAccelByte>(InviteeFriend);
		AccelByteFriend->SetPresence(*Presence);
	}
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}
