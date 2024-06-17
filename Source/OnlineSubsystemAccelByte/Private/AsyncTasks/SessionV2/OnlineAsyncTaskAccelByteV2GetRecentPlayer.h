// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Queries all invites for a player, includes both game session and party session invites. Developer then can filter those invites locally.
 */
class FOnlineAsyncTaskAccelByteV2GetRecentPlayer
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteV2GetRecentPlayer, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteV2GetRecentPlayer(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId, const FString& InNamespace);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteV2GetRecentPlayer");
	}

private:
	const int32 GetRecentPlayerLimit {200};

	/** Namespace from the OSS **/
	FString Namespace;

	/** Temporary holder for get recent player result */
	TMap<FString, FAccelByteModelsV2SessionRecentPlayer> RecentPlayerResultMap;
	
	/** Information on all recent players that we queried */
	TArray<FAccelByteUserInfoRef> RecentPlayersQueried;

	/** Cached error string **/
	FString ErrorStr;

	THandler<FAccelByteModelsV2SessionRecentPlayers> OnGetRecentPlayersSuccessDelegate;
	
	void OnGetRecentPlayersSuccess(const FAccelByteModelsV2SessionRecentPlayers& Result);

	FErrorHandler OnGetRecentPlayersErrorDelegate;
	
	void OnGetRecentPlayersError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when we finish querying for recent player information */
	FOnQueryUsersComplete OnQueryRecentPlayersCompleteDelegate;
	
	void OnQueryRecentPlayersComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried);

};

