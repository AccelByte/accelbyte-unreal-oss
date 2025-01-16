// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineAsyncTaskManager.h"

class FOnlineSubsystemAccelByte;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskManagerAccelByte : public FOnlineAsyncTaskManager
{
public:

	/** Constructor to set up the cached parent subsystem for this manager instance */
	FOnlineAsyncTaskManagerAccelByte(FOnlineSubsystemAccelByte* ParentSubsystem);

	virtual ~FOnlineAsyncTaskManagerAccelByte() = default;

	void OnlineTick() override;

	void CheckMaxParallelTasks();

private:

	/** Pointer to subsystem instance that constructed this manager
	 * using explicit TWeakPtr here instead of FOnlineSubsystemAccelByteWPtr to remove cyclic dependency
	 */
	TWeakPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> AccelByteSubsystem;

	/** How long Task elapsed can considered as too long*/
	const double TaskTimeThreshold = 30.0;
};
