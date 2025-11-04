// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteFriend.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

#include "Api/AccelByteLobbyApi.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteDeleteFriend::FOnlineAsyncTaskAccelByteDeleteFriend(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendId(FUniqueNetIdAccelByteUser::CastChecked(InFriendId))
	, ListName(InListName)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteDeleteFriend::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d; FriendId: %s"), LocalUserNum, *FriendId->ToDebugString());

	// Attempt to get a pointer to the friend in the friends list to make sure that we have them in our friends list
	// otherwise the request to delete them as a friend will fail
	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
	const TSharedPtr<FOnlineFriend> InviteeFriend = FriendsInterface->GetFriend(LocalUserNum, FriendId.Get(), ListName);

	// Check if we have the friend in our list currently, as well as whether the friend in our list is currently our friend
	// or is a friend invite that we have sent. If they are currently our friend, then we will send an unfriend request, or
	// if they are an outbound friend invite, we will cancel the invite to be friends.
	if (InviteeFriend.IsValid())
	{
		InviteStatus = InviteeFriend->GetInviteStatus();
		if (InviteStatus == EInviteStatus::Accepted)
		{
			// Since this friend is a valid pointer and is actually one of our friends, then we want to send a request to remove them
			OnUnfriendSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteFriend::OnUnfriendSuccess);
			OnUnfriendFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteFriend::OnCancelFriendRequestFailed);
			API_FULL_CHECK_GUARD(Lobby, ErrorStr);
			Lobby->Unfriend(FriendId->GetAccelByteId(), OnUnfriendSuccessDelegate, OnUnfriendFailedDelegate);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request through lobby websocket to remove a friend."));
		}
		else if (InviteStatus == EInviteStatus::PendingOutbound)
		{	
			// Since this friend is a valid pointer and is an outbound request we have sent to be their friend, we want to cancel this request
			OnCancelFriendRequestSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteFriend::OnCancelFriendRequestSuccess);
			OnCancelFriendRequestFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteFriend::OnCancelFriendRequestFailed);
			API_FULL_CHECK_GUARD(Lobby, ErrorStr);
			Lobby->CancelFriendRequest(FriendId->GetAccelByteId(), OnCancelFriendRequestSuccessDelegate, OnCancelFriendRequestFailedDelegate);

			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request through lobby websocket to cancel an outbound friend request."));
		}
		else
		{
			ErrorStr = TEXT("friend-error-remove-failed-invalid-invite-state");
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove friend or cancel an invite for friend %s as the invite status was not Accepted or PendingOutbound. Current invite status: %s"), *FriendId->ToDebugString(), EInviteStatus::ToString(InviteStatus));
		}
	}
	else
	{
		ErrorStr = TEXT("friend-error-remove-failed-friend-not-found");
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove friend or cancel an invite for friend %s as the friend/invite does not exist in the local friends list, maybe need to call ReadFriendsList?"), *FriendId->ToDebugString());
	}
}

void FOnlineAsyncTaskAccelByteDeleteFriend::Finalize()
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
			if (InviteStatus == EInviteStatus::Accepted)
			{
				FAccelByteModelsFriendUnfriendedPayload FriendUnfriendPayload{};
				FriendUnfriendPayload.SenderId = UserId->GetAccelByteId();
				FriendUnfriendPayload.FriendId = FriendId->GetAccelByteId();
				PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsFriendUnfriendedPayload>(FriendUnfriendPayload));
			}
			else if (InviteStatus == EInviteStatus::PendingOutbound)
			{
				FAccelByteModelsFriendRequestRejectedPayload FriendRequestRejectedPayload{};
				FriendRequestRejectedPayload.SenderId = FriendId->GetAccelByteId();
				FriendRequestRejectedPayload.ReceiverId = UserId->GetAccelByteId();
				PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsFriendRequestRejectedPayload>(FriendRequestRejectedPayload));
			}
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteFriend::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
	FriendsInterface->TriggerOnDeleteFriendCompleteDelegates(LocalUserNum, bWasSuccessful, FriendId.Get(), ListName, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteFriend::OnUnfriendSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteFriend::OnUnfriendFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());
	ErrorStr = TEXT("friend-error-remove-failed");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to remove friend %s as the request failed on the backend. Error code: %d. Error Message"), *FriendId->ToDebugString(), ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteDeleteFriend::OnCancelFriendRequestSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteFriend::OnCancelFriendRequestFailed(int32 ErrorCode,	const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *FriendId->ToDebugString());
	ErrorStr = TEXT("friend-rescind-request-failed");
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to cancel friend invate to user %s! Error code: %d. Error message: %s"), *FriendId->ToDebugString(), ErrorCode, *ErrorMessage);
}
