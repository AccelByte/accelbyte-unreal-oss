// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Task for setting ServerGameTelemetry or GameTelemetry ImmediateEventList
 */
class FOnlineAsyncTaskAccelByteSetImmediateEventList : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteSetImmediateEventList, ESPMode::ThreadSafe>
{
public:
	
	FOnlineAsyncTaskAccelByteSetImmediateEventList(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, TArray<FString> const& EventNames);

	virtual void Initialize() override;

protected:
	
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSetImmediateEventList");
	}
	
	TArray<FString> ImmediateEventList;
};
