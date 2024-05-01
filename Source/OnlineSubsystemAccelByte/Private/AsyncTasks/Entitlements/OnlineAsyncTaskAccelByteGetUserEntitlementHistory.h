// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGetUserEntitlementHistory
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetUserEntitlementHistory, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetUserEntitlementHistory(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalTargetUserNum
		, const FUniqueEntitlementId& InEntitlementId
		, bool InForceUpdate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetUserEntitlementHistory");
	}

private:
	void OnGetUserEntitlementHistorySuccess(TArray<FAccelByteModelsUserEntitlementHistory> const& Result);
	THandler<TArray<FAccelByteModelsUserEntitlementHistory>> SuccessDelegate;

	void OnGetUserEntitlementHistoryError(int32 Code, FString const& ErrMsg);
	FErrorHandler OnErrorDelegate;

	/* User entitlement Id */
	bool bForceUpdateEntitlementHistory{false};

	/* User entitlement Id */
	FUniqueEntitlementId EntitlementId;

	/* Target user to check the entitlement history */
	int32 LocalTargetUserNum;

	/* Incoming information for user entitlement history */
	TArray<FAccelByteModelsUserEntitlementHistory> UserEntitlementHistoryResponse{};

	/* Success or Failure status code */
	int32 HttpStatus = 0;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
