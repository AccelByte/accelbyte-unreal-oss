// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

/**
 * Async task to update dedicated or P2P game settings through the SessionBrowser APIs.
 */
class FOnlineAsyncTaskAccelByteUpdateV1GameSettings : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteUpdateV1GameSettings, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdateV1GameSettings(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName, FOnlineSessionSettings& InUpdatedSessionSettings, bool InBShouldRefreshOnlineData);

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

	bool bShouldRefreshOnlineData;
};