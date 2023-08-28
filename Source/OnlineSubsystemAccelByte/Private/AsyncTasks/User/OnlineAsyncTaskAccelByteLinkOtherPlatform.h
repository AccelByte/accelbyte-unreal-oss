// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteLinkOtherPlatform
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLinkOtherPlatform, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteLinkOtherPlatform(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, EAccelBytePlatformType InPlatformType, const FString& InTicket);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLinkOtherPlatform");
	}

private:

	// Endpoint Handlers
	void HandleSuccess();
	void HandleError(int32 ErrorCode, const FString& ErrorMessage, const FJsonObject& JsonObject);

	// Error Information
	FOnlineError OnlineError;

	// Input Parameters 
	EAccelBytePlatformType PlatformType;
	FString Ticket;
	
};
