// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteForcePlatformLinkV3
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteForcePlatformLinkV3, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteForcePlatformLinkV3(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InPlatformId, const FString& InTicket);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteForcePlatformLinkV3");
	}

private:
	void HandleSuccess();
	void HandleError(int32 Code, const FString& Message, const FJsonObject& JsonObject);

	FOnlineError OnlineError;
	FString PlatformId;
	FString Ticket;
};
