// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteTimerObject.h"

bool FAccelByteTimerObject::Start(int64 InExpirationTimeMs, const FTimerDelegate& InDelegate)
{
	if (!bIsStarted)
	{
		bIsComplete = false;
		bIsStarted = true;
		ExpirationTimeMs = InExpirationTimeMs;
		Delegate = InDelegate;
		return true;
	}

	return false;
}

bool FAccelByteTimerObject::StartIn(int64 StartInMs, const FTimerDelegate& InDelegate)
{
	FDateTime Now = FDateTime::UtcNow();
	int64 NowMs = Now.ToUnixTimestamp() * 1000 + Now.GetMillisecond();
	return Start(StartInMs + NowMs, InDelegate);
}

void FAccelByteTimerObject::Stop()
{
	bIsStarted = false;
}

void FAccelByteTimerObject::Tick(float DeltaTime)
{
	FDateTime Now = FDateTime::UtcNow();
	int64 NowMs = Now.ToUnixTimestamp() * 1000 + Now.GetMillisecond();

	if (ExpirationTimeMs <= NowMs)
	{
		bIsComplete = true;
		bIsStarted = false;
		Delegate.ExecuteIfBound();
	}
}

TStatId FAccelByteTimerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FAccelByteTickableTimer, STATGROUP_Tickables);
}
