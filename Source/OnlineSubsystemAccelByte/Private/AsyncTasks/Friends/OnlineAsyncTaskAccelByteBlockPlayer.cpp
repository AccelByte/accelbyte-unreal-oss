// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBlockPlayer.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"
#include "OnlineUserCacheAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteBlockPlayer::FOnlineAsyncTaskAccelByteBlockPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PlayerId(FUniqueNetIdAccelByteUser::CastChecked(InPlayerId))
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
	if (!FriendsInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to block player as the FriendsInterface was invalid! Id: %s"), *PlayerId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FoundFriend = FriendsInterface->GetFriend(LocalUserNum, PlayerId.Get(), EFriendsLists::ToString(EFriendsLists::Default));

	// Now, send the request to block the player through the lobby websocket
	OnBlockPlayerSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBlockPlayer::OnBlockPlayerSuccess);
	OnBlockPlayerFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBlockPlayer::OnBlockPlayerFailed);
	API_CLIENT_CHECK_GUARD(ErrorStr);
	ApiClient->Lobby.BlockPlayer(PlayerId->GetAccelByteId(), OnBlockPlayerSuccessDelegate, OnBlockPlayerFailedDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBlockPlayer::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (!bWasSuccessful)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	// Remove the blocked player from the users friends list if they were already in their friends list
	const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
	if (FoundFriend.IsValid())
	{
		FriendsInterface->RemoveFriendFromList(LocalUserNum, PlayerId);
	}

	// We will want to then perform actions on the party depending on party role
	PerformBlockedPlayerPartyOperation();

	// Add the blocked player to the blocked players list
	FriendsInterface->AddBlockedPlayerToList(LocalUserNum, BlockedPlayer);

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid() && PlayerId->IsValid())
	{
		FAccelByteModelsUserBlockedPayload UserBlockedPayload{};
		UserBlockedPayload.SenderId = UserId->GetAccelByteId();
		UserBlockedPayload.ReceiverId = PlayerId->GetAccelByteId();
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsUserBlockedPayload>(UserBlockedPayload));
	}
}

void FOnlineAsyncTaskAccelByteBlockPlayer::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	FriendsInterface->TriggerOnBlockedPlayerCompleteDelegates(LocalUserNum, bWasSuccessful, PlayerId.Get(), EFriendsLists::ToString(EFriendsLists::Default), ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBlockPlayer::OnBlockPlayerSuccess()
{
	// We'll need to get the UserInfo from UserCache
	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not get information on blocked user '%s'!"), *PlayerId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		FOnQueryUsersComplete OnQueryBlockedPlayerCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBlockPlayer::OnQueryBlockedPlayerComplete);
		UserStore->QueryUsersByAccelByteIds(LocalUserNum, { PlayerId->GetAccelByteId() }, OnQueryBlockedPlayerCompleteDelegate, true);
	}));
}

void FOnlineAsyncTaskAccelByteBlockPlayer::OnBlockPlayerFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("block-player-request-failed");
	UE_LOG_AB(Warning, TEXT("Failed to block player %s as the request to the backend failed! Error code: %d, Error message: %s"), *PlayerId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteBlockPlayer::OnQueryBlockedPlayerComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	if (bIsSuccessful && UsersQueried.IsValidIndex(0))
	{
		FAccelByteUserInfoRef User = UsersQueried[0];
		BlockedPlayer = MakeShared<FOnlineBlockedPlayerAccelByte>(User);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorStr = TEXT("block-player-failed-retrieve-player-info");
		UE_LOG_AB(Warning, TEXT("Failed to block player completely as we could not get information on the blocked player from the backend! LocalUserNum: %d; BlockedPlayerId: %s"), LocalUserNum, *PlayerId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}

void FOnlineAsyncTaskAccelByteBlockPlayer::PerformBlockedPlayerPartyOperation()
{
	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		return;
	}

	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		return;
	}

	const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		return;
	}

	// NOTE @Damar : FUniqueId public constructor will be deprecated, changed to Create method
	FUniqueNetIdAccelByteUserPtr AccelByteUserId = FUniqueNetIdAccelByteUser::TryCast(LocalUserId.ToSharedRef());
	if (!AccelByteUserId.IsValid())
	{
		return;
	}

	TSharedPtr<FOnlinePartyAccelByte> UserParty = PartyInterface->GetFirstPartyForUser(UserId.ToSharedRef());
	if (!UserParty.IsValid())
	{
		return;
	}
	
	// Check if the user is in a multi-user party
	TArray<FOnlinePartyMemberConstRef> CurrentMembers = UserParty->GetAllMembers();
	if (CurrentMembers.Num() > 1)
	{
		if (PartyInterface->IsMemberLeader(*LocalUserId.Get(), UserParty->PartyId.Get(), *LocalUserId.Get()))
		{
			// If this user blocks a player who is currently in the same party as the user, and is the party leader, then kick the blocked player
			for (FOnlinePartyMemberConstRef PartyMember : CurrentMembers)
			{
				TSharedRef<const FUniqueNetIdAccelByteUser> PartyMemberCompositeId = FUniqueNetIdAccelByteUser::CastChecked(PartyMember->GetUserId());
				if (PartyMemberCompositeId->GetAccelByteId() == PlayerId->GetAccelByteId())
				{
					PartyInterface->KickMember(*LocalUserId.Get(), UserParty->PartyId.Get(), PlayerId.Get());
				}
			}
		}
		else
		{
			// If this user blocks a player who is currently in the same party as the user, and is just a member aka not party leader, then leave the party
			for (FOnlinePartyMemberConstRef PartyMember : CurrentMembers)
			{
				TSharedRef<const FUniqueNetIdAccelByteUser> PartyMemberCompositeId = FUniqueNetIdAccelByteUser::CastChecked(PartyMember->GetUserId());
				if (PartyMemberCompositeId->GetAccelByteId() == PlayerId->GetAccelByteId())
				{
					PartyInterface->LeaveParty(*LocalUserId.Get(), UserParty->PartyId.Get());
				}
			}
		}
	}
}