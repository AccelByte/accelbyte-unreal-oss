// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteLobbyModels.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineUserCacheAccelByte.h"

/**
 * Task to get a list of all users that the user has blocked
 */
class FOnlineAsyncTaskAccelByteQueryBlockedPlayers
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryBlockedPlayers, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteQueryBlockedPlayers(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryBlockedPlayers");
	}

private:

	/** Array of blocked users that we have found for this user */
	TArray<TSharedPtr<FOnlineBlockedPlayer>> FoundBlockedPlayers;

	/** String representing errors that occurred while trying to query blocked players, passed to delegate */
	FString ErrorStr;

	/** Delegate handler for when the request to list all blocked users succeeds */
	void OnGetListOfBlockedUsersSuccess(const FAccelByteModelsListBlockedUserResponse& Result);

	/** Delegate handler for when the request to list all blocked users fails */
	void OnGetListOfBlockedUsersError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when we get information on all blocked users */
	void OnQueryBlockedPlayersComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

};

