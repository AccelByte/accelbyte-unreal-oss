// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteQueryPlatformSubscription
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryPlatformSubscription, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryPlatformSubscription(
		FOnlineSubsystemAccelByte* const InABSubsystem,
		int32 InLocalUserNum,
		const FOnlineQuerySubscriptionRequestAccelByte& QueryRequest);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryPlatformSubscription");
	}

private:
	void HandleQuerySubscriptionSuccess(FAccelByteModelsThirdPartyUserSubscriptions const& Result);
	void HandleQuerySubscriptionError(int32 Code, FString const& ErrMsg);

	FAccelByteModelsThirdPartyUserSubscriptionQueryRequest QueryRequest = {};
	
	FAccelByteModelsThirdPartyUserSubscriptions SuccessResult = {};
	int32 ErrorCode = 0;
	FString ErrorMessage = "";

	THandler<FAccelByteModelsThirdPartyUserSubscriptions> DelegateOnSuccess;
	FErrorHandler DelegateOnError;
};
