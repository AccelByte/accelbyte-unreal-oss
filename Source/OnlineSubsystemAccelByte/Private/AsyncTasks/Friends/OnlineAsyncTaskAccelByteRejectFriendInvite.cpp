// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRejectFriendInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

#include "Api/AccelByteLobbyApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRejectFriendInvite::FOnlineAsyncTaskAccelByteRejectFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendId(FUniqueNetIdAccelByteUser::CastChecked(InFriendId))
	, ListName(InListName)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());

	// Attempt to get a pointer to the friend in the friends list to make sure that we have their invite in our list
	// otherwise, the request will fail
	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
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
			OnRejectFriendSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRejectFriendInvite::OnRejectFriendSuccess);
			OnRejectFriendFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRejectFriendInvite::OnRejectFriendFailed);
			API_FULL_CHECK_GUARD(Lobby, ErrorStr);
			Lobby->RejectFriendRequest(FriendId->GetAccelByteId(), OnRejectFriendSuccessDelegate, OnRejectFriendFailedDelegate);
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
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// If the request was successful, then we want to get rid of this user from our friends list
	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());
		FriendInterface->RemoveFriendFromList(LocalUserNum, FriendId);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid() && FriendId->IsValid())
		{
			FAccelByteModelsFriendRequestRejectedPayload FriendRequestRejectedPayload{};
			FriendRequestRejectedPayload.SenderId = FriendId->GetAccelByteId();
			FriendRequestRejectedPayload.ReceiverId = UserId->GetAccelByteId();
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsFriendRequestRejectedPayload>(FriendRequestRejectedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
	FriendsInterface->TriggerOnRejectInviteCompleteDelegates(LocalUserNum, bWasSuccessful, FriendId.Get(), ListName, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::OnRejectFriendSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRejectFriendInvite::OnRejectFriendFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to reject friend invite for user %s as the request failed on the backend. Error code: %d. Error message: %s"), *FriendId->ToDebugString(), ErrorCode, *ErrorMessage);
	ErrorStr = TEXT("friend-reject-failed-request-error");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
