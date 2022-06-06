// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserCacheAccelByte.h"

/**
 * Task for blocking a player on the AccelByte backend
 */
class FOnlineAsyncTaskAccelByteBlockPlayer : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteBlockPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBlockPlayer");
	}

private:

	/** Id of the user that we want to block */
	TSharedRef<const FUniqueNetIdAccelByteUser> PlayerId;

	/** Shared pointer to a friend instance that corresponds to the player we are blocking, could be nullptr */
	TSharedPtr<FOnlineFriend> FoundFriend;

	/** Shared pointer to a blocked player instance that we construct on a successful block */
	TSharedPtr<FOnlineBlockedPlayer> BlockedPlayer;

	/** String representation of errors we encountered while running this task, passed to delegate */
	FString ErrorStr;

	/** Delegate handler for when a request to block a player completes */
	void OnBlockPlayerResponse(const FAccelByteModelsBlockPlayerResponse& Result);

	/** Delegate handler for when we complete a query for information about the newly blocked player */
	void OnQueryBlockedPlayerComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

};
