// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncMetaQuestIAP
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncMetaQuestIAP, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSyncMetaQuestIAP(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FOnRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncMetaQuestIAP");
	}

private:
	FOnRequestCompleted Delegate;

	TArray<TSharedRef<FPurchaseReceipt>> EntitlementInfos;

	FText ErrorText;
	FOnlineError OnlineError;

	void OnSyncOculusConsumableEntitlementsSuccess(const TArray<FAccelByteModelsSyncOculusConsumableEntitlementInfo>& Response);
	void OnSyncOculusConsumableEntitlementsFailed(const int32 ErrorCode, const FString& ErrorMessage);
};
