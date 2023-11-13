// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePresenceInterfaceAccelByte.h"

/**
 * Async task to query user presence using Locker API.
 */
class FOnlineAsyncTaskAccelByteBulkQueryUserPresence
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkQueryUserPresence, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteBulkQueryUserPresence(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const TArray<FUniqueNetIdRef>& InTargetUserIds);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override {
		return TEXT("FOnlineAsyncTaskAccelByteBulkQueryUserPresence");
	}

private:

	/** UserId of the friend we want to get the presence for */
	TArray<TSharedRef<const FUniqueNetIdAccelByteUser>> TargetUserIds;

	/** Map of user IDs with presence we bulk queried */
	TMap<FString, TSharedRef<FOnlineUserPresenceAccelByte>> PresenceResult;

	/** Delegate handler for when the QueryUserPresence call fails */
	void OnQueryUserPresenceError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when the QueryUserPresence call succeeds */
	void OnQueryUserPresenceSuccess(const FAccelByteModelsBulkUserStatusNotif& Result);
};

