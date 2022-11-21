// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

/**
 * Task for Sending Telemetry Event From ServerGameTelemetry or GameTelemetry
 */
class FOnlineAsyncTaskAccelByteSendTelemetry : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteSendTelemetry, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteSendTelemetry(
		FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum,
		FAccelByteModelsTelemetryBody const& TelemetryBody, FVoidHandler const& OnSuccess, FErrorHandler const& OnError);

	virtual void Initialize() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendTelemetry");
	}

private:
	
	FAccelByteModelsTelemetryBody TelemetryBody;
	FVoidHandler const& OnSuccessHandler;
	FErrorHandler const& OnErrorHandler;
};
