// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendFriendInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Api/AccelByteLobbyApi.h"

FOnlineAsyncTaskAccelByteSendFriendInvite::FOnlineAsyncTaskAccelByteSendFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnSendInviteComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendId(FUniqueNetIdAccelByteUser::CastChecked(InFriendId))
	, ListName(InListName)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

FOnlineAsyncTaskAccelByteSendFriendInvite::FOnlineAsyncTaskAccelByteSendFriendInvite(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InFriendCode, const FString& InListName, const FOnSendInviteComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FriendId(FUniqueNetIdAccelByteUser::Invalid())
	, FriendCode(InFriendCode)
	, ListName(InListName)
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteSendFriendInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	// If we have the ID of the user that we want to friend, send that request off
	if (FriendId->IsValid())
	{
		QueryInvitedFriend(FriendId->GetAccelByteId());
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("ID of the user you wish to friend or a friend code must be specified to send a request!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendFriendInvite::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendInterface->AddFriendToList(LocalUserNum, InvitedFriend);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendFriendInvite::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(LocalUserNum, bWasSuccessful, ((bWasSuccessful) ? InvitedFriend->GetUserId().Get() : FriendId.Get()), ListName, ErrorStr);
	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendInterface->TriggerOnOutgoingInviteSentDelegates(LocalUserNum);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendFriendInvite::OnGetUserByFriendCodeError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("friend-request-invalid-code");
	UE_LOG_AB(Warning, TEXT("Failed to get user by friend code %s to send a friend invite!"), *FriendCode);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteSendFriendInvite::QueryInvitedFriend(const FString& InFriendId)
{
	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query invited friend '%s' as our user store instance is invalid!"), *InFriendId);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnQueryUsersComplete OnQueryInvitedFriendCompleteDelegate = FOnQueryUsersComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteSendFriendInvite::OnQueryInvitedFriendComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, { InFriendId }, OnQueryInvitedFriendCompleteDelegate, true);
}

void FOnlineAsyncTaskAccelByteSendFriendInvite::OnQueryInvitedFriendComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	if (bIsSuccessful && UsersQueried.IsValidIndex(0))
	{
		TSharedRef<FAccelByteUserInfo> User = UsersQueried[0];

		// Create the friend instance first, and then try and send the invite, this will only be added to the list if the
		// invite request successfully sends. Use the queried ID instead of the AccelByte ID, as that will have their
		// platform ID filled out
		InvitedFriend = MakeShared<FOnlineFriendAccelByte>(User->DisplayName, User->Id.ToSharedRef(), EInviteStatus::PendingOutbound);

		// Send the actual request to send the friend request
		AccelByte::Api::Lobby::FRequestFriendsResponse OnRequestFriendResponseDelegate = AccelByte::Api::Lobby::FRequestFriendsResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteSendFriendInvite::OnRequestFriendResponse);
		ApiClient->Lobby.SetRequestFriendsResponseDelegate(OnRequestFriendResponseDelegate);
		ApiClient->Lobby.RequestFriend(User->Id->GetAccelByteId());
	}
	else
	{
		ErrorStr = TEXT("friend-request-failed");
		UE_LOG_AB(Warning, TEXT("Failed to get query information on user '%s' for friend invite!"), *FriendId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}

void FOnlineAsyncTaskAccelByteSendFriendInvite::OnRequestFriendResponse(const FAccelByteModelsRequestFriendsResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("FriendId: %s"), *InvitedFriend->GetUserId()->ToDebugString());

	if (Result.Code != TEXT("0"))
	{
		ErrorStr = TEXT("friend-request-failed");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to send friend request to user %s! Error code: %s"), *InvitedFriend->GetUserId()->ToDebugString(), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
