// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Utilities/AccelBytePoller.h"

DEFINE_LOG_CATEGORY(LogAccelBytePoller);

FAccelBytePoller::FAccelBytePoller()
	: MinInterval(1)
	, bEnabled(false)
	, Delay(0)
{
}

bool FAccelBytePoller::StartPolling(const OnPollExecute& InAction, const float InDelay)
{
	if(bEnabled)
	{
		UE_LOG(LogAccelBytePoller, Verbose, TEXT("Failed to start polling, poll already running"));
		return false;
	}

	if(!InAction.IsBound())
	{
		return false;
	}

	Action = InAction;
	Delay = FTimespan::FromSeconds(InDelay < MinInterval ? MinInterval : InDelay);

	if(OnTickDelegate.IsBound())
	{
		OnTickDelegate.Unbind();
	}

	if(OnTickDelegateHandle.IsValid())
	{
		OnTickDelegateHandle.Reset();
	}

	OnTickDelegate = FTickerDelegate::CreateThreadSafeSP(this, &FAccelBytePoller::Tick);
	OnTickDelegateHandle = FTickerAlias::GetCoreTicker().AddTicker(OnTickDelegate, 0.2f);
	LastExecTime = FDateTime::UtcNow();

	bEnabled = true;

	return true;
}

bool FAccelBytePoller::StopPolling()
{
	bEnabled = false;

	CleanupTicker();
	
	return true;
}

bool FAccelBytePoller::Tick(float DeltaTime)
{
	const bool bShouldExecute = (FDateTime::UtcNow() - LastExecTime) >= Delay;
	if(bShouldExecute)
	{
		Action.ExecuteIfBound();
		LastExecTime = FDateTime::UtcNow();
	}
	
	return true;
}

bool FAccelBytePoller::SetDelay(int32 InDelay)
{
	Delay = FTimespan::FromSeconds(InDelay < MinInterval ? MinInterval : InDelay);
	return true;
}

void FAccelBytePoller::CleanupTicker()
{
	if(OnTickDelegateHandle.IsValid())
	{
		FTickerAlias::GetCoreTicker().RemoveTicker(OnTickDelegateHandle);
		OnTickDelegateHandle.Reset();
	}
	
	if(OnTickDelegate.IsBound())
	{
		OnTickDelegate.Unbind();
	}
}

FAccelBytePoller::~FAccelBytePoller()
{
	CleanupTicker();
}
