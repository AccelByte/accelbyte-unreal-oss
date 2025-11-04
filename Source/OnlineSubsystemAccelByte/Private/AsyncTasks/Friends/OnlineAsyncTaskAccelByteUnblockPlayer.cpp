// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUnblockPlayer.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

#include "Api/AccelByteLobbyApi.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteUnblockPlayer::FOnlineAsyncTaskAccelByteUnblockPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PlayerId(FUniqueNetIdAccelByteUser::CastChecked(InPlayerId))
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d; PlayerId: %s"), LocalUserNum, *PlayerId->ToString());

	// Unblocking a player is straightforward as we just will send the request to unblock them and delete the entry from
	// the blocked players list if the unblock call is successful
	OnUnblockPlayerSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnblockPlayer::OnUnblockPlayerSuccess);
	OnUnblockPlayerFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUnblockPlayer::OnUnblockPlayerFailed);
	API_FULL_CHECK_GUARD(Lobby, ErrorStr);
	Lobby->UnblockPlayer(PlayerId->GetAccelByteId(), OnUnblockPlayerSuccessDelegate, OnUnblockPlayerFailedDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());
		FriendsInterface->RemoveBlockedPlayerFromList(LocalUserNum, PlayerId);

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid() && PlayerId->IsValid())
		{
			FAccelByteModelsUserBlockedPayload UserBlockedPayload{};
			UserBlockedPayload.SenderId = UserId->GetAccelByteId();
			UserBlockedPayload.ReceiverId = PlayerId->GetAccelByteId();
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsUserBlockedPayload>(UserBlockedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	const IOnlineFriendsPtr FriendsInterface = SubsystemPin->GetFriendsInterface();
	FriendsInterface->TriggerOnUnblockedPlayerCompleteDelegates(LocalUserNum, bWasSuccessful, PlayerId.Get(), EFriendsLists::ToString(EFriendsLists::Default), ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::OnUnblockPlayerSuccess()
{
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteUnblockPlayer::OnUnblockPlayerFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("unblock-player-request-failed");
	UE_LOG_AB(Warning, TEXT("Failed to unblock player %s as the request failed on the backend! Error code: %d, Error message: %s"), *PlayerId->ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
