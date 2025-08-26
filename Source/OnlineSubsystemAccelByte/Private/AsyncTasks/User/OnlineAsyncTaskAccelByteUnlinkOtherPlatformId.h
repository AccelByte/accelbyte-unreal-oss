// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InPlatformId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId");
	}

private:
	// Endpoint Handlers
	void HandleSuccess();
	void HandleError(int32 Code, const FString& Message, const FJsonObject& JsonObject);

	// Error Information
	FOnlineError OnlineError;

	// Input Parameter
	FString PlatformId;
};
