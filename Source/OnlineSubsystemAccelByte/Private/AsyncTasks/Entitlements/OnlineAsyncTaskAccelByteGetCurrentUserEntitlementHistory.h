// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory(FOnlineSubsystemAccelByte* const InABSubsystem
		, FUniqueNetId const& InLocalUserId
		, bool InForceUpdate
		, FUniqueEntitlementId const& InEntitlementId
		, EAccelByteEntitlementClass const& InEntitlementClass
		, FDateTime InStartDate
		, FDateTime InEndDate
		, FPagedQuery const& InPage);

	FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalUserNum
		, bool InForceUpdate
		, FUniqueEntitlementId const& InEntitlementId
		, EAccelByteEntitlementClass const& InEntitlementClass
		, FDateTime InStartDate
		, FDateTime InEndDate
		, FPagedQuery const& InPage);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory");
	}

private:
	void GetCurrentUserEntitlementHistory(int32 Offset, int32 Limit);

	void OnGetUserEntitlementHistorySuccess(FAccelByteModelsUserEntitlementHistoryPagingResult const& Result);
	THandler<FAccelByteModelsUserEntitlementHistoryPagingResult> SuccessDelegate;

	void OnGetUserEntitlementHistoryError(int32 Code, FString const& ErrMsg);
	FErrorHandler OnErrorDelegate;

	/* Flag to update user entitlement history from backend*/
	bool bForceUpdateEntitlementHistory{false};

	/* User entitlement Id */
	FUniqueEntitlementId EntitlementId{};

	/* Entitlement class*/
	EAccelByteEntitlementClass EntitlementClass{ EAccelByteEntitlementClass::NONE };

	/* Start date for filtering*/
	FDateTime StartDate{ 0 };

	/* End date for filtering*/
	FDateTime EndDate{ 0 };

	/* User entitlement Id */
	FPagedQuery PagedQuery = FPagedQuery{};

	/* Incoming user entitlement history information for server */
	TArray<FAccelByteModelsBaseUserEntitlementHistory> CurrentUserEntitlementHistoryResponse{};

	/* Success or Failure status code */
	int32 HttpStatus{ 0 };

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
