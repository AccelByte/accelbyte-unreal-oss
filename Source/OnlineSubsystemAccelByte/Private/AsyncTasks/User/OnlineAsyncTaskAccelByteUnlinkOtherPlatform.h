// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteUnlinkOtherPlatform
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUnlinkOtherPlatform, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUnlinkOtherPlatform(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, EAccelBytePlatformType InPlatformType);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUnlinkOtherPlatform");
	}

private:
	// Endpoint Handlers
	void HandleSuccess();
	void HandleError(int32 Code, const FString& Message, const FJsonObject& JsonObject);

	// Error Information
	FOnlineError OnlineError;

	// Input Parameter
	EAccelBytePlatformType PlatformType;
};
