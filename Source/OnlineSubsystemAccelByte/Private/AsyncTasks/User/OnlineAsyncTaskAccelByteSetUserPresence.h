// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "../OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Async task to set user presence using Lobby API.
 */
class FOnlineAsyncTaskAccelByteSetUserPresence : public FOnlineAsyncTaskAccelByte {
public:

	FOnlineAsyncTaskAccelByteSetUserPresence(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlineUserPresenceStatus& InStatus, const IOnlinePresence::FOnPresenceTaskCompleteDelegate& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override {
		return TEXT("FOnlineAsyncTaskAccelByteSetUserPresence");
	}

private:

	/** Local presence status cached on the client */
	TSharedRef<FOnlineUserPresenceStatus> LocalCachedPresenceStatus;

	/** Delegate to be fired after SetUserPresence is complete */
	IOnlinePresence::FOnPresenceTaskCompleteDelegate Delegate;

	/** Delegate handler for when the SetUserPresence call done */
	void OnSetUserPresenceResponse(const FAccelByteModelsSetOnlineUsersResponse& Result);
};

