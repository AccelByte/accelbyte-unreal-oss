// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineUserCacheAccelByte.h"

class FOnlineAsyncTaskAccelByteQueryUserProfile
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryUserProfile, ESPMode::ThreadSafe>
{
public:

	/** Constructor to setup the QueryUserProfile task */
	FOnlineAsyncTaskAccelByteQueryUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& InUserIds, const FOnQueryUserProfileComplete& InDelegate);

	/** Constructor to setup the QueryUserProfile task */
	FOnlineAsyncTaskAccelByteQueryUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const TArray<FString>& InUserIds, const FOnQueryUserProfileComplete& InDelegate);

	/** Constructor to setup the QueryUserProfile task */
	FOnlineAsyncTaskAccelByteQueryUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const TArray<TSharedRef<const FUniqueNetId>>& InUserIds, const FOnQueryUserProfileComplete& InDelegate);

	/** Constructor to setup the QueryUserProfile task */
	FOnlineAsyncTaskAccelByteQueryUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const TArray<FString>& InUserIds, const FOnQueryUserProfileComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryUserProfile");
	}

private:

	/** Array of user IDs that we initially sent to query */
	TArray<TSharedRef<const FUniqueNetId>> InitialUserIds;

	/** Array of strings for initial user IDs */
	TArray<FString> UserIdsToQuery;

	/** Array of users that we have retrieved from the query */
	TArray<FAccelByteModelsPublicUserProfileInfo> UsersQueried;

	/** Array of user IDs that we successfully queried (should be the same as initial, but may also differ if a user isn't found) */
	TArray<TSharedRef<const FUniqueNetId>> QueriedUserIds;
	
	/** String representing the error that was encountered while trying to query user information */
	FString ErrorStr;

	/** Delegate to fire on completion */
	FOnQueryUserProfileComplete Delegate;

	/** Initial query user profile endpoint */
	void QueryUserProfile(const TArray<FString>& UserIds);
	
	/** Delegate handler for when we complete a query for users from the backend */
	void OnQueryUsersProfileComplete(const FAccelByteModelsPublicUserProfileInfoV2& InUsersQueried);

	/**
	 * Delegate handler for when getting the user's profile from the game namespace from the AccelByte SDK fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnQueryUserProfileError(int32 ErrorCode, const FString& ErrorMessage);

};