// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUnblockPlayer.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

FOnlineAsyncTaskAccelByteUnblockPlayer::FOnlineAsyncTaskAccelByteUnblockPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PlayerId(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InPlayerId.AsShared()))
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d; PlayerId: %s"), LocalUserNum, *PlayerId->ToString());

	// Unblocking a player is straightforward as we just will send the request to unblock them and delete the entry from
	// the blocked players list if the unblock call is successful
	AccelByte::Api::Lobby::FUnblockPlayerResponse OnUnblockPlayerResponseDelegate = AccelByte::Api::Lobby::FUnblockPlayerResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteUnblockPlayer::OnUnblockPlayerResponse);
	ApiClient->Lobby.SetUnblockPlayerResponseDelegate(OnUnblockPlayerResponseDelegate);
	ApiClient->Lobby.UnblockPlayer(PlayerId->GetAccelByteId());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendsInterface->RemoveBlockedPlayerFromList(LocalUserNum, PlayerId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	const IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface();
	FriendsInterface->TriggerOnUnblockedPlayerCompleteDelegates(LocalUserNum, bWasSuccessful, PlayerId.Get(), EFriendsLists::ToString(EFriendsLists::Default), ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::OnUnblockPlayerResponse(const FAccelByteModelsUnblockPlayerResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		ErrorStr = TEXT("unblock-player-request-failed");
		UE_LOG_AB(Warning, TEXT("Failed to unblock player %s as the request failed on the backend! Error code: %s"), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}
