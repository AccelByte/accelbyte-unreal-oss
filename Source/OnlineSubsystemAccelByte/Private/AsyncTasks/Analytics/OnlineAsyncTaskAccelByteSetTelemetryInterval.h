// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Task for setting ServerGameTelemetry or GameTelemetry interval
 */
class FOnlineAsyncTaskAccelByteSetTelemetryInterval : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteSetTelemetryInterval, ESPMode::ThreadSafe>
{
public:
	
	FOnlineAsyncTaskAccelByteSetTelemetryInterval(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, int32 IntervalInSeconds);

	virtual void Initialize() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSetTelemetryInterval");		
	}

	int32 IntervalInSeconds;
};
