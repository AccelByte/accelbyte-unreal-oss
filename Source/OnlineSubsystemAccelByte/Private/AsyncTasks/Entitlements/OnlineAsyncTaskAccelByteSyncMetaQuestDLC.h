// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncMetaQuestDLC
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncMetaQuestDLC, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSyncMetaQuestDLC(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FOnRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncMetaQuestDLC");
	}

private:
	FOnRequestCompleted Delegate;

	FString ErrorInfo{};

	void OnSyncOculusDLCSuccess();
	void OnSyncOculusDLCFailed(const int32 ErrorCode, const FString& ErrorMessage);
};
