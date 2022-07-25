// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePresenceInterfaceAccelByte.h"

/**
 * Async task to query user presence using Locker API.
 */
class FOnlineAsyncTaskAccelByteQueryUserPresence : public FOnlineAsyncTaskAccelByte {
public:

	FOnlineAsyncTaskAccelByteQueryUserPresence(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InTargetUserId, const IOnlinePresence::FOnPresenceTaskCompleteDelegate& InDelegate, int32 InLocalUserNum);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override {
		return TEXT("FOnlineAsyncTaskAccelByteQueryUserPresence");
	}

private:

	/** UserId of the friend we want to get the presence for */
	TSharedRef<const FUniqueNetIdAccelByteUser> TargetUserId;

	/** Local presence cached on the client */
	TSharedRef<FOnlineUserPresenceAccelByte> LocalCachedPresence;

	/** Delegate to be fired after QueryUserPresence is complete */
	IOnlinePresence::FOnPresenceTaskCompleteDelegate Delegate;

	/** Delegate handler for when the QueryUserPresence call fails */
	void OnQueryUserPresenceError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when the QueryUserPresence call succeeds */
	void OnQueryUserPresenceSuccess(const FAccelByteModelsBulkUserStatusNotif& Result);
};

