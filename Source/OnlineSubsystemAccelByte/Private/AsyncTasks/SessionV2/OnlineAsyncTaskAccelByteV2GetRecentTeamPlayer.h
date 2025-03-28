// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Queries all invites for a player, includes both game session and party session invites. Developer then can filter those invites locally.
 */
class FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId, const FString& InNamespace);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteV2GetRecentTeamPlayer");
	}

private:
	const int32 GetRecentTeamPlayerLimit {200};

	/** Namespace from the OSS **/
	FString Namespace;

	/** Temporary holder for get recent player result */
	TMap<FString, FAccelByteModelsV2SessionRecentPlayer> RecentTeamPlayerResultMap;
	
	/** Information on all recent players that we queried */
	TArray<FAccelByteUserInfoRef> RecentTeamPlayersQueried;

	/** Cached error string **/
	FString ErrorStr;

	THandler<FAccelByteModelsV2SessionRecentPlayers> OnGetRecentTeamPlayersSuccessDelegate;
	
	void OnGetRecentTeamPlayersSuccess(const FAccelByteModelsV2SessionRecentPlayers& Result);

	FErrorHandler OnGetRecentTeamPlayersErrorDelegate;
	
	void OnGetRecentTeamPlayersError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when we finish querying for recent player information */
	FOnQueryUsersComplete OnQueryRecentTeamPlayersCompleteDelegate;
	
	void OnQueryRecentTeamPlayersComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried);

};

