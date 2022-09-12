// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserCacheAccelByte.h"

struct FAccelByteModelsSessionBrowserRecentPlayerGetResult;

/**
 * Task for blocking a player on the AccelByte backend
 */
class FOnlineAsyncTaskAccelByteGetRecentPlayer : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteGetRecentPlayer(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString &InNamespace);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetRecentPlayer");
	}

private:	

	/** Namespace from the OSS **/
	FString Namespace;

	/** Cached error string **/
	FString ErrorString;

	/** Information on all recent players that we queried */
	TArray<TSharedRef<FAccelByteUserInfo>> RecentPlayersQueried;

	void OnGetRecentPlayerSuccess(const FAccelByteModelsSessionBrowserRecentPlayerGetResult &InResult);
	
	void OnGetRecentPlayerError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when we finish querying for recent player information */
	void OnQueryRecentPlayersComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

};
