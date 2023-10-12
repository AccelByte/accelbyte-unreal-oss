// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteFriend.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

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
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d; FriendId: %s"), LocalUserNum, *FriendId->ToDebugString());

	// Attempt to get a pointer to the friend in the friends list to make sure that we have them in our friends list
	// otherwise the request to delete them as a friend will fail
	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
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
			AccelByte::Api::Lobby::FUnfriendResponse OnUnfriendResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FUnfriendResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteFriend::OnUnfriendResponse);
			ApiClient->Lobby.SetUnfriendResponseDelegate(OnUnfriendResponseDelegate);
			ApiClient->Lobby.Unfriend(FriendId->GetAccelByteId());
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request through lobby websocket to remove a friend."));
		}
		else if (InviteStatus == EInviteStatus::PendingOutbound)
		{	
			// Since this friend is a valid pointer and is an outbound request we have sent to be their friend, we want to cancel this request
			AccelByte::Api::Lobby::FCancelFriendsResponse OnCancelFriendRequestResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FCancelFriendsResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteFriend::OnCancelFriendRequestResponse);
			ApiClient->Lobby.SetCancelFriendsResponseDelegate(OnCancelFriendRequestResponseDelegate);
			ApiClient->Lobby.CancelFriendRequest(FriendId->GetAccelByteId());
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
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	// If the request was successful, then we want to get rid of this user from our friends list
	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendInterface->RemoveFriendFromList(LocalUserNum, FriendId);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
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
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	FriendsInterface->TriggerOnDeleteFriendCompleteDelegates(LocalUserNum, bWasSuccessful, FriendId.Get(), ListName, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteFriend::OnUnfriendResponse(const FAccelByteModelsUnfriendResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorStr = TEXT("friend-error-remove-failed");
		UE_LOG_AB(Warning, TEXT("Failed to remove friend %s as the request failed on the backend. Error code: %s"), *FriendId->ToDebugString(), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteDeleteFriend::OnCancelFriendRequestResponse(const FAccelByteModelsCancelFriendsResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorStr = TEXT("friend-error-request-cancel-failed");
		UE_LOG_AB(Warning, TEXT("Failed to cancel friend request for user %s as the request failed on the backend. Error code: %s"), *FriendId->ToDebugString(), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}
