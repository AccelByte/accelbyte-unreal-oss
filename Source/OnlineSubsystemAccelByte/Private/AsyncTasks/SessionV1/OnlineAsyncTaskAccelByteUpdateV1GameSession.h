// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

/**
 * Async task to update dedicated or P2P game sessions through the SessionBrowser APIs.
 * it will only update the max player and current player
 */
class FOnlineAsyncTaskAccelByteUpdateV1GameSession : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteUpdateV1GameSession(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName, FOnlineSessionSettings& InUpdatedSessionSettings, uint32 InCurrentPlayer, bool InBShouldRefreshOnlineData);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateGameSession");
	}

private:

	FName SessionName;

	FString SessionId;

	FOnlineSessionSettings& UpdatedSessionSettings;

	uint32 CurrentPlayer;

	bool bShouldRefreshOnlineData;
};