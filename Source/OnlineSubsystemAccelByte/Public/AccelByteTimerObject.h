// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Tickable.h"
#include "TimerManager.h"

/**
* This class will automatically register to gamethread tickable objects. 
* Start() function will make Tick() active
* When current time reached ExpirationTimeMs, the Delegate will be invoked once
*/
class FAccelByteTimerObject : public FTickableGameObject
{
public:
	FAccelByteTimerObject() = default;

	bool Start(int64 ExpirationTimeMs, const FTimerDelegate& InDelegate);
	bool StartIn(int64 StartInMs, const FTimerDelegate& InDelegate);
	void Stop();
	FORCEINLINE bool IsComplete() const { return bIsComplete; }
	FORCEINLINE bool IsStarted() const { return bIsStarted; }

protected:
	//~ Begin FTickableGameObject Interface.
	virtual bool IsTickable() const override { return bIsStarted && !bIsComplete; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject Interface.

private:
	// timer started flag
	bool bIsStarted{false};
	// expiration time in epoch milliseconds 
	int64 ExpirationTimeMs;
	// timer complete flag
	bool bIsComplete{false};
	// delegate for timer complete
	FTimerDelegate Delegate;
};
