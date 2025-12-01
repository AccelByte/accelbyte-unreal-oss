// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteSyncMetaSubscription
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSyncMetaSubscription, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSyncMetaSubscription(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InUserId
		, const FAccelByteModelsSyncOculusSubscriptionRequest& Request
		, const FOnSyncMetaSubscriptionCompleteDelegate& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override 
	{
		return TEXT("FOnlineAsyncTaskAccelByteSyncMetaSubscription");
	}

private:
	FAccelByteModelsSyncOculusSubscriptionRequest SyncRequest;
	FOnSyncMetaSubscriptionCompleteDelegate Delegate;

	FString ErrorInfo{};

	//The name function has Oculus name because the endpoint is Oculus
	void OnSyncOculusSubscriptionSuccess(const TArray<FAccelByteModelsThirdPartySubscriptionTransactionInfo>& Response);
	void OnSyncOculusSubscriptionFailed(const int32 ErrorCode, const FString& ErrorMessage);
	TArray<FAccelByteModelsThirdPartySubscriptionTransactionInfo> SyncSubscriptionResponse;
};
