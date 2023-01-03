// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryBlockedPlayers.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"
#include "Api/AccelByteUserApi.h"
#include <OnlineFriendsInterfaceAccelByte.h>

FOnlineAsyncTaskAccelByteQueryBlockedPlayers::FOnlineAsyncTaskAccelByteQueryBlockedPlayers(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryBlockedPlayers::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	THandler<FAccelByteModelsListBlockedUserResponse> OnGetListOfBlockedUsersSuccessDelegate = THandler<FAccelByteModelsListBlockedUserResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryBlockedPlayers::OnGetListOfBlockedUsersSuccess);
	FErrorHandler OnGetListOfBlockedUsersErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryBlockedPlayers::OnGetListOfBlockedUsersError);
	ApiClient->Lobby.GetListOfBlockedUsers(OnGetListOfBlockedUsersSuccessDelegate, OnGetListOfBlockedUsersErrorDelegate);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryBlockedPlayers::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
		FriendInterface->AddBlockedPlayersToList(UserId.ToSharedRef(), FoundBlockedPlayers);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryBlockedPlayers::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineFriendsPtr FriendInterface = Subsystem->GetFriendsInterface();
	FriendInterface->TriggerOnQueryBlockedPlayersCompleteDelegates(UserId.ToSharedRef().Get(), bWasSuccessful, ErrorStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryBlockedPlayers::OnGetListOfBlockedUsersSuccess(const FAccelByteModelsListBlockedUserResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; Blocked user amount: %d"), *UserId->ToDebugString(), Result.Data.Num());

	// Get an array of user IDs that we will query for information to display
	TArray<FString> BlockedUserIds;
	for (const FBlockedData& BlockedUser : Result.Data)
	{
		BlockedUserIds.Add(BlockedUser.BlockedUserId);
	}

	// We have no blocked users to query, thus we just can complete this task
	if (BlockedUserIds.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("User '%s' has no blocked players, task ending!"), *UserId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query blocked user information as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnQueryUsersComplete OnQueryBlockedPlayersCompleteDelegate = FOnQueryUsersComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryBlockedPlayers::OnQueryBlockedPlayersComplete);
	UserStore->QueryUsersByAccelByteIds(UserId.ToSharedRef().Get(), BlockedUserIds, OnQueryBlockedPlayersCompleteDelegate, true);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off %d requests to get information on blocked users."), Result.Data.Num());
}

void FOnlineAsyncTaskAccelByteQueryBlockedPlayers::OnGetListOfBlockedUsersError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("blocked-players-request-failed");
	UE_LOG_AB(Warning, TEXT("Failed to get list of blocked users as the query to the backend failed. Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryBlockedPlayers::OnQueryBlockedPlayersComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	if (bIsSuccessful)
	{
		for (const TSharedRef<FAccelByteUserInfo>& BlockedPlayer : UsersQueried)
		{
			FoundBlockedPlayers.Add(MakeShared<FOnlineBlockedPlayerAccelByte>(BlockedPlayer->DisplayName, BlockedPlayer->Id.ToSharedRef()));
		}
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		ErrorStr = TEXT("blocked-players-information-query-fail");
		UE_LOG_AB(Warning, TEXT("Failed to get information about blocked players!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}
