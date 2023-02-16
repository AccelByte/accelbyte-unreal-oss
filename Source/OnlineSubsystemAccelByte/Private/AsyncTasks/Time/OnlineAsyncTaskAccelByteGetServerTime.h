// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineTimeInterfaceAccelByte.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Async task to set user Time using Lobby API.
 */
class FOnlineAsyncTaskAccelByteGetServerTime : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteGetServerTime, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetServerTime(FOnlineSubsystemAccelByte* const InABInterface);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override {
		return TEXT("FOnlineAsyncTaskAccelByteGetServerTime");
	}

private:

	/** ServerTime cached on the client */
	TSharedRef<FDateTime> LocalCachedServerTime;
	FString ErrorMessage;
	void HandleGetServerTimeSuccess(FTime const& Result);
	void HandleGetServerTimeError(int32 Code, FString const& ErrMsg);
};

