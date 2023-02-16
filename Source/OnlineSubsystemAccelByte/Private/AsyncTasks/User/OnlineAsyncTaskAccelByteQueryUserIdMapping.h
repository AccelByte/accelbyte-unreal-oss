// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"

class FOnlineAsyncTaskAccelByteQueryUserIdMapping : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteQueryUserIdMapping, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteQueryUserIdMapping(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InDisplayNameOrEmail, const IOnlineUser::FOnQueryUserMappingComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryUserIdMapping");
	}

private:

	/** Display name or email to query for a user ID */
	FString DisplayNameOrEmail;

	/** Delegate to fire on completion */
	IOnlineUser::FOnQueryUserMappingComplete Delegate;

	/** ID of the user that was found by the request */
	TSharedPtr<const FUniqueNetIdAccelByteUser> FoundUserId;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/** Delegate handler for when the SearchUsers call gets data back, meaning we can search for a mapping */
	void OnSearchUserSuccessResponse(const FPagedPublicUsersInfo& Result);

	/** Delegate handler for when the SearchUsers call fails */
	void OnSearchUserErrorResponse(int32 Code, const FString& Message);

};