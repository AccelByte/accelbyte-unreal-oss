// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineUserCacheAccelByte.h"

class FOnlineAsyncTaskAccelByteQueryUserInfo : public FOnlineAsyncTaskAccelByte
{
public:

	/** Constructor to setup the QueryUserInfo task */
	FOnlineAsyncTaskAccelByteQueryUserInfo(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& InUserIds, const FOnQueryUserInfoComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryUserInfo");
	}

private:

	/** Array of user IDs that we initially sent to query */
	TArray<TSharedRef<const FUniqueNetId>> InitialUserIds;

	/** Array of strings for initial user IDs */
	TArray<FString> UserIdsToQuery;

	/** Delegate to fire on completion */
	FOnQueryUserInfoComplete Delegate;

	/** Array of users that we have retrieved from the query */
	TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried;

	/** Array of user IDs that we successfully queried (should be the same as initial, but may also differ if a user isn't found) */
	TArray<TSharedRef<const FUniqueNetId>> QueriedUserIds;
	
	/** String representing the error that was encountered while trying to query user information */
	FString ErrorStr;

	/** Delegate handler for when we complete a query for users from the backend */
	void OnQueryUsersComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> InUsersQueried);

};