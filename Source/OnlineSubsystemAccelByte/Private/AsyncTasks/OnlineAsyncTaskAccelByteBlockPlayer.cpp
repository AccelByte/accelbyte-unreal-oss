// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBlockPlayer.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"
#include "OnlineUserCacheAccelByte.h"

FOnlineAsyncTaskAccelByteBlockPlayer::FOnlineAsyncTaskAccelByteBlockPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PlayerId(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InPlayerId.AsShared()))
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteBlockPlayer::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d; PlayerId: %s"), LocalUserNum, *PlayerId->ToDebugString());

	if (!PlayerId->IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to block player as their ID was invalid! Id: %s"), *PlayerId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// First, check if the player we want to block is one of our friends, if this is the case, then we just want to get
	// a reference to this player in this task so we can not only update their friend status but also add the blocked
	// player to the list without an extra request to get their display name
	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	FoundFriend = FriendsInterface->GetFriend(LocalUserNum, PlayerId.Get(), EFriendsLists::ToString(EFriendsLists::Default));

	// Now, send the request to block the player through the lobby websocket
	AccelByte::Api::Lobby::FBlockPlayerResponse OnBlockPlayerResponseDelegate = AccelByte::Api::Lobby::FBlockPlayerResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteBlockPlayer::OnBlockPlayerResponse);
	ApiClient->Lobby.SetBlockPlayerResponseDelegate(OnBlockPlayerResponseDelegate);
	ApiClient->Lobby.BlockPlayer(PlayerId->GetAccelByteId());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBlockPlayer::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// Remove the blocked player from the users friends list if they were already in their friends list
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		if (FoundFriend.IsValid())
		{
			FriendsInterface->RemoveFriendFromList(LocalUserNum, PlayerId);
		}

		// We will want to then perform actions on the party depending on party role
		const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (IdentityInterface.IsValid())
		{
			const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
			if (PartyInterface.IsValid())
			{
				// NOTE @Damar : FUniqueId public constructor will be deprecated, changed to Create method
				TSharedRef<const FUniqueNetIdAccelByteUser> CompositeId = FUniqueNetIdAccelByteUser::Create(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef().Get()).ToSharedRef();
				TSharedPtr<FOnlinePartyAccelByte> UserParty = PartyInterface->GetFirstPartyForUser(CompositeId);
				if (UserParty != nullptr)
				{
					// Check if the user is in a multi-user party
					TArray<FOnlinePartyMemberConstRef> CurrentMembers = UserParty->GetAllMembers();
					if (CurrentMembers.Num() > 1)
					{
						if (PartyInterface->IsMemberLeader(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef().Get(), UserParty->PartyId.Get(), IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef().Get()))
						{
							// If this user blocks a player who is currently in the same party as the user, and is the party leader, then kick the blocked player
							for (FOnlinePartyMemberConstRef PartyMember : CurrentMembers)
							{
								TSharedRef<const FUniqueNetIdAccelByteUser> PartyMemberCompositeId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(PartyMember->GetUserId());
								if (PartyMemberCompositeId->GetAccelByteId() == PlayerId->GetAccelByteId())
								{
									PartyInterface->KickMember(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef().Get(),UserParty->PartyId.Get(), PlayerId.Get());
								}
							}
						}
						else
						{
							// If this user blocks a player who is currently in the same party as the user, and is just a member aka not party leader, then leave the party
							for (FOnlinePartyMemberConstRef PartyMember : CurrentMembers)
							{
								TSharedRef<const FUniqueNetIdAccelByteUser> PartyMemberCompositeId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(PartyMember->GetUserId());
								if (PartyMemberCompositeId->GetAccelByteId() == PlayerId->GetAccelByteId())
								{
									PartyInterface->LeaveParty(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef().Get(), UserParty->PartyId.Get());
								}
							}
						}
					}
				}
			}
		}

		// Add the blocked player to the blocked players list
		FriendsInterface->AddBlockedPlayerToList(LocalUserNum, BlockedPlayer);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBlockPlayer::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	FriendsInterface->TriggerOnBlockedPlayerCompleteDelegates(LocalUserNum, bWasSuccessful, PlayerId.Get(), EFriendsLists::ToString(EFriendsLists::Default), ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBlockPlayer::OnBlockPlayerResponse(const FAccelByteModelsBlockPlayerResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorStr = TEXT("block-player-request-failed");
		UE_LOG_AB(Warning, TEXT("Failed to block player %s as the request to the backend failed! Error code: %s"), *PlayerId->ToDebugString(), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		// If we have a friend instance already, we want to construct a blocked player instance from that friend and complete the task...
		if (FoundFriend.IsValid())
		{
			BlockedPlayer = MakeShared<FOnlineBlockedPlayerAccelByte>(FoundFriend->GetDisplayName(), PlayerId);
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
		// Otherwise, we need to go and query the player ID to get a display name to properly construct a blocked player...
		else
		{
			FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
			if (!UserStore.IsValid())
			{
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not get information on blocked user '%s'!"), *PlayerId->ToDebugString());
				CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
				return;
			}

			FOnQueryUsersComplete OnQueryBlockedPlayerCompleteDelegate = FOnQueryUsersComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteBlockPlayer::OnQueryBlockedPlayerComplete);
			UserStore->QueryUsersByAccelByteIds(LocalUserNum, { PlayerId->GetAccelByteId() }, OnQueryBlockedPlayerCompleteDelegate, true);
		}
	}
}

void FOnlineAsyncTaskAccelByteBlockPlayer::OnQueryBlockedPlayerComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	if (bIsSuccessful && UsersQueried.IsValidIndex(0))
	{
		TSharedRef<FAccelByteUserInfo> User = UsersQueried[0];
		BlockedPlayer = MakeShared<FOnlineBlockedPlayerAccelByte>(User->DisplayName, User->Id.ToSharedRef());
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorStr = TEXT("block-player-failed-retrieve-player-info");
		UE_LOG_AB(Warning, TEXT("Failed to block player completely as we could not get information on the blocked player from the backend! LocalUserNum: %d; BlockedPlayerId: %s"), LocalUserNum, *PlayerId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}
