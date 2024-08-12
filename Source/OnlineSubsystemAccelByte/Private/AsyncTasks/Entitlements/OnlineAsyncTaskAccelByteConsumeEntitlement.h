// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteConsumeEntitlement
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteConsumeEntitlement, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteConsumeEntitlement(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FUniqueEntitlementId& InEntitlementId, int32 InUseCount, TArray<FString> InOptions = {}, const FString& InRequestId = {});

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteConsumeEntitlement");
	}

private:
	void HandleConsumeEntitlementSuccess(FAccelByteModelsEntitlementInfo const& Result);
	void HandleConsumeEntitlementError(int32 Code, FString const& ErrMsg);

	FUniqueEntitlementId EntitlementId;
	int32 UseCount;
	TArray<FString> Options;
	FString RequestId{};
	int32 ErrorCode = 0;
	FString ErrorMessage{};
	TSharedPtr<FOnlineEntitlementAccelByte> Entitlement;
};
