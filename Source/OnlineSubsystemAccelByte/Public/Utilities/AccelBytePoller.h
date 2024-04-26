// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "Containers/Ticker.h"
#include "Core/AccelByteDefines.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAccelBytePoller, Log, All);

DECLARE_DELEGATE(OnPollExecute)

class ONLINESUBSYSTEMACCELBYTE_API FAccelBytePoller : public TSharedFromThis<FAccelBytePoller, ESPMode::ThreadSafe>
{
public:
	FAccelBytePoller();
	~FAccelBytePoller();

	bool StartPolling(const OnPollExecute& InAction, const float InDelay);
	bool StopPolling();
	bool SetDelay(int32 InDelay);

private:
	int32 MinInterval;
	bool bEnabled;
	FTimespan Delay;
	FDateTime NextPollTime {0};
	FDateTime LastExecTime {0};
	OnPollExecute Action;

	FTickerDelegate OnTickDelegate;
	FDelegateHandleAlias OnTickDelegateHandle;

	bool Tick(float DeltaTime);
	void CleanupTicker();
};
