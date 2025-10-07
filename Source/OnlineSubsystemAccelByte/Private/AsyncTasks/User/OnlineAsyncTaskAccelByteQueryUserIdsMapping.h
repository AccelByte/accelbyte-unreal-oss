// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"

class FOnlineAsyncTaskAccelByteQueryUserIdsMapping
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryUserIdsMapping, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteQueryUserIdsMapping(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InDisplayNameOrEmail, const FOnQueryUserIdsMappingComplete& InDelegate, int32 InOffset = 0, int32 InLimit = 100);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryUserIdsMapping");
	}

private:

	/** Display name or email to query for a user ID */
	FString DisplayNameOrEmail;

	/** Delegate to fire on completion */
	FOnQueryUserIdsMappingComplete Delegate;

	/** ID of the user that was found by the request */
	TArray<TSharedRef<const FUniqueNetIdAccelByteUser>> SearchedUsers;

	/** String containing an error if one was encountered */
	FString ErrorString;

	int32 Offset = 0;

	int32 Limit = 100;

	/** Delegate handler for when the SearchUsers call gets data back, meaning we can search for a mapping */
	void OnSearchUserSuccessResponse(const FPagedPublicUsersInfo& Result);

	/** Delegate handler for when the SearchUsers call fails */
	void OnSearchUserErrorResponse(int32 Code, const FString& Message);

};