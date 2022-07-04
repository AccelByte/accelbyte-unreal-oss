// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteAgreementModels.h"
#include <OnlineIdentityInterfaceAccelByte.h>
#include <OnlineAgreementInterfaceAccelByte.h>

/**
 * Task for accept agreement policies
 */
class FOnlineAsyncTaskAccelByteAcceptAgreementPolicies : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteAcceptAgreementPolicies, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteAcceptAgreementPolicies(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const TArray<FOnlineAgreementAccelByte::FABAcceptAgreementPoliciesRequest>& InDocumentsToAccept);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteAcceptAgreementPolicies");
	}

private:

	/**
	 * Delegate handler for when accept agreement policies succeeds
	 */
	void OnAcceptAgreementPoliciesSuccess(const FAccelByteModelsAcceptAgreementResponse& Result);
	THandler<FAccelByteModelsAcceptAgreementResponse> OnAcceptAgreementPoliciesSuccessDelegate;

	/**
	 * Delegate handler for when accept agreement policies fails
	 */
	void OnAcceptAgreementPoliciesError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnAcceptAgreementPoliciesErrorDelegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	bool bRequestResult;
	bool bIsMandatory;

	TArray<FOnlineAgreementAccelByte::FABAcceptAgreementPoliciesRequest> DocumentsToAccept;
};
